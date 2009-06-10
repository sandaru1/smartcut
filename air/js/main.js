Ext.state.Manager.setProvider(new Ext.air.FileProvider({
	file: 'tasks.state',
	// if first time running
	defaultState : {
		mainWindow : {
			width:780,
			height:580,
			x:10,
			y:10
		},
		defaultReminder: 480
	}
}));

Ext.onReady(function(){
    Ext.QuickTips.init();

	// maintain window state automatically
	var win = new Ext.air.NativeWindow({
		id: 'mainWindow',
		instance: window.nativeWindow		
	});

	//var v = new Ext.Panel({title:'Player',renderTo:'player',height:400,items:[flowPlayer]});

	var timeline = new Timeline({renderTo:'timeline'});
  var effects = new Effects({renderTo:'effects'});
	win.show();

  $f("flowPlayer", "/flash/flowplayer.swf", { 
        clip: { 
            autoPlay: false, 
            autoBuffering: true
        }, 
        plugins: {controls: null} 
    }).controls("flowBar", {}); 

/*
  var object = new Ext.air.NativeWindow({
    		file:'object.html',
    		width:500,
    		height:400
    });
  object.show();*/
});
