Ext.onReady(function(){
    var store = new Ext.data.JsonStore({
        url: 'http://localhost/ext/examples/view/get-images.php',
        root: 'images',
        fields: ['name', 'url']
    });
    store.load();

	var timeline = Ext.air.NativeWindow.getRootHtmlWindow().Ext.getCmp('timeline');

	var importHandler = function(btn) {
		timeline.addClip('test');
		timeline.hideLibrary();
	}

	var dv = new Ext.DataView({
			store: store,
            tpl: Templates.timeline,
			itemSelector:'div.clip',
			multiSelect:true
	});

	var buttons = [{xtype:'button',text:'Import',handler:importHandler},{xtype:'button',text:'Cancel',handler:function(){timeline.hideLibrary()}}];

	var clips = new Ext.Panel({title:'Clips',height:350,width:160,items:[dv],autoScroll:true})

	var player = new Ext.Panel({title:'Video'});

	var panel = new Ext.Panel({title:'Movie Library',items:[clips,player],layout:'column',bbar:buttons,renderTo:Ext.getBody()});

	var vp = new Ext.air.ChromeViewport({
				panel: panel
			});
});
