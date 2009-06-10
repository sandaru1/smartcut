import json

effects = [
      {'display' : 'Edge Detect', 'name': 'edgetv'},
      {'display' : 'Dice', 'name': 'dicetv'},
      {'display' : 'Warp', 'name': 'warptv'},
      {'display' : 'Face Blur', 'name': 'faceblur'}]

def getList():
  return json.dumps({'effects':effects})

