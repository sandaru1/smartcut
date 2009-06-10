var timeline = Ext.air.NativeWindow.getRootHtmlWindow().Ext.getCmp('timeline');
var effects = Ext.air.NativeWindow.getRootHtmlWindow().Ext.getCmp('effects');

var c;

function onEndCrop( coords, dimensions ) {
  c = coords;
}
		
Event.observe( 
	window, 
	'load', 
	function() { 
    $('testImage').src = 'http://localhost/smartcut/frames/'+timeline.dv.getSelectedRecords()[0].data['name']+'.jpg';
		new Cropper.Img( 
			'testImage',
			{
				onEndCrop: onEndCrop 
			}
		) 
	}
);

function done() {
  effects.hideObject(c); 
}
