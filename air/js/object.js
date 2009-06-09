	var timeline = Ext.air.NativeWindow.getRootHtmlWindow().Ext.getCmp('timeline');

		function onEndCrop( coords, dimensions ) {
  		timeline.addClip(coords.x1 + ' ' + coords.y1 + ' ' + coords.x2 + ' ' + coords.y2);
		}
		
		// basic example
		Event.observe( 
			window, 
			'load', 
			function() { 
				new Cropper.Img( 
					'testImage',
					{
						onEndCrop: onEndCrop 
					}
				) 
			}
		); 		
