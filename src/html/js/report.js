




function show_remaining_table(json)
{
  var table = document.getElementById('kucun_table');
  json.recodes.forEach(recode => {
    const elementString = `
    <tr>
      <td>${recode.id}</td> <td>${recode.name}</td> <td>${recode.total}</td>
    </tr>`;
    table.innerHTML += elementString;   
  });

}

function main_init()
{ 
  wms_get("remaining","a=1",show_remaining_table);
}

window.addEventListener('DOMContentLoaded', main_init);
