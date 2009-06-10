Ext.onReady(function(){
    var store = new Ext.data.JsonStore({
        url: 'http://localhost:8080/MediaLibrary/GetContent',
        root: 'images',
        fields: ['name', 'url']
    });
    store.load();

	var timeline = Ext.air.NativeWindow.getRootHtmlWindow().Ext.getCmp('timeline');

	var importHandler = function(btn) {
    if (dv.getSelectedRecords().length==0) 
      return;
		timeline.addClip(dv.getSelectedRecords()[0].data['name'],dv.getSelectedRecords()[0].data['url']);
		timeline.hideLibrary();
	}

  var uploadHandler = function(btn) {
    Ext.Ajax.request({
    	url : 'http://localhost:8080/MediaLibrary/UploadClip' , 
    	params : { file : fibasic.getValue() },
    	method: 'GET',
    	success: function ( result, request ) { 
        store.load();
    	},
    	failure: function ( result, request) { 
    		Ext.MessageBox.alert('Failed', 'An error occurred');
    	} 
    });
  }

  var splitHandler = function(btn) {
    Ext.Ajax.request({
    	url : 'http://localhost:8080/Effects/AutoSplit' , 
    	params : { 'name' : dv.getSelectedRecords()[0].data['name'] },
    	method: 'GET',
    	success: function ( result, request ) { 
        store.load();
    	},
    	failure: function ( result, request) { 
    		Ext.MessageBox.alert('Failed', 'An error occurred');
    	} 
    });
  }

  var deleteHandler = function(btn) {
    Ext.Ajax.request({
    	url : 'http://localhost:8080/MediaLibrary/RemoveClip' , 
    	params : { 'name' : dv.getSelectedRecords()[0].data['name'] },
    	method: 'GET',
    	success: function ( result, request ) { 
        store.load();
    	},
    	failure: function ( result, request) { 
    		Ext.MessageBox.alert('Failed', 'An error occurred');
    	} 
    });
  }

  $f("flowPlayer", "/flash/flowplayer.swf", { 
/*        clip: { 
            autoPlay: false, 
            autoBuffering: true,
            url: 'http://localhost/smartcut/flv/'+dv.getSelectedRecords()[0].data['name']+'.flv'
        }, */
        plugins: {controls: null} 
    }).controls("flowBar", {}); 


	var dv = new Ext.DataView({
			store: store,
      tpl: Templates.timeline,
			itemSelector:'div.clip',
			multiSelect:true,
      listeners: {
       	selectionchange: {
          fn: function(dv,nodes){
              if (dv.getSelectedRecords().length>0) {
                $f("flowPlayer").stop();
                $f("flowPlayer").setClip('http://localhost/smartcut/flv/'+dv.getSelectedRecords()[0].data['name']+'.flv');
                //v.loadVideo('http://localhost/smartcut/flv/'+dv.getSelectedRecords()[0].data['name']+'.flv');
              }
            }
          }
      }
	});

	var buttons = [{xtype:'button',text:'Import',handler:importHandler},
                {xtype:'button',text:'Cancel',handler:function(){timeline.hideLibrary()}},' ','-',
                {xtype:'button',text:'Auto Split',handler:splitHandler},
                {xtype:'button',text:'Delete',handler:deleteHandler}
                ];

	var clips = new Ext.Panel({title:'Clips',height:350,width:160,items:[dv],autoScroll:true,renderTo:'clips-div'})
  
  var fibasic = new Ext.form.FileUploadField({});
	var upbuttons = [{xtype:'button',text:'Upload',handler:uploadHandler}];
	var up = new Ext.Panel({title:'Clips',width:500,items:[fibasic],bbar:upbuttons,renderTo:'upload-div'})

	//var v = new Ext.air.VideoPanel({renderTo:'player',width:320,height:280,url:''});

	var panel = new Ext.Panel({title:'Movie Library',items:[content],bbar:buttons,renderTo:Ext.getBody()});

	var vp = new Ext.air.ChromeViewport({
				panel: panel
			});
});
