#include <windows.h>
#include <WinInet.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(lib, "Wininet.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   static HINTERNET hInternet = NULL;                                      // Internet��ʼ�����
   HINTERNET        hConnect = NULL;                                       // HTTP�Ự���
   HINTERNET        hRequest = NULL;                                       // HTTP������
   TCHAR     szServerName[MAX_PATH] = { 0 };                               // ��������IP��ַ
   LPCTSTR   arrszAcceptTypes[] = { TEXT("text/*"), TEXT("*/*"), NULL };   // �ĵ�����
   DWORD     dwNumberOfBytesAvailable = 0;      // InternetQueryDataAvailable��������
   LPVOID    lpBuf = NULL;                      // ������ָ��
   DWORD     dwNumberOfBytesRead = 0;           // ʵ�ʶ�ȡ�����ֽ���
   BOOL      bRet = FALSE;                      // InternetReadFile��������ֵ
   TCHAR     szFileLocal[MAX_PATH] = { 0 };     // �����ļ���������
   HANDLE    hFileLocal = INVALID_HANDLE_VALUE; // �����ļ��ļ����

   LPVOID lpBuffer = NULL;                      // HttpQueryInfo�������õĻ�����ָ��
   DWORD  dwBufferLength = 0;                   // �������ĳ���
   LPCSTR lpStrOptional =                       // ��������
      "username=Admin&password=123456&remember=yes&login=%E7%99%BB%E5%BD%95";

   HINTERNET hFile = NULL;                      // InternetOpenUrl�������ص�Internet�ļ����

   switch (uMsg)
   {
   case WM_INITDIALOG:
      SetDlgItemText(hwndDlg, IDC_EDIT_HOST, TEXT("www.httptest.com"));

      // �򿪲���ʼ��WinINet��
      hInternet = InternetOpen(TEXT("Mozilla/5.0"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_GET:
         // ���ӵ�ָ��վ���HTTP���񣬷���һ��HTTP�Ự���
         if (GetDlgItemText(hwndDlg, IDC_EDIT_HOST, szServerName, _countof(szServerName)) > 0)
         {
            hConnect = InternetConnect(hInternet, szServerName, INTERNET_DEFAULT_HTTP_PORT,
               NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
         }
         else
         {
            MessageBox(hwndDlg, TEXT("��������������IP��ַ"), TEXT("������ʾ"), MB_OK);
            return TRUE;
         }

         // ����һ��HTTP������
         hRequest = HttpOpenRequest(hConnect, TEXT("GET"), TEXT("index.html"), NULL,
            TEXT("http://www.httptest.com/"), arrszAcceptTypes, INTERNET_FLAG_KEEP_CONNECTION, 0);

         // ��һ������HTTP�����ͷ��ӵ�HTTP������
         HttpAddRequestHeaders(hRequest,
            TEXT("Accept-Encoding:gzip, deflate\r\nAccept-Language:zh-CN\r\n\r\n"), -1,
            HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_COALESCE);

         // ��ָ���������͵�HTTP������
         HttpSendRequest(hRequest, NULL, 0, NULL, 0);

         // ���������ļ�
         GetModuleFileName(NULL, szFileLocal, _countof(szFileLocal));
         StringCchCopy(_tcsrchr(szFileLocal, TEXT('\\')) + 1,
            _tcslen(TEXT("index.html")) + 1, TEXT("index.html"));
         hFileLocal = CreateFile(szFileLocal, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

         // ѭ����ȡ����
         do
         {
            dwNumberOfBytesRead = 0;
            bRet = FALSE;

            // ��ѯ��������ָ���ļ��Ŀ���������
            InternetQueryDataAvailable(hRequest, &dwNumberOfBytesAvailable, 0, 0);
            if (dwNumberOfBytesAvailable > 0)
            {
               lpBuf = new BYTE[dwNumberOfBytesAvailable];

               // �Ӵ򿪵�Internet�ļ��ж�ȡ����
               bRet = InternetReadFile(hRequest, lpBuf,
                  dwNumberOfBytesAvailable, &dwNumberOfBytesRead);

               // д�뱾���ļ�
               WriteFile(hFileLocal, lpBuf, dwNumberOfBytesRead, NULL, NULL);

               delete[]lpBuf;
            }
         } while (bRet && dwNumberOfBytesRead > 0);
         MessageBox(hwndDlg, TEXT("����index.html�ɹ�"), TEXT("�����ɹ�"), MB_OK);

         // �رձ����ļ�������ر�HTTP���������ر�HTTP�Ự���
         CloseHandle(hFileLocal);
         InternetCloseHandle(hRequest);
         InternetCloseHandle(hConnect);
         break;

      case IDC_BTN_POST:
         // ���ӵ�ָ��վ���HTTP���񣬷���һ��HTTP�Ự���
         if (GetDlgItemText(hwndDlg, IDC_EDIT_HOST, szServerName, _countof(szServerName)) > 0)
         {
            hConnect = InternetConnect(hInternet, szServerName, INTERNET_DEFAULT_HTTP_PORT,
               NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
         }
         else
         {
            MessageBox(hwndDlg, TEXT("��������������IP��ַ"), TEXT("������ʾ"), MB_OK);
            return TRUE;
         }

         // ����һ��HTTP������
         hRequest = HttpOpenRequest(hConnect, TEXT("POST"), TEXT("login.php"), NULL,
            TEXT("http://www.httptest.com/login.html"),
            arrszAcceptTypes, INTERNET_FLAG_KEEP_CONNECTION, 0);

         // ��һ������HTTP�����ͷ��ӵ�HTTP��������POST�������峤��Ϊ68���ֽ�
         HttpAddRequestHeaders(
            hRequest,
            TEXT("Accept-Encoding:gzip, deflate\r\n\
            Accept-Language:zh-CN\r\n\
            Content-Type:application/x-www-form-urlencoded\r\n\
            Content-Length:68\r\n\r\n"),
            -1, HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_COALESCE);

         // ��ָ���������͵�HTTP������������POST��������
         HttpSendRequest(hRequest, NULL, 0, (LPVOID)lpStrOptional, strlen(lpStrOptional));

         // ��ȡ���������ͷ��ÿ����ͷ���Իس����зָ�
         lpBuffer = NULL;
         dwBufferLength = 0;
         HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF | HTTP_QUERY_FLAG_REQUEST_HEADERS,
            NULL, &dwBufferLength, NULL);
         lpBuffer = new BYTE[dwBufferLength + 2];
         ZeroMemory(lpBuffer, dwBufferLength + 2);
         HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF | HTTP_QUERY_FLAG_REQUEST_HEADERS,
            lpBuffer, &dwBufferLength, NULL);
         MessageBox(hwndDlg, (LPCTSTR)lpBuffer, TEXT("�����ͷ"), MB_OK);
         delete[]lpBuffer;

         // ��ȡ���������ص����б�ͷ��ÿ����ͷ���Իس����зָ�
         lpBuffer = NULL;
         dwBufferLength = 0;
         HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, NULL, &dwBufferLength, NULL);
         lpBuffer = new BYTE[dwBufferLength + 2];
         ZeroMemory(lpBuffer, dwBufferLength + 2);
         HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, lpBuffer, &dwBufferLength, NULL);
         MessageBox(hwndDlg, (LPCTSTR)lpBuffer, TEXT("��Ӧ��ͷ"), MB_OK);
         delete[]lpBuffer;

         // �ر�HTTP���������ر�HTTP�Ự���
         InternetCloseHandle(hRequest);
         InternetCloseHandle(hConnect);

         //////////////////////////////////////////////////////////////////////////
         //// ���ӵ�ָ��վ���HTTP���񣬷���һ��HTTP�Ự���
         //if (GetDlgItemText(hwndDlg, IDC_EDIT_HOST, szServerName, _countof(szServerName)) > 0)
         //{
         //   hConnect = InternetConnect(hInternet, szServerName, INTERNET_DEFAULT_HTTP_PORT,
         //      NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
         //}
         //else
         //{
         //   MessageBox(hwndDlg, TEXT("��������������IP��ַ"), TEXT("������ʾ"), MB_OK);
         //   return TRUE;
         //}

         //// ����һ��HTTP������
         //hRequest = HttpOpenRequest(hConnect, TEXT("GET"), TEXT("index.php"), NULL,
         //   TEXT("http://www.httptest.com/login.php"), arrszAcceptTypes, INTERNET_FLAG_KEEP_CONNECTION, 0);

         //// ��һ������HTTP�����ͷ��ӵ�HTTP������
         //HttpAddRequestHeaders(hRequest,
         //   TEXT("Accept-Encoding:gzip, deflate\r\nAccept-Language:zh-CN\r\n\r\n"), -1,
         //   HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_COALESCE);

         //// ��ָ���������͵�HTTP������
         //HttpSendRequest(hRequest, NULL, 0, NULL, 0);

         //// ���������ļ�
         //GetModuleFileName(NULL, szFileLocal, _countof(szFileLocal));
         //StringCchCopy(_tcsrchr(szFileLocal, TEXT('\\')) + 1,
         //   _tcslen(TEXT("index.php")) + 1, TEXT("index.php"));
         //hFileLocal = CreateFile(szFileLocal, GENERIC_READ | GENERIC_WRITE,
         //   FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

         //// ѭ����ȡ����
         //do
         //{
         //   dwNumberOfBytesRead = 0;
         //   bRet = FALSE;

         //   // ��ѯ��������ָ���ļ��Ŀ���������
         //   InternetQueryDataAvailable(hRequest, &dwNumberOfBytesAvailable, 0, 0);
         //   if (dwNumberOfBytesAvailable > 0)
         //   {
         //      lpBuf = new BYTE[dwNumberOfBytesAvailable];

         //      // �Ӵ򿪵�Internet�ļ��ж�ȡ����
         //      bRet = InternetReadFile(hRequest, lpBuf,
         //         dwNumberOfBytesAvailable, &dwNumberOfBytesRead);

         //      // д�뱾���ļ�
         //      WriteFile(hFileLocal, lpBuf, dwNumberOfBytesRead, NULL, NULL);

         //      delete[]lpBuf;
         //   }
         //} while (bRet && dwNumberOfBytesRead > 0);
         //MessageBox(hwndDlg, TEXT("����index.php�ɹ�"), TEXT("�����ɹ�"), MB_OK);

         //// �رձ����ļ�������ر�HTTP���������ر�HTTP�Ự���
         //CloseHandle(hFileLocal);
         //InternetCloseHandle(hRequest);
         //InternetCloseHandle(hConnect);

         //////////////////////////////////////////////////////////////////////////
         // ��ָ��URL��HTTP��Դ
         hFile = InternetOpenUrl(hInternet, TEXT("http://www.httptest.com/index.php"),
            TEXT("Accept-Encoding:gzip, deflate\r\nAccept-Language:zh-CN\r\n\r\n"),
            _tcslen(TEXT("Accept-Encoding:gzip, deflate\r\nAccept-Language:zh-CN\r\n\r\n")),
            INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS |
            INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_AUTH |
            INTERNET_FLAG_RESYNCHRONIZE, 0);

         // ���������ļ�
         GetModuleFileName(NULL, szFileLocal, _countof(szFileLocal));
         StringCchCopy(_tcsrchr(szFileLocal, TEXT('\\')) + 1,
            _tcslen(TEXT("index.php")) + 1, TEXT("index.php"));
         hFileLocal = CreateFile(szFileLocal, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

         // ѭ����ȡ����
         do
         {
            dwNumberOfBytesRead = 0;
            bRet = FALSE;

            // ��ѯ��������ָ���ļ��Ŀ���������
            InternetQueryDataAvailable(hFile, &dwNumberOfBytesAvailable, 0, 0);
            if (dwNumberOfBytesAvailable > 0)
            {
               lpBuf = new BYTE[dwNumberOfBytesAvailable];

               // �Ӵ򿪵�Internet�ļ��ж�ȡ����
               bRet = InternetReadFile(hFile, lpBuf,
                  dwNumberOfBytesAvailable, &dwNumberOfBytesRead);

               // д�뱾���ļ�
               WriteFile(hFileLocal, lpBuf, dwNumberOfBytesRead, NULL, NULL);

               delete[]lpBuf;
            }
         } while (bRet && dwNumberOfBytesRead > 0);
         MessageBox(hwndDlg, TEXT("����index.php�ɹ�"), TEXT("�����ɹ�"), MB_OK);

         // �رձ����ļ�������ر�Internet�ļ����
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