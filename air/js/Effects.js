var Effects = function(config){
  Effects.superclass.constructor.call(this, config);
	
//  this.on('contextmenu', this.onContextMenu, this);
}

Ext.extend(Effects, Ext.Panel, {
  layout:'anchor',
  id:'effects',
  title:'Effects',
  autoScroll:true,

  initComponent : function(){
    var store = new Ext.data.JsonStore({
        url: 'http://localhost:8080/Effects/GetList',
        root: 'effects',
        fields: ['name', 'display']
    });
    store.load();

    this.dv = new Ext.DataView({
          store: store,
          tpl: Templates.effect,
          itemSelector:'div.effect',
          multiSelect:true
        });
    this.items = [this.dv];
    this.bbar = [{xtype:'button',text:'Apply Effect',handler: this.onApplyPress, scope: this}];

    Effects.superclass.initComponent.call(this);
	},

  onApplyPress: function(btn) {
    recs = this.dv.getSelectedRecords();
    if (recs.length==0) return;
    if (recs[0].data['name']=='objblur') {
      this.object = new Ext.air.NativeWindow({
      		file:'object.html',
      		width:560,
      		height:400
      });
      this.object.show();
      return;
    }
    Ext.getCmp('timeline').addEffect(recs[0]);
  },

  hideObject: function(coord) {
    this.object.hide();
    Ext.getCmp('timeline').addEffectPipe('objblur x1='+coord.x1+' y1='+coord.y1+' x2='+coord.x2+' y2='+coord.y2);
  }
});
