import json
import glob
import os
import shutil
import autosplit

def getContent():
  os.chdir('media/flv')
  thumbs = glob.glob("*")
  os.chdir('../../')
  t = []
  for i in thumbs:
    num = i.split('.')[0]
    if num == "final":
      continue
    t.append({'name' : num,'url' : 'http://localhost/smartcut/thumbs/'+num+'.jpg'})
  return json.dumps({'images':t})

def uploadClip(path):
  f = file('media/next')
  num = int(f.read())
  f.close()
  f = file('media/next','w')
  f.write(str(num+1))
  f.close()
  num = str(num)
  shutil.copyfile(path,'./media/videos/' + num + '.avi')
  os.system('gnome-video-thumbnailer -j ./media/videos/' + num + '.avi ./media/thumbs/' + num + '.jpg')
  # convert to flv
  os.system('gst-launch-0.10 filesrc location=./media/videos/'+num+'.avi ! decodebin ! queue ! ffenc_flv ! ffmux_flv ! filesink location=./media/flv/'+num+'.flv')

def AutoSplit(name):
  r = autosplit.autoSplit(os.getcwd()+'/media/videos/'+name+'.avi')
  for i in range(0,r):
    uploadClip('/tmp/'+str(i)+'.avi')

def removeClip(name):
  os.unlink('./media/videos/' + name + '.avi')
  os.unlink('./media/flv/' + name + '.flv')
  os.unlink('./media/thumbs/' + name + '.jpg')

