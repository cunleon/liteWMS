// LiteWMS.cpp : 定义应用程序的入口点。
//

//#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>
// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

//#include "resource.h"

//#include <locale.h>
//#include <wrl.h>
//#include "webview2/wil/com.h"
#include "webview2/WebView2.h"

//using namespace Microsoft::WRL;

#pragma comment(lib, "version.lib")
#pragma comment(lib, "webview2/WebView2LoaderStatic.lib")

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
char szTitle[MAX_LOADSTRING] = "LiteWMS 极简仓库管理系统";                  // 标题栏文本
char  szWindowClass[MAX_LOADSTRING]="LiteWMS_WindowsClass";            // 主窗口类名

WCHAR temp_path[1024] = L"";


// Pointer to WebViewController
static ICoreWebView2Controller * webviewController;
// Pointer to WebView window
static ICoreWebView2 * webview;

extern "C" int start_wms(int argc, char** argv);
// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,_In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    char aaa[] = "sssss";
    char bbb[] = "ccccc";
    char* argv[] = {aaa,bbb};
    start_wms(0, argv);
    // TODO: 在此处放置代码。

    GetTempPathW(1024, temp_path);
    lstrcatW(temp_path, L"LiteWMS");

    // 初始化全局字符串
    //LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    //LoadStringW(hInstance, IDC_LITEWMS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = NULL;//LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LITEWMS));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    RemoveDirectoryW(temp_path);
    //可能需要先清空文件
    WCHAR cmd_line[2048] = L"";
    wsprintfW(cmd_line,L"rmdir /s /q %s", temp_path);
    _wsystem(cmd_line);

    return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = 0;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LITEWMS));
    wcex.hCursor        = 0;//LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName = 0;// MAKEINTRESOURCEW(IDC_LITEWMS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm = 0;// LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExA(&wcex);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowA(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   if (!hWnd)
   {
      return FALSE;
   }
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   WCHAR title[128] = L"";
   MultiByteToWideChar(CP_UTF8, 0, "ListWMS 极简仓库管理系统", -1, title, 128);
   SetWindowTextW(hWnd,title);

   //WCHAR temp_path[1024] = L"";
   //GetTempPathW(1024, temp_path);
   //lstrcatW(temp_path, L"LiteWMS\\");
   //
   CreateCoreWebView2EnvironmentWithOptions(nullptr, temp_path, nullptr,
       Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
           [hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

               // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
               env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                   [hWnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                       if (controller != nullptr) {
                           webviewController = controller;
                           webviewController->get_CoreWebView2(&webview);
                       }

                       // Add a few settings for the webview
                       // The demo step is redundant since the values are the default settings
                       wil::com_ptr<ICoreWebView2Settings> settings;
                       webview->get_Settings(&settings);
                       settings->put_IsScriptEnabled(TRUE);
                       settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                       settings->put_IsWebMessageEnabled(TRUE);

                       // Resize WebView to fit the bounds of the parent window
                       RECT bounds;
                       GetClientRect(hWnd, &bounds);
                       webviewController->put_Bounds(bounds);

                       // Schedule an async task to navigate to Bing
                       webview->Navigate(L"http://127.0.0.1:8080/stock.html");

                       // <NavigationEvents>
                       // Step 4 - Navigation events
                       // register an ICoreWebView2NavigationStartingEventHandler to cancel any non-https navigation
                       EventRegistrationToken token;
                       //webview->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
                       //    [](ICoreWebView2* webview, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
                       //        wil::unique_cotaskmem_string uri;
                       //        args->get_Uri(&uri);

                       //        return S_OK;
                       //    }).Get(), &token);
                       // </NavigationEvents>

                       // <Scripting>
                       // Step 5 - Scripting
                       // Schedule an async task to add initialization script that freezes the Object object
                       //webview->AddScriptToExecuteOnDocumentCreated(L"Object.freeze(Object);", nullptr);
                       //// Schedule an async task to get the document URL
                       //webview->ExecuteScript(L"window.document.URL;", Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                       //    [](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
                       //        LPCWSTR URL = resultObjectAsJson;
                       //        //doSomethingWithURL(URL);
                       //        return S_OK;
                       //    }).Get());
                       // </Scripting>

                       // <CommunicationHostWeb>
                       // Step 6 - Communication between host and web content
                       // Set an event handler for the host to return received message back to the web content
                       //webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                       //    [](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                       //        wil::unique_cotaskmem_string message;
                       //        args->TryGetWebMessageAsString(&message);
                       //        // processMessage(&message);
                       //        webview->PostWebMessageAsString(message.get());
                       //        return S_OK;
                       //    }).Get(), &token);

                       // Schedule an async task to add initialization script that
                       // 1) Add an listener to print message from the host
                       // 2) Post document URL to the host
                       //webview->AddScriptToExecuteOnDocumentCreated(
                       //    L"window.chrome.webview.addEventListener(\'message\', event => alert(event.data));" \
                       //    L"window.chrome.webview.postMessage(window.document.URL);",
                       //    nullptr);
                       // </CommunicationHostWeb>

                       return S_OK;
                   }).Get());
               return S_OK;
           }).Get());

   //

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
        if (webviewController != nullptr) {
            RECT bounds;
            GetClientRect(hWnd, &bounds);
            webviewController->put_Bounds(bounds);
        };
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:

                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
