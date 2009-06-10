#!/usr/bin/env python

""" eli.py - a class to handle mjpegtools' edit lists (.eli files)
    v0.1 (c) Heiko Noordhof

    An eli object inherits from python's built-in list object and
    allowes slicing and concatenating of frames in an edit list.
"""

__author__ =    'Heiko Noordhof <heiko@heiks.nl>'
__version__ =   '0.1'
__copyright__ = 'Copyright (c) 2005 Heiko Noordhof'
__license__ =   'General Public License (GPL)'


import os.path
import sys
from cStringIO import StringIO

# Magic string for edit list files (or strings)
eli_magic = "LAV Edit List"
default_videotype = "PAL"

class StringItem(str):
    def __init__(self, s):
        str.__init__(self, s)
        self.index = None
        self.refcount = 1        

class UniqStringList:
    '''A indexed list of strings in which no duplicates are stored.
    
    When a string value is added that already exists in the list, it will be
    stored at the index of the first string that had this value.
    
    A reference count is maintained for the string values, so if a certain
    string value is added 3 times, it will be deleted from the list when it is
    removed 3 times.
    '''
    def __init__(self, sequence=None):
        '''Create new UniqStringList object 
        UniqStringList -> new empty UniqStringList.
        UniqStringList(sequence) -> new UniqStringList initialized from
        sequence's items.
        '''
        self._dict = {}
        self._list = []
        if sequence is not None:
            for item in sequence:
                self.add(item)

    def __delete(self, item):
        item.refcount -= 1
        idx = item.index
        if item.refcount <= 0:
            del self._dict[item]
            del self._list[idx]
            for s in self._list[idx:]:
                s.index -= 1

    def  __str__(self):
        result = ''
        for s in self:
            result = '%s%s\n' % (result, s)
        return result[:-1]    # remove last newline
    
    def refcount(self, s):
        try:
            if isinstance(s, str): return self._dict[s].refcount
            elif isinstance(s, int): return self._list[s].refcount
            else: raise TypeError
        except  (KeyError, IndexError): return 0
    
    def __getitem__(self, s):
        '''Returns:
         - The index where s is stored if s is a string.
         - The string with index s if s is an integer.
        '''
        if isinstance(s, str): return self._dict[s]
        elif isinstance(s, int): return self._list[s]
        else: raise TypeError
    
    def __delitem__(self, s):
        self.remove(s)        
            
    def __len__(self):
        return len(self._list)
    
    def __iter__(self):
        return self._list.__iter__()
    
    def add(self, s):
        '''Adds a string value to the UniqStringList.        
        Returns: index of the string value where it was stored in the list.
        '''
        item = self._dict.get(s)
        if item == None:  # not in list yet
            item = StringItem(s)
            item.index = len(self._list)
            self._list.append(item)
            self._dict[s] = item
            return item.index
        else:  # value already in list
            item.refcount += 1
            return item.index
            
    def remove(self, s):
        '''Removes a string for the list by decreasing the reference count.
        Only if the reference count reaches zero the string is really deleted.
        s can be:
            a integer -> the string with index s is deleted.
            a string -> the string s is deleted.
         '''
        if isinstance(s, str): self.__delete(self._dict[s])
        elif isinstance(s, int): self.__delete(self._list[s])
        else: raise TypeError
            
    def show(self):
        '''returns string containing the list with index and reference counts'''
        result = ''
        for s in self:
            item = self._dict[s]
            result = '%s%d %s %d\n' % (result, item.index, s, item.refcount)
        return result[:-1]    # remove last newline

class EliError(Exception):
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return repr(self.msg)
        
        
class EliFrameBlock:
    def __init__(self, path=None, start=None, stop=None, comment=None):
        if comment is None: comment = ''
        self.path = path
        self.start = start
        self.stop = stop
        self.comment = comment
 
    def __len__(self):
        return self.stop - self.start + 1
        
