var entrys_per_page = 30;
var recodes = [];
var current_page = 0;


var json_category;//类别
var json_items;//物品
var json_sector;//部门
var json_place;//库位 
var json_vendors;//供应商



function get_stock_recodes(month,top)
{
  var query_str = "arg1="+month+"&arg2="+top;
  wms_get("stock_recodes",query_str,init_recodes_table);
}
function on_page_prev()
{
  var total_pages = Math.ceil(recodes.length/entrys_per_page);
  if(current_page-1 < 0){
    swal({icon: "warning",title: '',text: '已经是第一页了'});
    return;
  }
  init_recodes_table(recodes,current_page-1);
}
function on_page_next()
{
  var total_pages = Math.ceil(recodes.length/entrys_per_page);
  if(current_page+1 >= total_pages){
    swal({icon: "warning",title: '',text: '已经是最后一页了'});
    return;
  }
  init_recodes_table(recodes,current_page+1);
}

function export_all_clicked()
{
  //alert(JSON.stringify(recodes))
  // 将 JSON 数据转换为二维数组
  const data = [
      ['ID','字','号','日期','分类','物品名称','摘要或备注','单位','入库','出库','总金额','库存','供货商/领用人'] // 表头
  ];

  recodes.forEach(recode => {
    data.push([recode.id,recode.word,recode.code,recode.date,recode.cat_name,recode.item_name,
      recode.note,recode.unit,recode.stock_in,recode.stock_out,recode.amount,recode.stock,recode.who]);
  })

  // 创建工作表
  const ws = XLSX.utils.aoa_to_sheet(data);
  const wb = XLSX.utils.book_new();
  XLSX.utils.book_append_sheet(wb, ws, 'Sheet1');
  // 导出 Excel 文件
  XLSX.writeFile(wb, 'output.xlsx');
}

function print_all_clicked()
{
  var jprint_data= {};
  jprint_data.recodes=recodes;
  jprint_data.page=1000;

  wms_post("print_recodes","a=1",JSON.stringify(jprint_data),function(json){
    if(json.result == 0){
      swal({icon: "success",title: '',text: '成功发送打印命令'});  
    }
  });
}
function init_recodes_table(recodes,page)
{
  if(recodes.length == 0)
    return;
  current_page = page;

  var total_pages = Math.ceil(recodes.length/entrys_per_page);
  //if(recodes.length%entrys_per_page != 0)
  //  total_pages +=1;
  document.getElementById('page_info').innerHTML = `第${page+1}页/共${total_pages}页`;
  

  var recodes_table = document.getElementById('recodes_table');
  recodes_table.innerHTML="";
  var start_index = page*entrys_per_page;
  var end_index = (page+1)*entrys_per_page;
  var page_recodes = recodes.slice(start_index,end_index);

  page_recodes.forEach(recode => {
    if(recode.stock_in == 0)
      recode.stock_in = "";
    if(recode.stock_out == 0)
      recode.stock_out = "";
    if(recode.amount == 0)
      recode.amount = "";
    const elementString = `
    <tr>
      <td>${recode.id}</td> <td>${recode.word}</td> <td>${recode.code}</td> 
      <td>${recode.date}</td> <td>${recode.cat_name}</td>
      <td>${recode.item_name}</td> <td>${recode.note}</td> <td>${recode.unit}</td><td>${recode.stock_in}</td>
      <td>${recode.stock_out}</td> <td>${recode.amount}</td><td>${recode.stock}</td>
      <td>${recode.who}</td>
    </tr>`;
    recodes_table.innerHTML += elementString;   
  });
}
function filter_change()
{
  var recodes_table = document.getElementById('recodes_table');
  recodes_table.innerHTML="";

  var month = document.getElementById('time_select').value;
  var item_id = document.getElementById('item_select').value;
  var stock_in_out = document.getElementById('in_out_select').value;
  var ven_select = document.getElementById('vendor_select');
  var selectedIndex = ven_select.selectedIndex;
  if(selectedIndex == 0){
    var vendor = "";
  }
  else{
    var vendor = ven_select.options[selectedIndex].text;
  }
  
  var query_str = `arg1=${month}&arg2=${item_id}&arg3=${stock_in_out}&arg4=${vendor}`;
  wms_get("stock_recodes",query_str,revoded_recodes);
}
function cat_select_change(select)
{
  var cat_id = select.value;
  var item_select = document.getElementById('item_select');
  init_item_select(item_select,json_items,cat_id,""); 
  //swal({icon: "success",title: '我是类型 Error',text: 'cat_id:'+cat_id});
  //swal({title: '我是类型 Warning',type: 'warning',});
}
function init_category_select(select,joptions,select_id)
{
  select.innerHTML = '<option value="0">所有类别</option>';
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
  select.innerHTML = '<option value="0">所有物品</option>';
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
function init_vendor_select(select,joptions)
{
  select.innerHTML = '<option value="">所有供应商</option>';
  for(var i=0;i<joptions.length;i++){
    var op = document.createElement("option");
    op.innerHTML = joptions[i].name;
    op.setAttribute("value",joptions[i].id);
    select.appendChild(op); 
  }
}

function recoved_items(json)
{
  json_category = json.category;
  json_items = json.items;
  json_sector = json.sectors;
  json_place = json.places;
  json_vendors = json.vendors;

  var curren_date = new Date();
  document.getElementById('time_select').value = curren_date.getMonth()+1;

  var cat_select = document.getElementById('cat_select');
  init_category_select(cat_select,json_category,0)
  var vendor_select = document.getElementById('vendor_select');
  init_vendor_select(vendor_select,json_vendors)

}
function revoded_recodes(json)
{
  recodes = json.recodes;
  current_page = 0;
  init_recodes_table(recodes,0);
}

function main_init()
{ 
  wms_get("items","a=1",recoved_items);
  var curren_date = new Date();
  var month = curren_date.getMonth()+1;
  wms_get("stock_recodes",`arg1=${month}&arg2=0&arg3=0&arg4=`,revoded_recodes);

}


window.addEventListener('DOMContentLoaded', main_init);
 