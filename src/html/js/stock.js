
var json_category;//类别
var json_items;//物品
var json_sector;//部门
var json_place;//库位 

var recode_table_maxid=9999999;


function updata_item_datalist()
{
  wms_get("items","a=1",function (json){
    init_item_datalist(json.items);
  });
} 

//添加物品
function add_item_clk(btn)
{
  var elem_tds = btn.parentNode.parentNode.children;
  var cat_id    = elem_tds[0].firstElementChild.value;
  var place_id  = elem_tds[1].firstElementChild.value;  
  var name      = elem_tds[2].firstElementChild.value;
  var unit      = elem_tds[3].firstElementChild.value;
  var json = new Object();
  json.category_id = parseInt(cat_id);
  json.name = name;
  json.unit = unit;
  json.place_id = parseInt(place_id);
  wms_post("add_item","a=1",JSON.stringify(json),function(json){
    if(json.result == 0){
      showToast('物品添加成功',1000) ;
    }
  });

}

function recode_row_clicked(td)
{
  var tr = td.parentElement;
  var recode_id = tr.children[0].innerHTML;
  var word = tr.children[1].innerHTML;
  var code = tr.children[2].innerHTML;
  var date = tr.children[3].innerHTML;
  var cat_id = tr.children[4].innerHTML;
  var item_id = tr.children[5].innerHTML;
  var item_name = tr.children[6].innerHTML;
  var note = tr.children[7].innerHTML;
  var unit = tr.children[8].innerHTML;
  var stock_in = tr.children[9].innerHTML;
  var stock_out = tr.children[10].innerHTML;
  var amount = tr.children[11].innerHTML;
  var who = tr.children[13].innerHTML;

  document.getElementById('work_date').value = date;
  document.getElementById('voucher_word').value = word;
  document.getElementById('voucher_code').value = code;
  document.getElementById('recode_who').value = who;
  document.getElementById('recode_note').value = note;
  document.getElementById('item_unit').value = unit;

  var cate_input = document.getElementById('item_cate');
  cate_input.value = cat_id;

  var item_select = document.getElementById('item_input');
  item_select.value = item_name;

  document.getElementById('stock_in').value = stock_in;
  document.getElementById('stock_out').value = stock_out;
  document.getElementById('amount').value = amount;

  document.getElementById('recode_id').value = recode_id;

  document.getElementById('save_btn').innerHTML = "保存修改记录";
  document.getElementById('reset_btn').innerHTML = "取消修改";

}

function init_recodes_table(json)
{
  if(json.recodes.length == 0)
    return;
  recode_table_maxid = json.recodes[0].id;//记录当前显示的最大ID

  var recodes_table = document.getElementById('recodes_table');
  recodes_table.innerHTML="";

  json.recodes.forEach(recode => {
    if(recode.stock_in == 0)
      recode.stock_in = "";
    if(recode.stock_out == 0)
      recode.stock_out = "";
    if(recode.amount == 0)
      recode.amount = "";
    const elementString = `
    <tr>
      <td>${recode.id}</td> <td>${recode.word}</td> <td>${recode.code}</td> 
      <td>${recode.date}</td> <td>${recode.cat_name}</td><td>${recode.item_id}</td> 
      <td>${recode.item_name}</td> <td>${recode.note}</td> <td>${recode.unit}</td><td>${recode.stock_in}</td>
      <td>${recode.stock_out}</td> <td>${recode.amount}</td><td>${recode.stock}</td>
      <td>${recode.who}</td><td onclick="recode_row_clicked(this)">编辑</td>
    </tr>`;
    recodes_table.innerHTML += elementString;   
  });
}
//获取出入库记录，参数：月份，最大记录，记录个数
function get_recodes_last(max_id,top)
{
  var query_str = "arg1="+max_id+"&arg2="+top;
  wms_get("stock_recodes_last",query_str,init_recodes_table);
}

function last10_recode_click(btn)
{
  get_recodes_last(9999999,10);
}
function next10_recode_click(btn)
{
  recode_table_maxid -=10;
  if(recode_table_maxid <0){
    swal({icon: "warning",title: '',text: '没有更多记录'});
    return;    
  }
  get_recodes_last(recode_table_maxid,10);
}

function on_word_change(select)
{
  var el_in = document.getElementById('stock_in');
  if(select.value == "入字"){
    el_in.readOnly = false;
  }
  else if(select.value == "出字"){
    el_in.readOnly = true;
  }

}

function reset_inputs()
{
  document.getElementById('item_input').value="";
  document.getElementById('stock_in').value="";
  document.getElementById('stock_out').value="";
  document.getElementById('amount').value="";
  document.getElementById('recode_id').value = 0;
  document.getElementById('recode_id').value = 0;
  document.getElementById('recode_note').value = "";
  document.getElementById('save_btn').innerHTML = "保存出入库记录";

}

