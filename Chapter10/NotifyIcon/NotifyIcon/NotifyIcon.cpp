#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

// ֪ͨ��Ϣ
#define WM_TRAYMSG (WM_APP + 100)

// ȫ�ֱ���
HINSTANCE g_hInstance;
HWND g_hwndDlg;

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   g_hInstance = hInstance;

   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
   POINT pt;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      g_hwndDlg = hwndDlg;

      // Ϊ��������һ��ͼ��
      SendMessage(hwndDlg, WM_SETICON, ICON_BIG,
         (LPARAM)LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_BIRD)));
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case ID_PROGRAM_SHOW:               // ��ʾ �˵���
         ShowWindow(hwndDlg, SW_SHOW);
         SetForegroundWindow(hwndDlg);
         break;

      case ID_PROGRAM_EXIT:               // �˳� �˵���
         SendMessage(hwndDlg, WM_SYSCOMMAND, SC_CLOSE, 0);
         break;

      case IDCANCEL:
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;

   case WM_SYSCOMMAND:
      switch (wParam & 0xFFF0)
      {
      case SC_MINIMIZE:
         // ���֪ͨ����ͼ��
         nid.hWnd = hwndDlg;
         nid.uID = IDI_ICON_BIRD;
         nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_INFO;
         nid.uCallbackMessage = WM_TRAYMSG;
         nid.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_BIRD));
         StringCchCopy(nid.szTip, _countof(nid.szTip), TEXT("NotifyIcon����\n���ߣ�����"));
         StringCchCopy(nid.szInfoTitle, _countof(nid.szInfoTitle), TEXT("����֪ͨ�ı���"));
         StringCchCopy(nid.szInfo, _countof(nid.szInfo), TEXT("����֪ͨ���ı�1\n����֪ͨ���ı�2"));
         Shell_NotifyIcon(NIM_ADD, &nid);

         // ���ش���
         ShowWindow(hwndDlg, SW_HIDE);
         return TRUE;

      case SC_CLOSE:
         // ɾ��֪ͨ����ͼ��
         nid.hWnd = hwndDlg;
         nid.uID = IDI_ICON_BIRD;
         Shell_NotifyIcon(NIM_DELETE, &nid);

         SendMessage(hwndDlg, WM_CLOSE, 0, 0);
         return TRUE;
      }
      return FALSE;

   case WM_TRAYMSG:
      switch (lParam)
      {
      case WM_LBUTTONUP:                  // ���������
          // ��ʾ����
         ShowWindow(hwndDlg, SW_SHOW);
         SetForegroundWindow(hwndDlg);
         break;

      case WM_RBUTTONUP:                  // ����Ҽ����
          // ������ݲ˵�
         GetCursorPos(&pt);
         TrackPopupMenu(GetSubMenu(LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MENU_POPUP)), 0),
            TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, hwndDlg, NULL);
         break;
      }
      return TRUE;
   }

   return FALSE;
}