class EliParser:
    def __init__(self, eli):
        self.eli = eli
        self.file = None
        self.count = None
        self.linecount = 0
        self.filelist = []

    def __next_line(self):
        line = self.file.readline().strip()
        self.linecount += 1
        while line == "\n":
            line = f.readline().strip()
            self.linecount += 1
        return line

    def __parse_magic(self):
        line = self.__next_line()
        if line != eli_magic:
            sys.stderr.write('Warning (line %d): expected file to start with "%s"\n' % (eli_magic,self.linecount))

    def __parse_videotype(self):
        videotype = self.__next_line()
        videotype = videotype.upper()
        if videotype != "PAL" and videotype != "NTSC":
            sys.stderr.write("Warning (line %d): invalid video type '%s'."\
                                    "Default '%s' assumed\n" % (self.linecount, self.videotype,
                                    default_videotype))
        self.eli.videotype = videotype

    def __parse_filecount(self):
        line = self.__next_line()
        self.count = int(line)

    def __parse_paths(self):
        n = 0
        while n < self.count:
            path = self.__next_line()
            if not path:
                raise EliError("unexpected end of file in line %d" % (self.linecount,))
            path = os.path.normpath(path)
            self.filelist.append(path)
            n += 1            

    def __parse_blocks(self):
        line = self.__next_line()
        while line:
            chopped = line.split()
            if len(chopped) < 3:
                raise EliError("expected frame block in line %d: 3 integers per line"\
                                        "(and optional comment)" % (self.linecount,))
            # Check syntax
            if not chopped[0].isdigit():
                raise EliError("file index (integer) expected in line %d" % (self.linecount,))
            if not chopped[1].isdigit():
                raise EliError("start frame(integer) expected in line %d" % (self.linecount,))
            if not chopped[2].isdigit():
                raise EliError("start frame (integer) expected in line %d" % (self.linecount,))
                
            # Get movie file path this frame block refers to
            idx = int(chopped[0])
            path = self.filelist[idx]
            start = int(chopped[1])
            stop = int(chopped[2])
            comment = ''
            del chopped[:3]
            comment = " ".join(chopped)
            self.eli.frameblocks.append(EliFrameBlock(path, start, stop, comment))
            line = self.__next_line()
            
    def parse(self, f):
        self.file = f
        self.__parse_magic()
        self.__parse_videotype()
        self.__parse_filecount()
        self.__parse_paths()
        self.__parse_blocks()
        
        
        