//保存当前出入库记录
function save_stock_recode(btn)
{
  var word_date = document.getElementById('work_date').value;
  var voucher_word = document.getElementById('voucher_word').value;
  var voucher_code = document.getElementById('voucher_code').value;
  var who = document.getElementById('recode_who').value;
  var note = document.getElementById('recode_note').value;
  //var item_id = document.getElementById('item_select').value;
  var stock_in = document.getElementById('stock_in').value;
  var stock_out = document.getElementById('stock_out').value;
  var amount = document.getElementById('amount').value;
  var recode_id = document.getElementById('recode_id').value;

  var item_input = document.getElementById('item_input');
  var item_list_options = document.getElementById('item_list').children;
  var item_name = item_input.value;
  var option_length = item_list_options.length;
  
  for(var i=0;i<option_length;i++){
		if(item_name == item_list_options[i].value){
			var item_id = item_list_options[i].getAttribute('item_id');
			break;
		}
	}
  if(voucher_code == ""){
    swal({icon: "error",title: '输入错误',text: '凭证号不能为空'});
    return;
  }
  if(stock_in == 0 && stock_out==0){
    swal({icon: "error",title: '输入错误',text: '出库和入库不能同时为0'});
    return;
  }
  if(item_id == undefined){
    swal({icon: "error",title: '',text: '输入的物品可能不存在，请先添加物品'});
    return;
  }

  var start_date = new Date(word_date);
  var start_date_str = DateToStr(start_date); 
  var json = new Object();
  json.recode_id = parseInt(recode_id);//新记录时为0，修改记录时为原记录ID
  json.date = start_date_str.substring(0, 10);
  json.word = voucher_word;
  json.code = parseInt(voucher_code);
  json.item_id = parseInt(item_id);
  json.who  = who;
  json.note = note;
  json.stock_in = parseInt(stock_in);
  json.stock_out = parseInt(stock_out);
  json.amount = parseInt(amount);


  
  //上传数据//判断返回结果
  wms_post("save_recode","a=1",JSON.stringify(json),function(json){
    if(json.result == 0){
      get_recodes_last(9999999,10);
      swal({icon: "success",title: '',text: '凭证记录成功'});
      reset_inputs();      
    }
  });
}

function on_note_keydown(elem,event)
{
  var row_json = get_row_recode(elem.parentNode.parentNode);
  alert(JSON.stringify(row_json));

}
function on_input_keydown(elem,event)
{
  if (event.keyCode === 13) {
    elem.parentNode.nextElementSibling.firstElementChild.focus();
  }
}
function item_select_change(select)
{
  var index = select.selectedIndex;
  document.getElementById('item_unit').value = select.options[index].getAttribute("unit");
   //select.options[index].unit
  //alert(select.options[index].getAttribute("unit"));
  var input = select.parentNode.nextElementSibling.firstElementChild;
  input.select();  
  input.focus();
}
function cat_select_change(select)
{
  var cat_id = select.value;
  var item_select = select.parentNode.nextElementSibling.firstElementChild
  init_item_select(item_select,json_items,cat_id,""); 
  item_select.focus();
  item_select.click();

  //swal({icon: "success",title: '我是类型 Error',text: 'cat_id:'+cat_id});
  //swal({title: '我是 Title',text: '我是 Text'});
  //swal({title: '我是类型 Warning',type: 'warning',});
  //alert("ddddd"+cat_id);
}

function init_vendor_list(jvendors)
{
  var vendor_list = document.getElementById('vendor_list');
  for(var i=0;i<jvendors.length;i++){
    vendor_list.innerHTML += `<option>${jvendors[i].name}</option>`;
  }

}


function init_category_select(select,joptions,select_id)
{

  select.innerHTML = '<option value="">请选择</option>';
  for(var i=0;i<joptions.length;i++){
    var op = document.createElement("option");
    op.innerHTML = joptions[i].name;
    op.setAttribute("value",joptions[i].id);
    select.appendChild(op); 
    if(select_id == joptions[i].id)  
      op.selected = true;
      //document.getElementById('mySelect').value = '2';
  }
}

function init_item_select(select,joptions,category_id,select_id)
{
  select.innerHTML = '<option value="">请选择</option>';
  for(var i=0;i<joptions.length;i++){
    if(joptions[i].category_id != category_id)
      continue;
    var op = document.createElement("option");
    op.innerHTML = joptions[i].name;
    op.setAttribute("value",joptions[i].id);
    op.setAttribute("unit",joptions[i].unit);
    select.appendChild(op); 
    if(select_id == joptions[i].id)  
      op.selected = true;
      //document.getElementById('mySelect').value = '2';
  }
}

function on_item_input_change(input)
{
  var item_name = input.value;
  var item_list_options = document.getElementById('item_list').children;
  var option_length = item_list_options.length;

  var cat_input = document.getElementById('item_cate');
  var unit_input = document.getElementById('item_unit');
  
  for(var i=0;i<option_length;i++){
		if(item_name == item_list_options[i].value){
			var cat_name = item_list_options[i].getAttribute('cat_name');
      var item_unit = item_list_options[i].getAttribute('unit');
      cat_input.value = cat_name;
      unit_input.value = item_unit;
			break;
		}
	}


}

function init_item_datalist(jitems)
{
  var item_list = document.getElementById('item_list');
  item_list.innerHTML ="";
  for(var i=0;i<jitems.length;i++){
    item_list.innerHTML += `<option value="${jitems[i].name}" cat_name="${jitems[i].cat_name}" item_id="${jitems[i].id}" unit="${jitems[i].unit}">${jitems[i].name} </option>`;
  }
}

function init_stock_table(json)
{
  json_category = json.category;
  json_items = json.items;
  json_sector = json.sectors;
  json_place = json.places;

  document.getElementById('work_date').valueAsDate=new Date();

  //附加初始化添加物品对话框的选择器
  var add_item_dlg_category_select = document.getElementById('add_item_dlg_category_select');
  var add_item_dlg_place_select = document.getElementById('add_item_dlg_place_select');

  init_category_select(add_item_dlg_category_select,json_category,"");
  
  init_vendor_list(json.vendors); 
  init_item_datalist(json.items);
}


function main_init()
{   
  document.getElementById('stock_in').readOnly = true;
  //show_page_doc('/doc/ver.md');           
  //获取系统信息，成功后调用show_sysinfo（json）显示信息
 // show_voucher_list();
  wms_get("items","a=1",init_stock_table);
  get_recodes_last(9999999,10);
}


window.addEventListener('DOMContentLoaded', main_init);
