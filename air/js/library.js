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

	var clips = new Ext.Panel({title:'Clips',height:350,width:160,items:[dv],autoScroll:true,renderTo:'clips-div'})
  
  var fibasic = new Ext.form.FileUploadField({});
	var upbuttons = [{xtype:'button',text:'Upload'}];
	var up = new Ext.Panel({title:'Clips',width:320,items:[fibasic],bbar:upbuttons,renderTo:'upload-div'})

	var v = new Ext.air.VideoPanel({renderTo:'player',width:320,height:280,url:''});

	var panel = new Ext.Panel({title:'Movie Library',items:[content],bbar:buttons,renderTo:Ext.getBody()});

	var vp = new Ext.air.ChromeViewport({
				panel: panel
			});
});
