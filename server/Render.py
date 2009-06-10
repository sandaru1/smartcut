import json
import os

def Render(timeline):
  t = json.loads(timeline);
  paths = []
  for i in t:
    path = os.getcwd()+'/media/videos/'+i['name']+'.avi'
    if len(i['effects']) > 0 :
      pipe = ""
      for k in i['effects']:
        pipe = " ! " + k
      pipe = "gst-launch-0.10 filesrc location=" + path + " ! decodebin ! ffmpegcolorspace" + pipe + " ! ffmpegcolorspace ! queue ! ffmpegcolorspace ! jpegenc ! avimux ! filesink location=/tmp/"+i['name']+'.avi'
      print pipe
      os.system(pipe)
      path = "/tmp/" + i['name'] + '.avi'
    paths.append(path)
  cmd = "mencoder -oac copy -ovc copy -o /tmp/final.avi"
  for i in paths:
    cmd += " '"+i+"'"
  os.system(cmd)
  os.system('gst-launch-0.10 filesrc location=/tmp/final.avi ! decodebin ! queue ! ffenc_flv ! ffmux_flv ! filesink location='+os.getcwd()+"/media/flv/final.flv")
