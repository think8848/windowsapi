#include <windows.h>
#include <WinInet.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(lib, "Wininet.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   static HINTERNET hInternet = NULL;                                      // Internet初始化句柄
   HINTERNET        hConnect = NULL;                                       // HTTP会话句柄
   HINTERNET        hRequest = NULL;                                       // HTTP请求句柄
   TCHAR     szServerName[MAX_PATH] = { 0 };                               // 主机名或IP地址
   LPCTSTR   arrszAcceptTypes[] = { TEXT("text/*"), TEXT("*/*"), NULL };   // 文档类型
   DWORD     dwNumberOfBytesAvailable = 0;      // InternetQueryDataAvailable函数参数
   LPVOID    lpBuf = NULL;                      // 缓冲区指针
   DWORD     dwNumberOfBytesRead = 0;           // 实际读取到的字节数
   BOOL      bRet = FALSE;                      // InternetReadFile函数返回值
   TCHAR     szFileLocal[MAX_PATH] = { 0 };     // 本地文件名缓冲区
   HANDLE    hFileLocal = INVALID_HANDLE_VALUE; // 本地文件文件句柄

   LPVOID lpBuffer = NULL;                      // HttpQueryInfo函数所用的缓冲区指针
   DWORD  dwBufferLength = 0;                   // 缓冲区的长度
   LPCSTR lpStrOptional =                       // 请求主体
      "username=Admin&password=123456&remember=yes&login=%E7%99%BB%E5%BD%95";

   HINTERNET hFile = NULL;                      // InternetOpenUrl函数返回的Internet文件句柄

   switch (uMsg)
   {
   case WM_INITDIALOG:
      SetDlgItemText(hwndDlg, IDC_EDIT_HOST, TEXT("www.httptest.com"));

      // 打开并初始化WinINet库
      hInternet = InternetOpen(TEXT("Mozilla/5.0"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_GET:
         // 连接到指定站点的HTTP服务，返回一个HTTP会话句柄
         if (GetDlgItemText(hwndDlg, IDC_EDIT_HOST, szServerName, _countof(szServerName)) > 0)
         {
            hConnect = InternetConnect(hInternet, szServerName, INTERNET_DEFAULT_HTTP_PORT,
               NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
         }
         else
         {
            MessageBox(hwndDlg, TEXT("请输入主机名或IP地址"), TEXT("错误提示"), MB_OK);
            return TRUE;
         }

         // 创建一个HTTP请求句柄
         hRequest = HttpOpenRequest(hConnect, TEXT("GET"), TEXT("index.html"), NULL,
            TEXT("http://www.httptest.com/"), arrszAcceptTypes, INTERNET_FLAG_KEEP_CONNECTION, 0);

         // 将一个或多个HTTP请求标头添加到HTTP请求句柄
         HttpAddRequestHeaders(hRequest,
            TEXT("Accept-Encoding:gzip, deflate\r\nAccept-Language:zh-CN\r\n\r\n"), -1,
            HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_COALESCE);

         // 将指定的请求发送到HTTP服务器
         HttpSendRequest(hRequest, NULL, 0, NULL, 0);

         // 创建本地文件
         GetModuleFileName(NULL, szFileLocal, _countof(szFileLocal));
         StringCchCopy(_tcsrchr(szFileLocal, TEXT('\\')) + 1,
            _tcslen(TEXT("index.html")) + 1, TEXT("index.html"));
         hFileLocal = CreateFile(szFileLocal, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

         // 循环读取数据
         do
         {
            dwNumberOfBytesRead = 0;
            bRet = FALSE;

            // 查询服务器上指定文件的可用数据量
            InternetQueryDataAvailable(hRequest, &dwNumberOfBytesAvailable, 0, 0);
            if (dwNumberOfBytesAvailable > 0)
            {
               lpBuf = new BYTE[dwNumberOfBytesAvailable];

               // 从打开的Internet文件中读取数据
               bRet = InternetReadFile(hRequest, lpBuf,
                  dwNumberOfBytesAvailable, &dwNumberOfBytesRead);

               // 写入本地文件
               WriteFile(hFileLocal, lpBuf, dwNumberOfBytesRead, NULL, NULL);

               delete[]lpBuf;
            }
         } while (bRet && dwNumberOfBytesRead > 0);
         MessageBox(hwndDlg, TEXT("下载index.html成功"), TEXT("操作成功"), MB_OK);

         // 关闭本地文件句柄，关闭HTTP请求句柄，关闭HTTP会话句柄
         CloseHandle(hFileLocal);
         InternetCloseHandle(hRequest);
         InternetCloseHandle(hConnect);
         break;

      case IDC_BTN_POST:
         // 连接到指定站点的HTTP服务，返回一个HTTP会话句柄
         if (GetDlgItemText(hwndDlg, IDC_EDIT_HOST, szServerName, _countof(szServerName)) > 0)
         {
            hConnect = InternetConnect(hInternet, szServerName, INTERNET_DEFAULT_HTTP_PORT,
               NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
         }
         else
         {
            MessageBox(hwndDlg, TEXT("请输入主机名或IP地址"), TEXT("错误提示"), MB_OK);
            return TRUE;
         }

         // 创建一个HTTP请求句柄
         hRequest = HttpOpenRequest(hConnect, TEXT("POST"), TEXT("login.php"), NULL,
            TEXT("http://www.httptest.com/login.html"),
            arrszAcceptTypes, INTERNET_FLAG_KEEP_CONNECTION, 0);

         // 将一个或多个HTTP请求标头添加到HTTP请求句柄，POST请求主体长度为68个字节
         HttpAddRequestHeaders(
            hRequest,
            TEXT("Accept-Encoding:gzip, deflate\r\n\
            Accept-Language:zh-CN\r\n\
            Content-Type:application/x-www-form-urlencoded\r\n\
            Content-Length:68\r\n\r\n"),
            -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_COALESCE);

         // 将指定的请求发送到HTTP服务器，附带POST请求主体
         HttpSendRequest(hRequest, NULL, 0, (LPVOID)lpStrOptional, strlen(lpStrOptional));

         // 获取所有请求标头，每个标头都以回车换行分隔
         lpBuffer = NULL;
         dwBufferLength = 0;
         HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF | HTTP_QUERY_FLAG_REQUEST_HEADERS,
            NULL, &dwBufferLength, NULL);
         lpBuffer = new BYTE[dwBufferLength + 2];
         ZeroMemory(lpBuffer, dwBufferLength + 2);
         HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF | HTTP_QUERY_FLAG_REQUEST_HEADERS,
            lpBuffer, &dwBufferLength, NULL);
         MessageBox(hwndDlg, (LPCTSTR)lpBuffer, TEXT("请求标头"), MB_OK);
         delete[]lpBuffer;

         // 获取服务器返回的所有标头，每个标头都以回车换行分隔
         lpBuffer = NULL;
         dwBufferLength = 0;
         HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, NULL, &dwBufferLength, NULL);
         lpBuffer = new BYTE[dwBufferLength + 2];
         ZeroMemory(lpBuffer, dwBufferLength + 2);
         HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, lpBuffer, &dwBufferLength, NULL);
         MessageBox(hwndDlg, (LPCTSTR)lpBuffer, TEXT("响应标头"), MB_OK);
         delete[]lpBuffer;

         // 关闭HTTP请求句柄，关闭HTTP会话句柄
         InternetCloseHandle(hRequest);
         InternetCloseHandle(hConnect);

         //////////////////////////////////////////////////////////////////////////
         //// 连接到指定站点的HTTP服务，返回一个HTTP会话句柄
         //if (GetDlgItemText(hwndDlg, IDC_EDIT_HOST, szServerName, _countof(szServerName)) > 0)
         //{
         //   hConnect = InternetConnect(hInternet, szServerName, INTERNET_DEFAULT_HTTP_PORT,
         //      NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
         //}
         //else
         //{
         //   MessageBox(hwndDlg, TEXT("请输入主机名或IP地址"), TEXT("错误提示"), MB_OK);
         //   return TRUE;
         //}

         //// 创建一个HTTP请求句柄
         //hRequest = HttpOpenRequest(hConnect, TEXT("GET"), TEXT("index.php"), NULL,
         //   TEXT("http://www.httptest.com/login.php"), arrszAcceptTypes, INTERNET_FLAG_KEEP_CONNECTION, 0);

         //// 将一个或多个HTTP请求标头添加到HTTP请求句柄
         //HttpAddRequestHeaders(hRequest,
         //   TEXT("Accept-Encoding:gzip, deflate\r\nAccept-Language:zh-CN\r\n\r\n"), -1,
         //   HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_COALESCE);

         //// 将指定的请求发送到HTTP服务器
         //HttpSendRequest(hRequest, NULL, 0, NULL, 0);

         //// 创建本地文件
         //GetModuleFileName(NULL, szFileLocal, _countof(szFileLocal));
         //StringCchCopy(_tcsrchr(szFileLocal, TEXT('\\')) + 1,
         //   _tcslen(TEXT("index.php")) + 1, TEXT("index.php"));
         //hFileLocal = CreateFile(szFileLocal, GENERIC_READ | GENERIC_WRITE,
         //   FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

         //// 循环读取数据
         //do
         //{
         //   dwNumberOfBytesRead = 0;
         //   bRet = FALSE;

         //   // 查询服务器上指定文件的可用数据量
         //   InternetQueryDataAvailable(hRequest, &dwNumberOfBytesAvailable, 0, 0);
         //   if (dwNumberOfBytesAvailable > 0)
         //   {
         //      lpBuf = new BYTE[dwNumberOfBytesAvailable];

         //      // 从打开的Internet文件中读取数据
         //      bRet = InternetReadFile(hRequest, lpBuf,
         //         dwNumberOfBytesAvailable, &dwNumberOfBytesRead);

         //      // 写入本地文件
         //      WriteFile(hFileLocal, lpBuf, dwNumberOfBytesRead, NULL, NULL);

         //      delete[]lpBuf;
         //   }
         //} while (bRet && dwNumberOfBytesRead > 0);
         //MessageBox(hwndDlg, TEXT("下载index.php成功"), TEXT("操作成功"), MB_OK);

         //// 关闭本地文件句柄，关闭HTTP请求句柄，关闭HTTP会话句柄
         //CloseHandle(hFileLocal);
         //InternetCloseHandle(hRequest);
         //InternetCloseHandle(hConnect);

         //////////////////////////////////////////////////////////////////////////
         // 打开指定URL的HTTP资源
         hFile = InternetOpenUrl(hInternet, TEXT("http://www.httptest.com/index.php"),
            TEXT("Accept-Encoding:gzip, deflate\r\nAccept-Language:zh-CN\r\n\r\n"),
            _tcslen(TEXT("Accept-Encoding:gzip, deflate\r\nAccept-Language:zh-CN\r\n\r\n")),
            INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS |
            INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_AUTH |
            INTERNET_FLAG_RESYNCHRONIZE, 0);

         // 创建本地文件
         GetModuleFileName(NULL, szFileLocal, _countof(szFileLocal));
         StringCchCopy(_tcsrchr(szFileLocal, TEXT('\\')) + 1,
            _tcslen(TEXT("index.php")) + 1, TEXT("index.php"));
         hFileLocal = CreateFile(szFileLocal, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

         // 循环读取数据
         do
         {
            dwNumberOfBytesRead = 0;
            bRet = FALSE;

            // 查询服务器上指定文件的可用数据量
            InternetQueryDataAvailable(hFile, &dwNumberOfBytesAvailable, 0, 0);
            if (dwNumberOfBytesAvailable > 0)
            {
               lpBuf = new BYTE[dwNumberOfBytesAvailable];

               // 从打开的Internet文件中读取数据
               bRet = InternetReadFile(hFile, lpBuf,
                  dwNumberOfBytesAvailable, &dwNumberOfBytesRead);

               // 写入本地文件
               WriteFile(hFileLocal, lpBuf, dwNumberOfBytesRead, NULL, NULL);

               delete[]lpBuf;
            }
         } while (bRet && dwNumberOfBytesRead > 0);
         MessageBox(hwndDlg, TEXT("下载index.php成功"), TEXT("操作成功"), MB_OK);

         // 关闭本地文件句柄，关闭Internet文件句柄
         CloseHandle(hFileLocal);
         InternetCloseHandle(hFile);
         break;
      }
      return TRUE;

   case WM_CLOSE:
      InternetCloseHandle(hInternet);
      EndDialog(hwndDlg, 0);
      return TRUE;
   }

   return FALSE;
}