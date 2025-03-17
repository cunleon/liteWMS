
//显示底部版权信息
function show_copyright()
{
    document.title = '极简仓库管理系统'
    document.getElementById("copyright_div").innerHTML="Copyright © 2024 磁县阜才中学&索存良";
}


//生成签名验证字符串
function get_sign_string()
{
    var username = localStorage.getItem("username"); 
    var password = localStorage.getItem("password");
    let md5_1 = md5(password); 
    var req_str = "u="+username+"&p="+md5_1;
    return req_str;
}

function on_sign_out()
{
    localStorage.removeItem("username");
    localStorage.removeItem("password");
    window.location.href="/sign-in.html";
}

//显示提示信息
function showToast(msg,duration)
{  
    duration=isNaN(duration)?3000:duration;  
    var m = document.createElement('div');  
    m.innerHTML = msg;  
    m.style.cssText=
    `position: absolute; top:30%; left:20%; z-index:999999;
    display:flex;align-items:center;justify-content:center;text-align:justify;
    width:60%; min-width:180px;
    background:#000; opacity:0.8;
    height:auto;min-height: 80px;
    color:#fff; line-height:30px; text-align:center; border-radius:4px;`;  
    
    document.body.appendChild(m);  
    setTimeout(function() {  
        var d = 0.5;  
        m.style.webkitTransition = '-webkit-transform ' + d + 's ease-in, opacity ' + d + 's ease-in';  
        m.style.opacity = '0';  
        setTimeout(function() { document.body.removeChild(m) }, d * 1000);  
    }, duration);  
}  


function show_page_doc(md)
{
  $.get(md, function (data) {
    document.getElementById("page-doc").innerHTML = marked.parse(data);
  });
}

function wms_get(cmd,query,func)
{
  var url = "/api/get?cmd="+ cmd +"&"+ query;
  $.getJSON(url, function (json) {
      if(json.result == 0){
        func(json);
      }
      else{
        alert("get "+cmd+"失败："+json.error);
      }
  });

}


//修改数据操作带上传json数据
function wms_post(cmd,query,post_data,func)
{
  //var sign_str = get_sign_string();
  var post_url = "/api/post?cmd="+cmd+"&"+query;//+"&"+sign_str;
  //ajax上传
  $.ajax({
      url:post_url,
      type:'post',
      data:post_data,//JSON.stringify(json),
      contentType:'application/json',
      dataType:'json',
      success:function (json) {
        if(json.login == 0){
          window.location.href = '/sign-in.html';           
          return;    
        }
        if(json.result != 0)
          showToast(json.error,1000);
        else
          func(json);
      }
  });

}

function wms_get_sidebar()
{
  $.get( "/inc/side-nav.html", function( html ) {
    document.getElementById('layoutSidenav_nav').innerHTML = html;
    sidebar_active();
  });
}

window.addEventListener('DOMContentLoaded', event => {
  show_copyright();
  wms_get_sidebar();
 
  const sidebarToggle = document.body.querySelector('#sidebarToggle');
  if (sidebarToggle) {

      sidebarToggle.addEventListener('click', event => {
          event.preventDefault();
          document.body.classList.toggle('sb-sidenav-toggled');
          localStorage.setItem('sb|sidebar-toggle', document.body.classList.contains('sb-sidenav-toggled'));
      });
  }
});


 //日期转字符串格式
 function DateToStr(date) 
 {
  var year = date.getFullYear();//年
  var month = date.getMonth();//月
  var day = date.getDate();//日
  var hours = 0;//date.getHours();//时
  var min = 0;//date.getMinutes();//分
  var second = 0;//date.getSeconds();//秒
  return year + "-" +
      ((month + 1) > 9 ? (month + 1) : "0" + (month + 1)) + "-" +
      (day > 9 ? day : ("0" + day)) + " " +
      (hours > 9 ? hours : ("0" + hours)) + ":" +
      (min > 9 ? min : ("0" + min)) + ":" +
      (second > 9 ? second : ("0" + second));
 }

 function sidebar_active()
 {
   url = window.location.pathname;
   if(url=="/")
     url="/index.shtml";
   arriteb = '[href="'+ url+'"]';      
   parent = document.getElementById('nav-box');
   var children = parent.children;

   for(i=0;i<children.length;i++){
      //alert(children[i].getAttribute("href"));
      if(children[i].getAttribute("href") == url)
        children[i].setAttribute('class', 'nav-link nav-link-select');
   }
 }