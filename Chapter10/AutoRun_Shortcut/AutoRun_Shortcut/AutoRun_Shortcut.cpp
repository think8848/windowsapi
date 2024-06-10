#include <windows.h>
#include <tchar.h>
#include <Shlobj.h>
#include <strsafe.h>
#include "resource.h"

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL MyCreateShortcut(LPTSTR lpszDestFileName, LPTSTR lpszShortcutFileName,
   LPTSTR lpszWorkingDirectory, WORD wHotKey, int iShowCmd, LPTSTR lpszDescription);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   GUID guid = { 0xB97D20BB, 0xF46A, 0x4C97, {0xBA, 0x10, 0x5E, 0x36, 0x08, 0x43, 0x08, 0x54} };
   LPTSTR lpStrStartup;                        // ���ص�ǰ�û��Ŀ����Զ���������Ŀ¼
   TCHAR szDestFileName[MAX_PATH] = { 0 };     // ��ִ���ļ�����·��
   TCHAR szFileName[MAX_PATH] = { 0 };         // ��ִ���ļ�����
   TCHAR szShortcutFileName[MAX_PATH] = { 0 }; // ��ݷ�ʽ�ı���·��

   switch (uMsg)
   {
   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_OK:
         // ��ȡ��ǰ�û��Ŀ����Զ���������Ŀ¼
         SHGetKnownFolderPath(guid, 0, NULL, &lpStrStartup);

         // ��ȡ��ǰ���̵Ŀ�ִ���ļ�����·��
         GetModuleFileName(NULL, szDestFileName, _countof(szDestFileName));

         // ƴ�տ�ݷ�ʽ�ı���·��
         // �����Զ���������Ŀ¼�����һ����б��
         StringCchCopy(szShortcutFileName, _countof(szShortcutFileName), lpStrStartup);
         if (szShortcutFileName[_tcslen(szShortcutFileName) - 1] != TEXT('\\'))
            StringCchCat(szShortcutFileName, _countof(szShortcutFileName), TEXT("\\"));
         // ��ִ���ļ�����.lnk
         StringCchCopy(szFileName, _countof(szFileName), _tcsrchr(szDestFileName, TEXT('\\')) + 1);
         *(_tcsrchr(szFileName, TEXT('.')) + 1) = TEXT('\0');
         StringCchCat(szFileName, _countof(szFileName), TEXT("lnk"));
         // �����Զ���������Ŀ¼\��ִ���ļ�����.lnk
         StringCchCat(szShortcutFileName, _countof(szShortcutFileName), szFileName);

         // �����Զ��庯��MyCreateShortcut������ݷ�ʽ
         MyCreateShortcut(szDestFileName, szShortcutFileName, NULL, 0, 0, NULL);

         CoTaskMemFree(lpStrStartup);
         break;

      case IDCANCEL:
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;
   }

   return FALSE;
}

/*********************************************************************************
  * �������ܣ�		ͨ������COM��ӿں������������ݷ�ʽ
  * ���������˵����
    1. lpszDestFileName������ʾ��ݷ�ʽָ���Ŀ���ļ�·��������ָ��
    2. lpszShortcutFileName������ʾ��ݷ�ʽ�ı���·��(��չ��Ϊ.lnk)������ָ��
    3. lpszWorkingDirectory������ʾ��ʼλ��(����Ŀ¼)���������ΪNULL��ʾ��������Ŀ¼
    4. wHotKey������ʾ��ݼ�������Ϊ0��ʾ�����ÿ�ݼ�
    5. iShowCmd������ʾ���з�ʽ����������ΪSW_SHOWNORMAL��SW_SHOWMINNOACTIVE��SW_SHOWMAXIMIZED
       �ֱ��ʾ���洰�ڡ���С������󻯣�����Ϊ0��ʾ���洰��
    6. lpszDescription������ʾ��ע(����)����������ΪNULL
  * ע�⣺�ú�����Ҫʹ��tchar.h��Shlobj.hͷ�ļ�
**********************************************************************************/
BOOL MyCreateShortcut(LPTSTR lpszDestFileName, LPTSTR lpszShortcutFileName,
   LPTSTR lpszWorkingDirectory, WORD wHotKey, int iShowCmd, LPTSTR lpszDescription)
{
   HRESULT hr;

   if (lpszDestFileName == NULL || lpszShortcutFileName == NULL)
      return FALSE;

   // ��ʼ��COM��
   CoInitializeEx(NULL, 0);

   // ����һ��IShellLink����
   IShellLink* pShellLink;
   hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_SERVER, IID_IShellLink, (LPVOID*)&pShellLink);
   if (FAILED(hr))
      return FALSE;

   // ʹ�÷��ص�IShellLink�����еķ������ÿ�ݷ�ʽ������
   // Ŀ���ļ�·��
   pShellLink->SetPath(lpszDestFileName);

   // ��ʼλ��(����Ŀ¼)
   if (!lpszWorkingDirectory)
   {
      TCHAR szWorkingDirectory[MAX_PATH] = { 0 };
      StringCchCopy(szWorkingDirectory, _countof(szWorkingDirectory), lpszDestFileName);
      LPTSTR lpsz = _tcsrchr(szWorkingDirectory, TEXT('\\'));
      *lpsz = TEXT('\0');
      pShellLink->SetWorkingDirectory(szWorkingDirectory);
   }
   else
   {
      pShellLink->SetWorkingDirectory(lpszWorkingDirectory);
   }

   // ��ݼ�(���ֽڱ�ʾ������룬���ֽڱ�ʾ���μ�)
   if (wHotKey != 0)
      pShellLink->SetHotkey(wHotKey);

   // ���з�ʽ
   if (!iShowCmd)
      pShellLink->SetShowCmd(SW_SHOWNORMAL);
   else
      pShellLink->SetShowCmd(iShowCmd);

   // ��ע(����)
   if (lpszDescription != NULL)
      pShellLink->SetDescription(lpszDescription);


   // ����IShellLink�ĸ���IUnknown�е�QueryInterface������ȡIPersistFile����
   IPersistFile* pPersistFile;
   hr = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
   if (FAILED(hr))
   {
      pShellLink->Release();
      return FALSE;
   }
   // ʹ�û�ȡ����IPersistFile�����е�Save���������ݷ�ʽ��ָ��λ��
   pPersistFile->Save(lpszShortcutFileName, TRUE);

   // �ͷ���ض���
   pPersistFile->Release();
   pShellLink->Release();
   // �ر�COM��
   CoUninitialize();

   return TRUE;
}