class Eli:
    """ eli - mjpegtools' edit list (.eli) object class.
    
    The path of an mjpeg file, an edit list file, or another eli-object
    can be passed to the constructor.
    The default is an empty edit list (contains zero frames) with videotype PAL.
    """

    def __init__(self, arg=None, videotype=None):    
        """__init__(arg)  -  Creates instance of eli class.
        Argument <arg> can be either:
        - None. 
        - Another eli object. The created eli object will be a "deep copy" of it.
        - A multi-line string containing the edit list.
        - A file(-like) object to read the edit list from.
        - The path of an edit list file (string ending in ".eli")
        - The path of an mjpeg file (string ending in ".mjpeg") # not inplemented yet.
        """
        if videotype is None:
            videotype = default_videotype
        videotype = videotype.upper()
        if videotype != "PAL" and videotype != "NTSC":
            raise EliError("Invalid video type. Should be either 'PAL' or 'NTSC'")
        self.videotype = videotype
        self.frameblocks = list()
        self.frames = _EliFrameInterface(self)
        self.reset()
        self.load(arg)
        
    def reset(self):
        del self.frameblocks[:]
        
    def setVideoType(self, videotype):
        videotype = videotype.upper()
        if videotype != "PAL" and videotype != "NTSC":
            raise EliError("Invalid video type. Should be either 'PAL' or 'NTSC'")
        self.videotype = videotype        
        
    def load(self, arg):
        if arg is None: return
        elif isinstance(arg, Eli): self.loadEli(arg)
        elif isinstance(arg, file): self.loadFile(arg)
        elif isinstance(arg, str):
            if arg.rfind('\n') > 0: self.loadString(arg)
            elif arg[-4:] == '.eli': self.loadFile(file(arg))
            elif arg[-6:] == '.mjpeg': self.loadMJPEG(file(arg))
            else: raise TypeError("String format not supported" % type(arg))
        else: raise TypeError("Argument type '%s' not supported" % type(arg))
        self.refresh()
        
    def loadEli(self, eli):
        for b in eli.frameblocks[:]:
            self.appendFrameBlock(b)

    def loadMJPEG(self, mjpegfile):
        self.reset()
        print "Not implemented (yet)!"

    def loadString(self, s):
        self.reset()
        self.loadFile(StringIO(s))

    def loadFile(self, f):
        self.reset()
        p = EliParser(self)
        p.parse(f)
        
    def getFrameBlock(self, index):
        result = Eli(videotype=self.videotype)
        fb = self.frameblocks[index]
        result.frameblocks.append(fb)
        return result
        
    def getFrameBlocksWithFile(self, path):
        result = Eli(videotype=self.videotype)
        path = os.path.normpath(path)
        for fb in self.frameblocks:
            if fb.path == path:
                result.frameblocks.append(fb)
        return result
        
    def deleteFrameBlocksWithFile(self,  path):
        path = os.path.normpath(path)
        tmplist = []
        for fb in self.frameblocks:
            if fb.path != path:
                tmplist.append(fb)
        del self.frameblocks[:]
        self.frameblocks.extend(tmplist)
        
    def getRange(self, start, stop):
        result = Eli()
        for i in range(start, stop):
            result.appendFrameBlock(self.frameblocks[i])
        return result            
        
    def deleteFrameBlock(self, arg):
        if isinstance(arg, int):
            del self.frameblocks[arg]
        elif isinstance(arg, str):
            path = os.path.normpath(arg)
            for i in range(len(self.frameblocks)):
                if self.frameblocks[i].path == path:
                    del self.frameblocks[i]
        else:
            raise TypeError("Argument type '%s' not supported" % type(arg))
        
    def appendEli(self, eli):
        for fb in eli.frameblocks[:]:
            self.appendFrameBlock(fb)
        
    def appendFrameBlock(self, frameblock):
        if frameblock.stop < frameblock.start: return
        #fb = EliFrameBlock(os.path.normpath(frameblock.path), frameblock.start, frameblock.stop, frameblock.comment)
        frameblock.path = os.path.normpath(frameblock.path)
        self.frameblocks.append(frameblock)
        
    def append(self, arg, start=None, stop=None, comment=None):
        if isinstance(arg, str): self.__append(os.path.normpath(arg), start, stop, comment)
        elif isinstance(arg, Eli): self.appendEli(arg)
        else: raise TypeError("Argument type '%s' not supported" % type(arg))
    
    def insert(self, index, arg, start=None, stop=None, comment=None):
        if isinstance(arg, str): self.__insert(index, arg, start, stop, comment)
        elif isinstance(arg, Eli): self.insertEli(index, arg)
        else: raise TypeError("Argument type '%s' not supported" % type(arg))
        
    def save(self, path):
        fl = file(path, 'w')
        fl.write(str(self))
        
    def string(self, framecount=True, comment=True):
        if comment and not framecount:  return self.__str__()
        files = self.refresh()
        s = '%s\n%s\n%d\n' % (eli_magic, self.videotype, len(files))
        for f in files:
            s = '%s%s\n' % (s, f)
        f = 0
        for fb in self.frameblocks:
            f2 = f + len(fb) - 1
            pathidx = files[fb.path].index
            if comment:
                s = '%s%-3d %8d %8d    #Frames: %8d  %8d  %s\n' % (s, pathidx,
                                                                fb.start, fb.stop, f, f2, fb.comment)
            elif framecount:
                s = '%s%-3d %8d %8d    #Frames: %8d  %8d\n' % (s, pathidx,
                                                                fb.start, fb.stop, f, f2)
            else:
                s = '%s%-3d %8d %8d\n' % (s, pathidx, fb.start, fb.stop)
            f = f2 + 1
        return s.strip()
        
    def __str__(self):
        files = self.refresh()
        s = '%s\n%s\n%d\n' % (eli_magic, self.videotype, len(files))
        for f in files:
            s = '%s%s\n' % (s, f)
        for b in self.frameblocks:
            pathidx = files[b.path].index
            s = '%s%-3d %8d %8d    %s\n' % (s, pathidx, b.start, b.stop, b.comment)
        return s.strip()
        
    ## def __repr__(self):
    ##   return str(self)
        
    def __len__(self):
        return len(self.frameblocks)

    def __getitem__(self, arg):
        if isinstance(arg, int): return self.getFrameBlock(arg)
        elif isinstance(arg, str): return self.getFrameBlocksWithFile(arg)
        elif isinstance(arg, slice):
            idxs = arg.indices(len(self))
            if idxs[2] != 1: raise IndexError("stepped slices are not supported")
            return self.getRange(idxs[0], idxs[1])
        else: raise TypeError("Argument type '%s' not supported" % type(arg))
        
    def __setitem__(self, index, eli):
        length = len(self)
        self.__delitem__(index)
        if isinstance(index, int): self.insertEli(index, eli)
        elif isinstance(index, slice):
            idxs = index.indices(length)
            self.insertEli(index.start, eli)
    
    def __delitem__(self, arg):
        if isinstance(arg, int): return self.deleteFrameBlock(arg)
        elif isinstance(arg, str): return self.deleteFrameBlocksWithFile(arg)
        elif isinstance(arg, slice):
            idxs = arg.indices(len(self))
            if idxs[2] != 1: raise IndexError("stepped slices are not supported")
            del self.frameblocks[idxs[0]: idxs[1]]
        else: raise TypeError("Argument type '%s' not supported" % type(arg))
        
    def __add__(self, term):
        result = Eli(self)
        for b in term.frameblocks:
            result.appendFrameBlock(b)
        return result        

    def __iadd__(self, term):
        for b in term.frameblocks:
            self.appendFrameBlock(b)
        return self
    
    def __mul__(self, factor):
        if not isinstance(factor, int): 
            raise TypeError("Argument type '%s' not supported" % type(arg))
        if factor < 0:
            raise ValueError("Negative operand not supported" % type(arg))
        result = Eli()
        for i in range(factor):
            for b in self.frameblocks:
                result.appendFrameBlock(b)
        return result            
    
    def __rmul__(self, factor):
        return self.__mul__(factor)
        
    def __imul__(self, factor):
        if not isinstance(factor, int): 
            raise TypeError("Argument type '%s' not supported" % type(arg))
        if factor < 0:
            raise ValueError("Negative operand not supported" % type(arg))
        if factor == 0:
            self.reset()
        copy = self.frameblocks[:]
        for i in range(1, factor):
            for b in copy:
                self.appendFrameBlock(b)      
        return self  
    
    def __iter__(self):        
        for b in self.frameblocks:
            result = Eli(videotype=self.videotype)
            result.frameblocks.append(b)
            yield result

    def __append(self, path, start, stop, comment=None):
        if stop < start: return
        if comment is None: comment = ''
        path = os.path.normpath(path)
        fb = EliFrameBlock(path, start, stop, comment)
        self.frameblocks.append(fb)
        
    def __insert(self, index, path, start, stop, comment=None):
        if stop < start: return
        if comment is None: comment = ''
        path = os.path.normpath(path)
        fb = EliFrameBlock(path, start, stop, comment)
        self.frameblocks.insert(index, fb)
        
    def insertEli(self, index, eli):
        for b in eli.frameblocks[:]:
            self.frameblocks.insert(index, b)
            index += 1

    def refresh(self):
        flist = UniqStringList()
        for b in self.frameblocks:
            idx = flist.add(b.path)
            b.path = flist[idx] # replace stored path with normalized path
        return flist
        

