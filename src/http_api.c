
#include <string.h>
#include <stdlib.h>
#include "clib/clweb.h"
#include "clib/cJson.h"
#include "wms_db.h"

cJSON* print_voucher(cJSON* jvoucher);
int print_recode(cJSON* jprint_data);

typedef struct mg_context		MG_CTX;
typedef struct mg_connection	MG_CONN;

const char* HTTP_HEADER_JSON		= "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n";
int			HTTP_HEADER_JSON_LEN	= sizeof("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n") - 1;

#define http_write_json(json)     \
		mg_write(conn, HTTP_HEADER_JSON, HTTP_HEADER_JSON_LEN); \
        char* str = cJSON_Print(json);  \
        mg_write(conn, str, strlen(str));   \
        cJSON_free(str);    \
        cJSON_Delete(json); 

#define set_error_go_exit(code,error_str)       ret = code;error=error_str;goto exit;



cJSON* get_items(MG_CONN* conn, char* arg1, char* arg2, char* arg3)
{
	cJSON* json_ret = cJSON_CreateObject();
	cJSON* json_category = db_get_items_category();
	cJSON_AddItemToObject(json_ret, "category", json_category);

	cJSON* json_items = db_get_items();
	cJSON_AddItemToObject(json_ret, "items", json_items);

	cJSON* json_sectors = db_get_sectors();
	cJSON_AddItemToObject(json_ret, "sectors", json_sectors);

	//place
	cJSON* json_places = db_get_places();
	cJSON_AddItemToObject(json_ret, "places", json_places);

	//供应商
	cJSON* son_vendor = db_get_vendors();
	cJSON_AddItemToObject(json_ret, "vendors", son_vendor);

	//当期所有凭证信息

	return json_ret;
}
//不用登录可调用API
int api_get(MG_CONN* conn, void* cbdata)
{
	int ret = 0;
	const char* error = "no";
	char cmd[32] = "";
	char arg1[64] = "";
	char arg2[128] = "";
	char arg3[256] = "";
	cJSON* json_ret = NULL;
	const struct mg_request_info* ri = mg_get_request_info(conn);
	const char* request_str = ri->query_string;
	mg_get_var(request_str, strlen(request_str), "cmd", cmd, sizeof(cmd));
	mg_get_var(request_str, strlen(request_str), "arg1", arg1, sizeof(arg1));
	mg_get_var(request_str, strlen(request_str), "arg2", arg2, sizeof(arg2));
	mg_get_var(request_str, strlen(request_str), "arg3", arg3, sizeof(arg3));
	if (strcmp(cmd, "items") == 0) {
		json_ret = get_items(conn,arg1,arg2,arg3);
	}
	else if (strcmp(cmd, "stock_recodes_last") == 0) {
		json_ret = cJSON_CreateObject();
		cJSON* json_stock_recodes = db_get_stock_recodes_last(atoi(arg1), atoi(arg2));
		cJSON_AddItemToObject(json_ret, "recodes", json_stock_recodes);
	}
	else if (strcmp(cmd, "stock_recodes") == 0) {
		char arg4[256] = "";
		mg_get_var(request_str, strlen(request_str), "arg4", arg4, sizeof(arg4));
		json_ret = cJSON_CreateObject();
		cJSON* json_stock_recodes = db_get_stock_recodes(atoi(arg1), atoi(arg2), atoi(arg3), arg4);
		cJSON_AddItemToObject(json_ret, "recodes", json_stock_recodes);
	}
	else if (strcmp(cmd, "save_vendor") == 0) {
		json_ret = db_save_vendor(arg1, arg2, arg3);
	}
	else if (strcmp(cmd, "remaining") == 0) {
		json_ret = cJSON_CreateObject();
		cJSON* json_recodes = db_get_stock_remaining();
		cJSON_AddItemToObject(json_ret, "recodes", json_recodes);
	}
	else{
		json_ret = cJSON_CreateObject();
		ret = -1;
		error = "unknow cmd";
	}
	cJSON_AddNumberToObject(json_ret,"result",ret);
	cJSON_AddStringToObject(json_ret, "error", error);
	http_write_json(json_ret);
	return 1;
}
//需要登录,通过Json上传数据，查询串也可以带参数
int api_post(MG_CONN* conn, void* cbdata)
{
	const char* error = "no";
	int ret = -1;
	int login = 0;
	char cmd[32] = "";
	char arg1[64] = "";
	char arg2[128] = "";
	char arg3[256] = "";
	unsigned char* buf = NULL;
	cJSON* json_ret = NULL;

	login = 1;
	if (login == 0) {
		set_error_go_exit(-1, "login is need");
	}
	const struct mg_request_info* ri = mg_get_request_info(conn);
	const char* request_str = ri->query_string;
	mg_get_var(request_str, strlen(request_str), "cmd", cmd, sizeof(cmd));
	mg_get_var(request_str, strlen(request_str), "arg1", arg1, sizeof(arg1));
	mg_get_var(request_str, strlen(request_str), "arg2", arg2, sizeof(arg2));
	mg_get_var(request_str, strlen(request_str), "arg3", arg3, sizeof(arg3));

	int buf_size = (int)ri->content_length;
	if (buf_size <= 0 || buf_size > 1024*640) {
		set_error_go_exit(-1, "content length error");
	}
	buf = malloc(buf_size);
	if (buf == NULL) {
		set_error_go_exit(-1, "memory error");
	}
	//取json数据
	int size = mg_read(conn, buf, buf_size);

	if (strcmp(cmd, "add_item") == 0) {
		json_ret = db_add_item(buf, size);
	}
	else if (strcmp(cmd, "save_recode") == 0) {
		json_ret = db_save_recode(buf, size);
	}
	else if (strcmp(cmd, "print_recodes") == 0) {
		cJSON* jprint_data = cJSON_ParseWithLength(buf, buf_size);
		if (jprint_data == NULL) {
			set_error_go_exit(-1, "json string parse error");
		}
		int ret = print_recode(jprint_data);
		cJSON_free(jprint_data);
	}
	else {
		set_error_go_exit(-1, "unknown cmd");
	}

exit:
	if (buf != NULL)
		free(buf);
	if (json_ret == NULL) {
		json_ret = cJSON_CreateObject();
		cJSON_AddNumberToObject(json_ret, "result", ret);
		cJSON_AddStringToObject(json_ret, "error", error);
	}
	cJSON_AddNumberToObject(json_ret, "login", login);
	http_write_json(json_ret);
	return 1;

}
//执行命令API ,有可能会改变数据，通过查询串传参
int api_cmd(MG_CONN* conn, void* cbdata)
{
	char cmd[32] = "";
	char arg1[64] = "";
	char arg2[128] = "";
	char arg3[256] = "";
	cJSON* json_ret = NULL;
	const struct mg_request_info* ri = mg_get_request_info(conn);
	const char* request_str = ri->query_string;
	mg_get_var(request_str, strlen(request_str), "cmd", cmd, sizeof(cmd));
	mg_get_var(request_str, strlen(request_str), "arg1", arg1, sizeof(arg1));
	mg_get_var(request_str, strlen(request_str), "arg2", arg2, sizeof(arg2));
	mg_get_var(request_str, strlen(request_str), "arg3", arg3, sizeof(arg3));
	if (strcmp(cmd, "items") == 0) {
		json_ret = get_items(conn, arg1, arg2, arg3);
	}
	cJSON_AddNumberToObject(json_ret, "result", 0);
	cJSON_AddStringToObject(json_ret, "error", "no");
	http_write_json(json_ret);
	return 1;
}
