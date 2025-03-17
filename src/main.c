// liteWMS.cpp :  lite Warehouse Management
//

#define _CRT_SECURE_NO_WARNINGS 1 

#include <stdio.h>
#include <locale.h>
#include <string.h>

//#include "clib/sqlite3.h"
#include "clib/clweb.h"
#include "clib/mzip.h"

int database_open(const char* filename);

int init_config(int argc, char** argv);
int mzip_root_init();
int webserver_start();
int webserver_stop();
int browser_open_url(const char* url);


char app_root[1024] = "";
char ip_addr[64] = "";
char http_root[1024] = "";
char http_port[8] = "8080";
char db_filename[1024] = "";

int start_wms(int argc, char** argv)
{
	setlocale(LC_ALL, ".utf8");
	mg_init_library(0);
	init_config(argc, argv);
	mzip_root_init();
	sprintf(db_filename, "%s/db/wms.db",app_root);
	database_open(db_filename);

	webserver_start();
	return 0;
}

int main(int argc,char ** argv)
{
	setlocale(LC_ALL, ".utf8");
	mg_init_library(0);
	init_config(argc,argv);
	mzip_root_init();

	webserver_start();
	char url[1024] = "";
	sprintf(url,"http://127.0.0.1:%s/",http_port);

	printf("程序正常运行，请不要关闭\n");
	printf("请通过浏览器在本机打开 %s 操作\n",url);
	printf("如需退出程序请按\"x\" +回车 退出本程序\n");

	browser_open_url(url);

	int c = 0;
	while (c!= 'x') {
		c = getchar();
	}

	webserver_stop();//关闭web服务器
					//关闭数据库连接
	printf("app exited.\n");
	printf("Bye!\n");
   return 0;
}




int init_config(int argc,char ** argv)
{
	char* realpath(char* argv0, char* path);
	realpath(argv[0], app_root);
#if _DEBUG
	sprintf(http_root, "%s/../LiteWMS/html", app_root);
#else
	sprintf(http_root, "%s/html", app_root);
#endif

	return 0;
}

#ifdef _WIN32

#include <windows.h>
char* realpath(char* argv0, char* path)
{
	wchar_t wcbuf[1024] = L"";
	GetModuleFileNameW(NULL, wcbuf, 1024);
	WideCharToMultiByte(CP_UTF8, 0, wcbuf, -1, path, 1024, NULL, NULL);
	for (int i = (int)strlen(path); i > 0; i--) {
		if (path[i] == '/' || path[i] == '\\') {
			path[i] = '\0';
			break;
		}
	}
	return path;
}
int browser_open_url(const char* url)
{
	ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
	return 0;
}

#else	//linux
int browser_open_url(const char* url)
{
	return 0;
}
#endif // _WIN32