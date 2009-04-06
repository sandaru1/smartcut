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


	//var v = new Ext.air.VideoPanel({renderTo:'player',width:200,height:200,url:'http://localhost/g.flv'});

	var timeline = new Timeline({renderTo:'timeline'});
	win.show();
});