class _EliFrameInterface:
    '''Overides methods to provide slicing and indexing frames (instead of eli-frameblocks)'''
    def __init__(self, eli):
        self.eli = eli

    def __len__(self):
        count = 0
        for b in self.eli.frameblocks:
            count += len(b)
        return count
    
    def __getitem__(self, arg):
        if not isinstance(arg, slice):
            raise TypeError("Argument type '%s' not supported" % type(arg))
        idxs = arg.indices(len(self))
        start = idxs[0]
        stop = idxs[1]
        result = Eli()
        if start >= stop:
            return result
        framecount = 0
        inslice = False
        for fb in self.eli.frameblocks:
            framecount += len(fb)
            if inslice:
                fbnew = EliFrameBlock(fb.path, fb.start, fb.stop, fb.comment)
                result.appendFrameBlock(fbnew)
            else:
                startoffset = framecount - start
                if startoffset == 0:
                    inslice = True
                if startoffset > 0:
                    inslice = True
                    fbnew = EliFrameBlock(fb.path, fb.start, fb.stop, fb.comment)
                    fbnew.start = fb.stop - startoffset + 1
                    result.appendFrameBlock(fbnew)
            if inslice:
                stopoffset = framecount - stop
                if stopoffset >= 0:
                    fbnew.stop = fb.stop - stopoffset
                    return result
                
    def __delitem__(self, arg):
        if isinstance(arg, int):
            if arg >= len(self): raise IndexError("Frame index out of range")
            else: del self[arg:arg+1]
            return
        elif not isinstance(arg, slice):
            raise TypeError("Only slices and indices (int's) are supported")
        idxs = arg.indices(len(self))
        start = idxs[0]
        stop = idxs[1]
        if start >= stop:
            return
        tmp1 = self.eli.frames[:start]
        tmp2 = self.eli.frames[stop:]
        self.eli.reset()
        self.eli.frameblocks.extend(tmp1.frameblocks)
        self.eli.frameblocks.extend(tmp2.frameblocks)
        
    def __setitem__(self, index, eli):
        if not isinstance(arg, slice):
            raise TypeError("Only slices are supported")
        idxs = arg.indices(len(self))
        start = idxs[0]
        stop = idxs[1]
        if start >= stop:
            return        
        tmp1 = self.eli.frames[:start]
        tmp2 = self.eli.frames[stop:]
        self.eli.reset()
        self.eli.frameblocks.extend(tmp1.frameblocks)
        self.eli.frameblocks.extend(eli.frameblocks)
        self.eli.frameblocks.extend(tmp2.frameblocks)
        
    
def main():
    e = Eli("/tmp/example.eli")
    print e.frames[2:12]
    print e
    return 0

if __name__ == "__main__":
    sys.exit(main())
