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
   static TCHAR  szFile[MAX_PATH] = { 0 };      // �����û�ѡ��ı����ļ����Ļ�����
   static TCHAR  szFileTitle[MAX_PATH] = { 0 }; // �����û���ѡ�����ļ����ļ�������չ���Ļ�����
   OPENFILENAME  ofn = { sizeof(OPENFILENAME) };
   ofn.hwndOwner = hwndDlg;
   ofn.lpstrFilter = TEXT("All(*.*)\0*.*\0");
   ofn.lpstrFile = szFile;
   //ofn.lpstrFile[0] = NULL;
   ofn.nMaxFile = _countof(szFile);
   ofn.lpstrFileTitle = szFileTitle;
   ofn.nMaxFileTitle = _countof(szFileTitle);
   ofn.lpstrInitialDir = TEXT("C:\\");
   ofn.lpstrTitle = TEXT("��ѡ��Ҫ�򿪵��ļ�");
   ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_CREATEPROMPT;

   static HINTERNET hInternet = NULL;                           // Internet��ʼ�����
   static HINTERNET hConnect = NULL;                            // FTP�Ự���

   TCHAR            szCurrentDirectory[MAX_PATH] = { 0 };
   DWORD            dwCurrentDirectory = _countof(szCurrentDirectory);

   switch (uMsg)
   {
   case WM_INITDIALOG:
      // �򿪲���ʼ��WinINet��
      hInternet = InternetOpen(TEXT("Mozilla/5.0"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
      // ���ӵ�ָ��վ���FTP��HTTP(��HTTPS)����
      hConnect = InternetConnect(hInternet, TEXT("127.0.0.1"), INTERNET_DEFAULT_FTP_PORT,
         TEXT("admin"), TEXT("123456"), INTERNET_SERVICE_FTP, 0, 0);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_BROWSE:
         if (GetOpenFileName(&ofn))
            SetDlgItemText(hwndDlg, IDC_EDIT_FILE, szFile);
         break;

      case IDC_BTN_PUTFILE:
         // ��ȡ�����ļ�·��
         GetDlgItemText(hwndDlg, IDC_EDIT_FILE, szFile, _countof(szFile));

         // �ϴ������ļ���FTP��������
         FtpPutFile(hConnect, szFile, szFileTitle, FTP_TRANSFER_TYPE_BINARY, 0);
         break;

      case IDC_BTN_GETFILE:
         // ��FTP����������һ���ļ�������ǰĿ¼
         GetModuleFileName(NULL, szFile, _countof(szFile));
         StringCchCopy(_tcsrchr(szFile, TEXT('\\')) + 1, _tcslen(szFileTitle) + 1, szFileTitle);

         FtpGetFile(hConnect, szFileTitle, szFile, FALSE, FILE_ATTRIBUTE_NORMAL,
            FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RESYNCHRONIZE, 0);
         break;

      case IDC_BTN_CREATEDIRECTORY:
         // ��FTP�������ϴ���һ���µ�Ŀ¼
         FtpCreateDirectory(hConnect, TEXT("�½��ļ���"));
         break;

      case IDC_BTN_REMOVEDIRECTORY:
         // ɾ��FTP�������ϵ�һ��Ŀ¼��ע�⣬ֻ��ɾ���ǿ�Ŀ¼���޷�ɾ����ǰĿ¼
         FtpSetCurrentDirectory(hConnect, TEXT("/"));

         FtpRemoveDirectory(hConnect, TEXT("�½��ļ���"));
         break;

      case IDC_BTN_SETDIRECTORY:
         // ����FTP�Ự�ĵ�ǰĿ¼
         FtpSetCurrentDirectory(hConnect, TEXT("�½��ļ���"));

         FtpGetCurrentDirectory(hConnect, szCurrentDirectory, &dwCurrentDirectory);
         SetDlgItemText(hwndDlg, IDC_EDIT_DIRECTORY, szCurrentDirectory);
         break;

      case IDC_BTN_GETDIRECTORY:
         // ��ȡFTP�Ự�ĵ�ǰĿ¼
         FtpGetCurrentDirectory(hConnect, szCurrentDirectory, &dwCurrentDirectory);

         SetDlgItemText(hwndDlg, IDC_EDIT_DIRECTORY, szCurrentDirectory);
         break;
      }
      return TRUE;

   case WM_CLOSE:
      // �ر���ؾ��
      InternetCloseHandle(hConnect);
      InternetCloseHandle(hInternet);
      EndDialog(hwndDlg, 0);
      return TRUE;
   }

   return FALSE;
}