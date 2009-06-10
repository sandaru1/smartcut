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
    this.tbar = [{xtype:'button',text:'Play',handler: this.onPlayPress, scope: this},
          {xtype:'button',text:'Stop',handler: this.onStopPress, scope: this},' ','-',
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

  onStopPress: function(btn) {
    $f("flowPlayer").stop();
  },

  onPlayPress: function(btn) {
    var records = this.dv.store.getRange(0,this.dv.store.getCount());
    var data = new Array();
    for(i=0;i<records.length;i++)
      data.push(records[i].data);
    $f("flowPlayer").stop();
    $f("flowPlayer").setClip('http://localhost/loading.flv');
    this.setTitle("Processing");
    Ext.Ajax.request({
    	url : 'http://localhost:8080/Render', 
    	params : { timeline : Ext.util.JSON.encode(data) },
    	method: 'GET',
    	success: function ( result, request ) { 
        Ext.getCmp("timeline").setTitle("Timeline");
        $f("flowPlayer").setClip('http://localhost/smartcut/flv/final.flv');
        $f("flowPlayer").play();
    	},
    	failure: function ( result, request) { 
    		Ext.MessageBox.alert('Failed', 'An error occurred');
    	} 
    });
  },

  addEffect: function(effect) {
    recs = this.dv.getSelectedRecords();
    if (recs.length==0) return;
    recs[0].data['effects'].push(effect.data['name']);
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
