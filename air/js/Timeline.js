var Timeline = function(config){
  Timeline.superclass.constructor.call(this, config);
	
  this.on('contextmenu', this.onContextMenu, this);
}

Ext.extend(Timeline, Ext.Panel, {
  layout:'anchor',
  id:'timeline',
  title:'Timeline',
  autoScroll:true,

  initComponent : function(){
    var store = new Ext.data.JsonStore({
        url: 'http://localhost/ext/examples/view/get-images.php',
        root: 'images',
        fields: ['name', 'url']
    });
    store.load();

    var dv = new Ext.DataView({
          store: store,
          tpl: Templates.timeline,
          itemSelector:'div.clip',
          multiSelect:true
        });
    this.items = [dv];
    this.tbar = [{xtype:'button',text:'Play'},{xtype:'button',text:'Stop'},' ','-',
    			{xtype:'button',text:'Add New Clip',handler: this.onAddPress, scope: this},
    			{xtype:'button',text:'Auto Split'},{xtype:'button',text:'Delete'}];

    Timeline.superclass.initComponent.call(this);
    this.library = new Ext.air.NativeWindow({
    		file:'library.html',
    		width:500,
    		height:400,
    		chrome:'none'
    });
	},

	onAddPress: function(btn) {
		this.library.show();
	},

	addClip: function(clip) {
		alert(clip);
	},

	hideLibrary: function() {
		this.library.hide();
	}

});
