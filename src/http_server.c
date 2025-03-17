#define _CRT_SECURE_NO_WARNINGS 1 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "clib/clweb.h"
#include "clib/mzip.h"

typedef struct mg_context		MG_CTX;
typedef struct mg_connection	MG_CONN;

int send_mzip_file(MG_CONN* conn, const char* filename);

extern char app_root[1024];
extern char ip_addr[64];
extern char http_root[1024];
extern char http_port[8];
extern char default_db[128];

extern int           http_root_size;
extern unsigned char http_root_data[];

struct _SESSION_T {
	unsigned char used;		//1 已使用
	unsigned char login;
	unsigned char uid;
	unsigned short id;
	long last_time;
};
typedef struct _SESSION_T SESSION_T;
#define MAX_SESSIONS	1024
SESSION_T session_arry[MAX_SESSIONS] = {0};

MG_CTX*	ctx			= NULL;
MZIP_T*	mzip_root	= NULL;		//zip文件或内存做的http_root
int use_mzip = 0;

//Set - Cookie: sessionId = abc123; Expires = Wed, 23 Aug 2024 07:28 : 00 GMT; Secure; HttpOnly

int check_logined(MG_CONN* conn)
{
	int ret = 0;
	const char* cookie = mg_get_header(conn, "Cookie");
	char session_id_str[64];
	ret = mg_get_cookie(cookie, "session_id", session_id_str, sizeof(session_id_str));
	if (ret < 0)
		return 0;
	unsigned int i_session_id = atoi(session_id_str);
	if (i_session_id > 1023)
		return 0;
	session_arry[i_session_id].last_time = time(NULL);
	if (session_arry[i_session_id].login != 1)
		return 0;
	return 1;
}
int set_session(MG_CONN* conn)
{
	//
	short session_id = 0;
	for (int i = 0; i < MAX_SESSIONS; i++) {


	}
	return 0;
}

//不用登录可调用API,获取数据，不会改变数据
int api_get(MG_CONN* conn, void* cbdata);
//需要登录,通过Json上传数据，查询串也可以带参数
int api_post(MG_CONN* conn, void* cbdata);
//执行命令API ,有可能会改变数据，通过查询串传参
int api_cmd(MG_CONN* conn, void* cbdata);

int api_index(MG_CONN* conn, void* cbdata)
{
	//mg_printf(conn, "HTTP/1.1 200 OK\r\nConnection: close\r\n");
	//mg_printf(conn,"Set-Cookie: first=1245\r\n");
	//mg_printf(conn, "Content-Type: text/html\r\n\r\n");

	const struct mg_request_info* req_info = mg_get_request_info(conn);
	const char* cookie = mg_get_header(conn, "Cookie");
	//如果数据库没有打开，发送管理员登录界面
	if (1) {
		//mg_send_http_redirect(conn, "/open.html", 307);
		//return 1;
	}
	else {
		if (use_mzip) {
			return send_mzip_file(conn, "index.html");
		}
	}
	//如果已经登录，重定向到首页
	if (1) {
		mg_send_http_redirect(conn, "/bk/index.html", 307);
		return 1;
	}
	else {
		if (use_mzip) {
			return send_mzip_file(conn, "index.html");
		}
	}


	//const struct mg_request_info* req_info = mg_get_request_info(conn);
	//const char* cookie = mg_get_header(conn, "Cookie");

	return 0;
}
int begin_request(MG_CONN* conn)
{
	//const struct mg_request_info* req_info = mg_get_request_info(conn);
	//const char* cookie = mg_get_header(conn, "Cookie");
	return 0;
}
int log_message(const MG_CONN* conn, const char* message)
{
	return 1;
}
int log_access(const MG_CONN* conn, const char* message)
{
	return 1;
}

int webserver_start()
{
	const char* options[] = {
			"document_root",http_root,
			"listening_ports",http_port,
			"request_timeout_ms","10000",
			"websocket_timeout_ms","3600000",
			"enable_auth_domain_check","no",
			0
	};
	struct mg_callbacks callbacks;

	/* Start CivetWeb web server */
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.begin_request = begin_request;
	callbacks.log_message = log_message;
	callbacks.log_access = log_access;

	ctx = mg_start(&callbacks, 0, options);

	/* Check return value: */
	if (ctx == NULL) {
		fprintf(stderr, "Cannot start CivetWeb - mg_start failed.\n");
		return -1;
	}

	/* Add handler EXAMPLE_URI, to explain the example */
	//mg_set_request_handler(ctx, "/index.html", api_index, 0);
	//mg_set_request_handler(ctx, "/$", api_index, 0);

	mg_set_request_handler(ctx, "/api/get", api_get, 0);
	mg_set_request_handler(ctx, "/api/post", api_post, 0);
	mg_set_request_handler(ctx, "/api/cmd", api_cmd, 0);


#ifdef USE_WEBSOCKET
	/* WS site for the websocket connection */
	mg_set_websocket_handler(ctx,
		"/websocket",
		WebSocketConnectHandler,
		WebSocketReadyHandler,
		WebsocketDataHandler,
		WebSocketCloseHandler,
		0);
#endif

	return 0;

}
int webserver_stop()
{
	mg_stop(ctx);
	return 0;
}
int mzip_root_init()
{
	//char zip_filename[1024] = "";
	//sprintf(zip_filename, "%s/html.zip", app_root);
	//mzip_root = muzip_fopen(zip_filename);
	mzip_root = muzip_mopen(http_root_data, http_root_size);
	return 0;
}
int send_mzip_file(MG_CONN* conn, const char* filename)
{
	//const char* filename = "index.html";
	int file_size = 0;
	char* data = mzip_getfile(mzip_root, filename, &file_size);
	//mime_type
	const char* mime_type = mg_get_builtin_mime_type(filename);

	mg_printf(conn,
		"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nConnection: close\r\n\r\n", mime_type);

	mg_write(conn, data, file_size);

	return 1;
}
