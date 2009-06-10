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
    var store = new Ext.data.SimpleStore({
        root: 'images',
        fields: ['name', 'url','effects']
    });
    //store.load();

    this.dv = new Ext.DataView({
          store: store,
          tpl: Templates.timeline,
          itemSelector:'div.clip',
          multiSelect:true
        });
    this.items = [this.dv];
    this.tbar = [{xtype:'button',text:'Play'},{xtype:'button',text:'Stop'},' ','-',
    			{xtype:'button',text:'Media Library',handler: this.onAddPress, scope: this},
    			{xtype:'button',text:'Delete', handler: this.onDeletePress, scope: this}];

    Timeline.superclass.initComponent.call(this);
    this.library = new Ext.air.NativeWindow({
    		file:'library.html',
    		width:800,
    		height:400,
    		chrome:'none'
    });
	},

  onDeletePress: function(btn) {
    recs = this.dv.getSelectedRecords();
    if (recs.length==0) return;
    for(var i=0;i<recs.length;i++) {
      this.dv.store.remove(recs[i]);
    }
  },

	onAddPress: function(btn) {
		this.library.show();
	},

	addClip: function(n,url) {
    var clip = Ext.data.Record.create([
    {name: 'name', mapping: 'name'},
    {name: 'url', mapping: 'url'},
    {name: 'effects',mapping: 'effects'}]);

    var newClip = new clip({
    'name': n,
    'url': url,
    'effects':new Array()});
    this.dv.store.add(newClip);
	},

	hideLibrary: function() {
		this.library.hide();
	}

});
