#!/usr/bin/python
import pygst
pygst.require("0.10")
import gst
import pygtk
import gtk
import gtk.glade

def OnPad(comp, pad):
    print "pad added!"
    convpad = compconvert.get_compatible_pad(pad, pad.get_caps())
    pad.link(convpad)


pipeline = gst.Pipeline("mypipeline")

comp = gst.element_factory_make("gnlcomposition", "mycomposition")
pipeline.add(comp)
comp.connect("pad-added", OnPad)

compconvert = gst.element_factory_make("audioconvert", "compconvert")
pipeline.add(compconvert)

sink = gst.element_factory_make("alsasink", "alsasink")
pipeline.add(sink)
compconvert.link(sink)

audio1 = gst.element_factory_make("gnlfilesource", "audio1")
comp.add(audio1)

audio1.set_property("location", "/media/data/Music/Yesterday.mp3")
audio1.set_property("start", 0 * gst.SECOND)
audio1.set_property("duration", 50 * gst.SECOND)
audio1.set_property("media-start", 10 * gst.SECOND)
audio1.set_property("media-duration", 50 * gst.SECOND)

pipeline.set_state(gst.STATE_PLAYING)

gtk.main()
