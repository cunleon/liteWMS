
var json_category;//类别
var json_items;//物品
var json_sector;//部门
var json_place;//库位 
var json_vendors;//供应商

function on_item_save_click(btn)
{
  var tr = btn.parentNode.parentNode;
  var id = tr.children[0].innerHTML;
  var cate_name = tr.children[1].firstElementChild.value;
  var name = tr.children[2].firstElementChild.value;

  
  if(id=="添加") id="0";
  var query_str = `arg1=${id}&arg2=${name}&arg3=${phone}`;
  wms_get("save_item",query_str,function (json){
    if(json.result==0)
      swal({icon: "success",title: '',text: '保存成功'});
    wms_get("items","a=1",recoved_items); 
  });
}

function on_vendor_save_click(btn)
{
  var tr = btn.parentNode.parentNode;
  var id = tr.children[0].innerHTML;
  var name = tr.children[1].firstElementChild.value;
  var phone = tr.children[2].firstElementChild.value;
  if(id=="添加") id="0";
  var query_str = `arg1=${id}&arg2=${name}&arg3=${phone}`;
  wms_get("save_vendor",query_str,function (json){
    if(json.result==0)
      swal({icon: "success",title: '',text: '保存成功'});
    wms_get("items","a=1",recoved_items); 
  });
}

function get_place_name(place_id)
{
  var name = "";
  for(index in json_place){
    console.log(json_place[index]);
    if(json_place[index].id == place_id){
      name = json_place[index].name;
      break;
    }
  }
  return name;
}
function init_item_table(items)
{
  var item_table = document.getElementById('item_table');
  item_table.innerHTML="";

  items.forEach(item => {
    var place_name = get_place_name(item.place_id);
    const elementString = `
    <tr>
      <td>${item.id}</td> <td><input type="text" list="category_list" value="${item.cat_name}"/></td> 
      <td><input type="text" value="${item.name}"/></td> <td> <input type="text" value="${item.unit}"/> </td> 
      <td> <input type="text" value="${item.frequency}"/> </td> <td> ${place_name} </td> 
      <td> <input type="text" value="${item.alarm}"/> </td> <td> 查看  修改 </td>
      <td><button onclicked=""><i class="icon-bin2"></i></button> </td>
      <td><button onclick="on_item_save_click(this)"><i class="icon-floppy-disk"></i></button></td>
    </tr>`;
    item_table.innerHTML += elementString;   
  });
  item_table.innerHTML +=`<tr>
      <td>添加</td> <td><input type="text" value="" placeholder="输入供货商名称"></td> 
      <td><input type="text" value="" placeholder="输入供货商电话"></td><td></td> 
      <td><button onclick="on_item_save_click(this)"><i class="icon-floppy-disk"></i> </button></td>
    </tr>`;

}

function init_vendor_table(vendors)
{
  var vendor_table = document.getElementById('vendors_table');
  vendor_table.innerHTML="";

  vendors.forEach(recode => {
    const elementString = `
    <tr>
      <td>${recode.id}</td> <td><input type="text" value="${recode.name}"/></td> 
      <td><input type="text" value="${recode.phone}"/></td> 
      <td>
      <button onclicked=""><i class="icon-bin2"></i></button> 
      </td>
      <td> 
      <button onclick="on_vendor_save_click(this)"><i class="icon-floppy-disk"></i></button>
      </td>
    </tr>`;
    vendor_table.innerHTML += elementString;   
  });
  vendor_table.innerHTML +=`<tr>
      <td>添加</td> <td><input type="text" value="" placeholder="输入供货商名称"></td> 
      <td><input type="text" value="" placeholder="输入供货商电话"></td><td></td> 
      <td><button onclick="on_vendor_save_click(this)"><i class="icon-floppy-disk"></i> </button></td>
    </tr>`;

}

function init_category_table(vendors)
{
  var recodes_table = document.getElementById('category_table');
  recodes_table.innerHTML="";

  vendors.forEach(recode => {
    const elementString = `
    <tr>
      <td>${recode.id}</td> <td><input type="text" value="${recode.name}"/></td> 
      <td>
      <button onclicked=""><i class="icon-bin2"></i> </button> 
      </td>
      <td> 
      <button ><i class="icon-floppy-disk"></i> </button>
      </td>
    </tr>`;
    recodes_table.innerHTML += elementString;   
  });

}

function recoved_items(json)
{
  json_category = json.category;
  json_items = json.items;
  json_sector = json.sectors;
  json_place = json.places;
  json_vendors = json.vendors;

  init_item_table(json.items);
  init_vendor_table(json.vendors);
  init_category_table(json.category);

}

function main_init()
{ 
  wms_get("items","a=1",recoved_items);
}


window.addEventListener('DOMContentLoaded', main_init);
