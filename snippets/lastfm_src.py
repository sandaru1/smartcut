# -*- coding: utf-8 -*-
# Licensed under the MIT license
# http://opensource.org/licenses/mit-license.php

# Copyright 2007, Philippe Normand <phil at base-art dot net>

import gobject, gst
import urllib2, urllib
import md5, sys

"""
TODO:

- make use of gst/gobject properties for user/password
- register lastfm:// uri handler when i figure the he** how to do that
- use gst debug lib within src.debug()

"""
    

class LastFMSource(gst.BaseSrc):
    __gsttemplates__ = (
        gst.PadTemplate("src",
                        gst.PAD_SRC,
                        gst.PAD_ALWAYS,
                        gst.caps_new_any()),
        )

    __gstdetails__ = ("Last.FM radios plugin",
                      "Source/File", "Read data on Last.FM radios quoi",
                      "Philippe Normand <philippe@fluendo.com>")

    blocksize = 4096
    fd = None
    
    def __init__(self, name, user=None, password=None):
        self.__gobject_init__()
        self.curoffset = 0
        self.set_name(name)
        self.user = user
        self.password = password
        if user and password:
            self.handshake()

        pad = self.get_pad("src")
        pad.add_buffer_probe(self.buffer_probe)

    def buffer_probe(self, pad, buffer):
        if buffer and buffer.data:
            if buffer.data.find('SYNC') > -1:
                self.update()
        return True

    def _urlopen(self, url, params):
        if url.startswith('/'):
            url = "http://%s%s%s" % (self.params['base_url'],
                                     self.params['base_path'], url)
            
        params = urllib.urlencode(params)
        url = "%s?%s" % (url, params)
        try:
            result = urllib2.urlopen(url).readlines()
        except Exception, ex:
            self.debug(ex)
            result = None
        else:
            self.debug(result)
        return result
        
    def set_property(self, name, value):
        if name == 'uri':
            self.tune_station(value)

    def do_create(self, offset, size):
        if self.fd:
            data = self.fd.read(self.blocksize)
            if data:
                self.curoffset += len(data)
                return gst.FLOW_OK, gst.Buffer(data)
            else:
                return gst.FLOW_UNEXPECTED, None
        else:
            return gst.FLOW_UNEXPECTED, None
            
    def debug(self, msg):
        print "[Last.FM] %s" % msg

    def handshake(self):
        self.logged = False
        self.debug("Handshaking...")

        passw = md5.md5(self.password).hexdigest()
        url = "http://ws.audioscrobbler.com/radio/handshake.php"
        params = {'version': '0.1', 'platform':'linux',
                  'username': self.user, 'passwordmd5': passw, 'debug':'0'}

        result = self._urlopen(url, params)
        if result:
            self.params = {}
            for line in result:
                if line.endswith('\n'):
                    line = line[:-1]
                parts = line.split('=')
                if len(parts) > 2:
                    parts = [parts[0], '='.join(parts[1:])]
                self.params[parts[0]] = parts[1]
            if self.params['session'] != 'FAILED':
                self.logged = True
                self.update()
                
    def tune_station(self, station_uri):
        if not self.logged:
            self.debug('Error: %s' % self.params.get('msg'))
            # TODO: raise some exception? how to tell gst not to go further?
            return
        
        self.debug("Tuning to %s" % station_uri)
        
        url = "/adjust.php"
        params = {"session": self.params['session'], "url": station_uri,
                  "debug": '0'}

        result = self._urlopen(url, params)
        if result:
            response = result[0][:-1].split('=')[1]
            if response == 'OK':
                self.fd = urllib2.urlopen(self.params['stream_url']).fp
                self.update()
                
    def update(self):
        """
        Update current track metadata to `self.track_infos` dictionary which
        looks like:

        ::
           {'album': str
            'albumcover_large': uri (str),
            'albumcover_small': uri (str),
            'albumcover_medium': uri (str),
            'shopname': str, 'artist': str,
            'track': str,
            'price': str, 'trackduration': int,
            'streaming': boolean ('true'/'false'),
            'artist_url': uri (str),
            'album_url': uri (str),
            'station': str,
            'radiomode': int, 'station_url': uri (str),
            'recordtoprofile': int,  'clickthrulink': str, 'discovery': int
           }
        """
        url = "/np.php"
        params = {'session': self.params['session'], 'debug':'0'}
        result = self._urlopen(url, params)
        self.track_infos = {}
        if result:
            for line in result:
                # strip ending \n
                if line.endswith('\n'):
                    line = line[:-1]
                parts = line.split('=')
                if len(parts) > 2:
                    parts = [parts[0], '='.join(parts[1:])]
                try:
                    value = int(parts[1])
                except:
                    value = parts[1]
                self.track_infos[parts[0]] = value
            self.debug(self.track_infos)

    def control(self, command):
        """
	Send control command to last.fm to skip/love/ban the currently
	played track or enable/disable recording tracks to profile.

        `command` can be one of: "love", "skip", "ban", "rtp" and "nortp"
        """
        url = "/control.php"
        params = {'session': self.params['session'], 'command': command,
                  'debug':'0'}
        result = self._urlopen(url, params)
        if result:
            response = result[0][:-1].split('=')[1]
            if response == 'OK':
                self.update()
                return True
        return False

    def set_discover(self, value):
        """

        """
        uri = "lastfm://settings/discovery/%s"
        if value:
            uri = uri % "on"
        else:
            uri = uri % "off"
        self.tune_station(uri)

    def love(self):
        return self.control('love')

    def skip(self):
        return self.control('skip')

    def ban(self):
        return self.control('ban')

    def set_record_to_profile(self, value):
        if value:
            cmd = 'rtp'
        else:
            cmd = 'nortp'
        return self.command(cmd)
    
gobject.type_register(LastFMSource)

def main(args):
    user, password, uri = args

    bin = gst.Pipeline('pipeline')

    lastfmsrc = LastFMSource('src',user, password)
    decoder = gst.element_factory_make('decodebin')
    queue = gst.element_factory_make('queue')
    convert = gst.element_factory_make('audioconvert')
    sink = gst.element_factory_make('autoaudiosink', 'sink')
        
    def on_new_decoded_pad(element, pad, last):
        caps = pad.get_caps()
        name = caps[0].get_name()
        apad = convert.get_pad('sink')
        if 'audio' in name:
            if not apad.is_linked(): # Only link once
                pad.link(apad)

    decoder.connect('new-decoded-pad', on_new_decoded_pad)
    bin.add(lastfmsrc, decoder, queue, convert, sink)
    lastfmsrc.link(decoder)
    convert.link(sink)
    
    lastfmsrc.set_property('uri', uri)
    bin.set_state(gst.STATE_PLAYING)
    
    mainloop = gobject.MainLoop()

    def bus_event(bus, message):
        t = message.type
        if t == gst.MESSAGE_EOS:
            mainloop.quit()
        elif t == gst.MESSAGE_ERROR:
            err, debug = message.parse_error()
            print "Error: %s" % err, debug
            mainloop.quit()           
        return True
    
    bin.get_bus().add_watch(bus_event)
    mainloop.run()
    bin.set_state(gst.STATE_NULL)
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
