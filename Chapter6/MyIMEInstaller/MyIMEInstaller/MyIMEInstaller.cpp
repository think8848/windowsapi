#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "Imm32.lib")

// ȫ�ֱ���
HWND  g_hwndDlg;
HKL   g_hklDefault;     // ԭ����Ĭ�����뷨�ļ��̲��־��
HKL   g_hklMy;          // �Լ������뷨�ļ��̲��־��
TCHAR g_szKLID[16];     // �Լ������뷨�ļ��̲��־�����ַ�����ʽ

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID InstallMyIME();    // ��װ���뷨
VOID UninstallMyIME();  // ж�����뷨
VOID ClearMyIME();      // �������뷨

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   static HANDLE hFileMap;
   static LPVOID lpMemory;
   TCHAR         szInjectDllName[MAX_PATH] = { 0 };      // ע��dll����·��
   LPTSTR        lpStr = NULL;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      g_hwndDlg = hwndDlg;

      // ͬĿ¼��ע��dll������·��
      GetModuleFileName(NULL, szInjectDllName, _countof(szInjectDllName));
      if (lpStr = _tcsrchr(szInjectDllName, TEXT('\\')))
         StringCchCopy(lpStr + 1, _tcslen(TEXT("MyIMETestDll.dll")) + 1, TEXT("MyIMETestDll.dll"));

      // ����һ�������ļ�ӳ���ں˶���4096�ֽڣ����ڴ��ע��dll������·��
      hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
         0, 4096, TEXT("DEAE59A6-F81B-4DC4-B375-68437206A1A4"));
      if (!hFileMap)
      {
         MessageBox(hwndDlg, TEXT("CreateFileMapping����ʧ��"), TEXT("��ʾ"), MB_OK);
         return TRUE;
      }

      // ���ļ�ӳ�����hFileMap��ȫ��ӳ�䵽���̵������ַ�ռ���
      lpMemory = MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
      if (!lpMemory)
      {
         MessageBox(hwndDlg, TEXT("MapViewOfFile����ʧ��"), TEXT("��ʾ"), MB_OK);
         return TRUE;
      }

      // ����ע��dll������·�����ڴ�ӳ���ļ�
      StringCchCopy((LPTSTR)lpMemory, MAX_PATH, szInjectDllName);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_INJECT:
         InstallMyIME();
         break;

      case IDC_BTN_EJECT:
         UninstallMyIME();
         break;

      case IDC_BTN_CLEAR:
         ClearMyIME();
         break;
      }
      return TRUE;

   case WM_CLOSE:
      UnmapViewOfFile(lpMemory);
      CloseHandle(hFileMap);
      UninstallMyIME();
      ClearMyIME();
      EndDialog(hwndDlg, 0);
      return TRUE;
   }

   return FALSE;
}

VOID InstallMyIME()
{
   // ����.ime�ļ���ϵͳĿ¼��
   CopyFile(TEXT("MyIME.ime"), TEXT("C:\\WINDOWS\\system32\\MyIME.ime"), FALSE);

   // ��ȡ��ǰĬ�����뷨�ļ��̲��־��(�����������ID��������ID)��ͨ��ȫ�ֱ���g_hklDefault����
   SystemParametersInfo(SPI_GETDEFAULTINPUTLANG, 0, &g_hklDefault, FALSE);

   // ��װ�Լ������뷨(�����ﷵ��ֵΪ0xE0200804)
   g_hklMy = ImmInstallIME(TEXT("MyIME.ime"), TEXT("�ҵ����뷨"));
   StringCchPrintf(g_szKLID, _countof(g_szKLID), TEXT("%08X"), (DWORD)g_hklMy);

   // ����Լ������뷨��װ�ɹ�
   if (ImmIsIME(g_hklMy))
   {
      // �����Լ������뷨��ϵͳ��
      LoadKeyboardLayout(g_szKLID, KLF_ACTIVATE);
      // Ͷ��һ��WM_INPUTLANGCHANGEREQUEST��Ϣ��ǰ̨����(ģ���û�ѡ���µ����뷨)
      PostMessage(GetForegroundWindow(), WM_INPUTLANGCHANGEREQUEST, 
         INPUTLANGCHANGE_SYSCHARSET, (LPARAM)g_hklMy);
      // ����ΪĬ�����뷨
      SystemParametersInfo(SPI_SETDEFAULTINPUTLANG, 0, &g_hklMy, SPIF_SENDCHANGE);
      MessageBox(g_hwndDlg, TEXT("�ҵ����뷨�Ѿ�����ΪĬ�����뷨"), TEXT("��ʾ"), MB_OK);
   }
}

VOID UninstallMyIME()
{
   // �����û�ԭ����Ĭ�����뷨
   SystemParametersInfo(SPI_SETDEFAULTINPUTLANG, 0, &g_hklDefault, SPIF_SENDCHANGE);

   // ж���Լ������뷨
   if (UnloadKeyboardLayout(g_hklMy))
      MessageBox(g_hwndDlg, TEXT("�ҵ����뷨�Ѿ�ж�سɹ�"), TEXT("��ʾ"), MB_OK);
}

VOID ClearMyIME()
{
   // ɾ��HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Keyboard Layouts\E0200804�Ӽ�
   TCHAR szSubKey[MAX_PATH] = TEXT("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts\\");
   StringCchCat(szSubKey, _countof(szSubKey), g_szKLID);
   RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);

   // ɾ��HKEY_CURRENT_USER\Keyboard Layout\Preload�����ֵΪ"E0200804"�ļ�ֵ��
   HKEY    hKey;
   LPCTSTR lpSubKey = TEXT("Keyboard Layout\\Preload");
   DWORD   dwIndex = 0;
   TCHAR   szValueName[16] = { 0 };
   DWORD   dwchValueName;
   TCHAR   szValueData[MAX_PATH] = { 0 };
   DWORD   dwcbValueData;
   LONG    lRet;

   RegOpenKeyEx(HKEY_CURRENT_USER, lpSubKey, 0, KEY_READ | KEY_WRITE, &hKey);
   while (TRUE)
   {
      dwchValueName = _countof(szValueName);
      dwcbValueData = sizeof(szValueData);
      lRet = RegEnumValue(hKey, dwIndex, szValueName, &dwchValueName, NULL, NULL, 
         (LPBYTE)szValueData, &dwcbValueData);
      if (lRet == ERROR_NO_MORE_ITEMS)
         break;

      if (_tcsicmp(g_szKLID, szValueData) == 0)
         RegDeleteValue(hKey, szValueName);

      dwIndex++;
   }

   // ɾ�����뷨�ļ�
   if (!DeleteFile(TEXT("C:\\WINDOWS\\system32\\MyIME.ime")))
   {
      // �´���������ϵͳ�Ժ�ɾ��
      MoveFileEx(TEXT("C:\\WINDOWS\\system32\\MyIME.ime"), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
      MessageBox(g_hwndDlg, TEXT("�ҵ����뷨��������ϣ�������ɾ��.ime�ļ�"), TEXT("��ʾ"), MB_OK);
   }
   else
   {
      MessageBox(g_hwndDlg, TEXT("�ҵ����뷨���������"), TEXT("��ʾ"), MB_OK);
   }
}