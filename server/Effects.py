import json

effects = [
      {'display' : 'Edge Detect', 'name': 'edgetv'},
      {'display' : 'Dice', 'name': 'dicetv'},
      {'display' : 'Warp', 'name': 'warptv'},
      {'display' : 'Face Blur', 'name': 'faceblur'},
      {'display' : 'QuarkTV', 'name': 'quarktv'},
      {'display' : 'Aging', 'name': 'agingtv'},
      {'display' : 'Rev', 'name': 'revtv'},
      {'display' : 'Vertigo', 'name': 'vertigotv'},
      {'display' : 'Object Blur', 'name': 'objblur'}]

def getList():
  return json.dumps({'effects':effects})
