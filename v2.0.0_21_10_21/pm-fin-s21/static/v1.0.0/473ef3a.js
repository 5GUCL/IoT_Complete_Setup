(window.webpackJsonp=window.webpackJsonp||[]).push([[5],{520:function(t,e,n){var content=n(523);content.__esModule&&(content=content.default),"string"==typeof content&&(content=[[t.i,content,""]]),content.locals&&(t.exports=content.locals);(0,n(19).default)("4c46b74a",content,!0,{sourceMap:!1})},522:function(t,e,n){"use strict";n(520)},523:function(t,e,n){var o=n(18)(!1);o.push([t.i,".disable-events[data-v-a40b5ea6]{pointer-events:none}",""]),t.exports=o},535:function(t,e,n){"use strict";n.r(e);var o={props:{channel:{type:String,required:!0,default:""},ncap:{type:String,required:!0,default:""},description:{type:String,required:!0,default:""}},data:function(){return{sioName:"s21-sio",sioChannel:"",ncapId:"",dialogDelete:!1,sioConnStatus:{},sioS21Conn:!1,sio_conn_status_ind:"red lighten-1",headers:[{text:"#",value:"sl_num",align:"start",sortable:!1},{text:"Event",value:"event",sortable:!1},{text:"IoT Device",value:"device",sortable:!1},{text:"Net Quantity",value:"quantity",sortable:!1},{text:"Time",value:"time",sortable:!1},{text:"Transaction",value:"transaction",sortable:!1,color:"success"}],iot_transactions:[],txn_counter:1}},computed:{getQuantitySlotControls:function(){return"item.quantity"},getTxnSlotControls:function(){return"item.transaction"}},watch:{sioConnStatus:function(t){this.sio_conn_status_ind="ping"===t||"pong"===t||"connect"===t||"connected"===t?"success":"red lighten-1"},dialogDelete:function(t){t||this.closeDelete()}},mounted:function(){var t=this;this.socket=this.$nuxtSocket({name:this.sioName,channel:"/"+this.channel,namespaceCfg:{emitters:[],listeners:[],emitBacks:[]},statusProp:"sioConnStatus",reconnection:!0,teardown:!0}),this.socket.on("connect",(function(e,n){t.sio_conn_status_ind="success",t.sioS21Conn=!0,n&&n()})),this.socket.on("disconnect",(function(){t.sio_conn_status_ind="red lighten-1",t.sioS21Conn=!1})),this.socket.on("s21_rx_event",(function(e,n){var o;o=200===e.s21_txn_context.status_code?"green accent-4":"deep-orange accent-3";var r={sl_num:t.txn_counter++,event:e.s21_rx_event,device:e.s21_rx_event_data.ncap_id,quantity:e.s21_rx_event_data.net_quantity,time:e.s21_rx_event_data.time_instance,transaction:e.s21_txn_context.status_str,txn_context:{color:o,server:e.s21_txn_context}};t.iot_transactions.unshift(r),n&&n()}))},created:function(){},methods:{clearData:function(){this.iot_transactions=[],this.txn_counter=1,this.dialogDelete=!1},closeDelete:function(){this.dialogDelete=!1},getQuantityColor:function(t){return t<=1e3?"red accent-1":t>1e3&&t<=2e3?"teal lighten-2":t>2e3&&t<=3e3?"green darken-1":t>3e3&&t<=4e3?"orange darken-2":"brown darken-1"},getTxnContextColor:function(t){return t.txn_context.color}}},r=(n(522),n(98)),c=n(141),l=n.n(c),d=n(251),v=n(217),_=n(116),f=n(531),x=n(618),h=n(511),m=n(496),C=n(216),k=n(218),y=n(514),D=n(61),w=n(231),component=Object(r.a)(o,(function(){var t=this,e=t.$createElement,n=t._self._c||e;return n("div",[n("v-data-table",{attrs:{headers:t.headers,items:t.iot_transactions,"footer-props":{showFirstLastPage:!0,firstIcon:"mdi-arrow-collapse-left",lastIcon:"mdi-arrow-collapse-right",prevIcon:"mdi-minus",nextIcon:"mdi-plus"}},scopedSlots:t._u([{key:"top",fn:function(){return[n("v-toolbar",{attrs:{flat:""}},[n("v-toolbar-title",[t._v("FIELD DATA")]),t._v(" "),n("v-divider",{staticClass:"mx-4",attrs:{inset:"",vertical:""}}),t._v(" "),n("div",{staticClass:"text-center"},[n("v-btn",{staticClass:"mx-2",attrs:{fab:"",dark:"",small:"",color:"deep-orange lighten-1"},on:{click:function(e){t.dialogDelete=!0}}},[n("v-icon",[t._v("\n              mdi-delete\n            ")])],1)],1),t._v(" "),n("v-dialog",{attrs:{"max-width":"500px"},model:{value:t.dialogDelete,callback:function(e){t.dialogDelete=e},expression:"dialogDelete"}},[n("v-card",[n("v-card-title",{staticClass:"text-h5"},[t._v("\n              Are you sure you want to delete the data?\n            ")]),t._v(" "),n("v-card-actions",[n("v-spacer"),t._v(" "),n("v-btn",{attrs:{color:"green darken-1",text:""},on:{click:t.closeDelete}},[t._v("\n                Cancel\n              ")]),t._v(" "),n("v-btn",{attrs:{color:"warning darken-1",text:""},on:{click:t.clearData}},[t._v("\n                OK\n              ")]),t._v(" "),n("v-spacer")],1)],1)],1),t._v(" "),n("v-spacer"),t._v(" "),n("v-toolbar-title",[t._v(" "+t._s(t.ncap)+" ")]),t._v(" "),n("v-divider",{staticClass:"mx-6",attrs:{inset:"",vertical:""}}),t._v(" "),n("div",{staticClass:"text-center"},[n("v-btn",{staticClass:"disable-events",attrs:{fab:"",dark:"",small:"",color:"grey lighten-4"}},[n("v-icon",{staticClass:"mx-auto",attrs:{color:t.sio_conn_status_ind,size:"32"}},[t._v("\n              mdi-wifi-strength-4\n            ")])],1)],1)],1),t._v(" "),n("v-divider"),t._v(" "),n("v-card",{attrs:{flat:""}},[n("v-card-title",{staticClass:"grey--text font-weight-light"},[t._v("\n          "+t._s(t.description)+"\n        ")])],1),t._v(" "),n("v-progress-linear",{attrs:{"buffer-value":"0",stream:"",color:t.sio_conn_status_ind}})]},proxy:!0},{key:t.getQuantitySlotControls,fn:function(e){var o=e.item;return[n("v-chip",{attrs:{color:t.getQuantityColor(o.quantity),dark:""}},[t._v("\n        "+t._s(o.quantity)+"\n      ")])]}},{key:t.getTxnSlotControls,fn:function(e){var o=e.item;return[n("v-chip",{attrs:{dark:""}},[n("v-icon",{staticClass:"pa-2",attrs:{left:"",color:t.getTxnContextColor(o)}},[t._v("\n          mdi-contactless-payment\n        ")]),t._v("\n        "+t._s(o.transaction)+"\n      ")],1)]}}],null,!0)})],1)}),[],!1,null,"a40b5ea6",null);e.default=component.exports;l()(component,{VBtn:d.a,VCard:v.a,VCardActions:_.a,VCardTitle:_.b,VChip:f.a,VDataTable:x.a,VDialog:h.a,VDivider:m.a,VIcon:C.a,VProgressLinear:k.a,VSpacer:y.a,VToolbar:D.a,VToolbarTitle:w.a})}}]);