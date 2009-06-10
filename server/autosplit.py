#!/usr/bin/env python

from eli import Eli
import gst
import os
import sys

def autoSplit(path):
  os.system("mencoder -lavcopts vcodec=mjpeg -ovc lavc "+path+" -o /tmp/video.avi -nosound");
  os.system("lav2yuv +p -S /tmp/s.eli /tmp/video.avi")

  e = Eli(file('/tmp/s.eli'))

  tmp = os.popen("lavinfo +p "+e.frameblocks[0].path + " | grep video_fps").read()
  tmp = float(tmp.split('=')[1])
  fs = gst.SECOND/tmp

  pos = 0
  for i in e.frameblocks:
    start = str(int(fs*i.start))
    duration = str(int(fs*len(i)))
    os.system("gst-launch-0.10 gnlfilesource location="+i.path+" media-start=" + start + " media-duration=" + duration + " ! queue ! jpegenc ! avimux ! filesink location=/tmp/" + str(pos) + ".avi")
    pos += 1
  return pos

def main():
  if len(sys.argv)<2 :
    print sys.argv[0] + " filename"
    sys.exit(1)
  autosplit(sys.argv[0])

if __name__ == "__main__":
  main()
