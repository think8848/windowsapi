#include <windows.h>
#include <TlHelp32.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ȫ�ֱ���
HINSTANCE g_hInstance;

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// ��ȡĿ�괰�ھ��
HWND SmallestWindowFromPoint(POINT pt);
// ��ȡ������ID
DWORD GetParentProcessIDByID(DWORD dwProcessId);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   g_hInstance = hInstance;

   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   static HCURSOR hCursorDrag;     // �϶�ʱ�Ĺ����
   static HICON hIconNormal;       // ���������ͼ��̬�ؼ����õ�ͼ����
   static HICON hIconDrag;         // �϶�ʱ��ͼ��̬�ؼ����õ�ͼ����
   static HWND hwndTarget;         // Ŀ�괰�ھ��
   static HDC hdcDesk;             // �����豸���������������Ŀ�괰����Χ������������
   RECT rect;
   POINT pt;
   DWORD dwProcessID, dwParentProcessID, dwCtrlID;
   TCHAR szBuf[128] = { 0 };
   LPTSTR lpBuf = NULL;
   int nLen;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      // Ϊ�Ի���������Ͻ�����һ��ͼ��
      SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(g_hInstance,
         MAKEINTRESOURCE(IDI_ICON_MAIN)));

      // �϶�ʱ�Ĺ��������������º��϶�ʱ��ͼ��̬�ؼ����õ�ͼ����
      hCursorDrag = LoadCursor(g_hInstance, MAKEINTRESOURCE(IDC_CURSOR_DRAG));
      hIconNormal = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_NORMAL));
      hIconDrag = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_DRAG));

      // �����豸������������Ŀ�괰����Χ������������
      hdcDesk = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
      SelectObject(hdcDesk, CreatePen(PS_SOLID, 2, RGB(255, 0, 255)));
      SetROP2(hdcDesk, R2_NOTXORPEN);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_CHK_TOPMOST:
         // �����ö�
         if (IsDlgButtonChecked(hwndDlg, IDC_CHK_TOPMOST) == BST_CHECKED)
            SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
         else
            SetWindowPos(hwndDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
         break;

      case IDC_BTN_MODIFYTITLE:
         // �޸ı���
         nLen = SendMessage(GetDlgItem(hwndDlg, IDC_EDIT_WINDOWTITLE), WM_GETTEXTLENGTH, 0, 0);
         lpBuf = new TCHAR[nLen + 1];
         SendMessage(GetDlgItem(hwndDlg, IDC_EDIT_WINDOWTITLE), WM_GETTEXT,
            (nLen + 1), (LPARAM)lpBuf);
         SendMessage(hwndTarget, WM_SETTEXT, 0, (LPARAM)lpBuf);
         delete[] lpBuf;
         break;

      case IDCANCEL:
         DeleteDC(hdcDesk);
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;

   case WM_LBUTTONDOWN:
      // ��ʼ�϶�
      GetWindowRect(GetDlgItem(hwndDlg, IDC_STATIC_ICON), &rect);
      GetCursorPos(&pt);
      if (PtInRect(&rect, pt))
      {
         SetCapture(hwndDlg);
         SetCursor(hCursorDrag);
         SendMessage(GetDlgItem(hwndDlg, IDC_STATIC_ICON), STM_SETIMAGE,
            IMAGE_ICON, (LPARAM)hIconDrag);
         SetTimer(hwndDlg, 1, 200, NULL);
      }
      return TRUE;

   case WM_LBUTTONUP:
      // ֹͣ�϶�
      ReleaseCapture();
      SendMessage(GetDlgItem(hwndDlg, IDC_STATIC_ICON), STM_SETIMAGE,
         IMAGE_ICON, (LPARAM)hIconNormal);
      KillTimer(hwndDlg, 1);
      return TRUE;

   case WM_TIMER:
      GetCursorPos(&pt);
      hwndTarget = SmallestWindowFromPoint(pt);
      // ��ʾ���ھ��
      wsprintf(szBuf, TEXT("0x%08X"), (UINT_PTR)hwndTarget);
      SetDlgItemText(hwndDlg, IDC_EDIT_WINDOWHANDLE, szBuf);

      // ��ʾ��������
      GetClassName(hwndTarget, szBuf, _countof(szBuf));
      SetDlgItemText(hwndDlg, IDC_EDIT_CLASSNAME, szBuf);

      // ��ʾ���ڹ���
      wsprintf(szBuf, TEXT("0x%08X"), (ULONG_PTR)GetClassLongPtr(hwndTarget, GCLP_WNDPROC));
      SetDlgItemText(hwndDlg, IDC_EDIT_WNDPROC, szBuf);

      // ������Ӵ��ڿؼ�����ʾID
      if (dwCtrlID = GetDlgCtrlID(hwndTarget))
      {
         wsprintf(szBuf, TEXT("%d"), dwCtrlID);
         SetDlgItemText(hwndDlg, IDC_EDIT_CONTROLID, szBuf);
      }
      else
      {
         SetDlgItemText(hwndDlg, IDC_EDIT_CONTROLID, TEXT(""));
      }

      // ��ʾ���ڱ���
      nLen = SendMessage(hwndTarget, WM_GETTEXTLENGTH, 0, 0);
      if (nLen > 0)
      {
         lpBuf = new TCHAR[nLen + 1];
         SendMessage(hwndTarget, WM_GETTEXT, (nLen + 1), (LPARAM)lpBuf);
         SendMessage(GetDlgItem(hwndDlg, IDC_EDIT_WINDOWTITLE), WM_SETTEXT, 0, (LPARAM)lpBuf);
         delete[] lpBuf;
      }
      else
      {
         SendMessage(GetDlgItem(hwndDlg, IDC_EDIT_WINDOWTITLE), WM_SETTEXT, 0, (LPARAM)TEXT(""));
      }

      // ��ʾ����ID
      GetWindowThreadProcessId(hwndTarget, &dwProcessID);
      wsprintf(szBuf, TEXT("%d"), dwProcessID);
      SetDlgItemText(hwndDlg, IDC_EDIT_PROCESSID, szBuf);

      // ��ʾ������ID
      if ((dwParentProcessID = GetParentProcessIDByID(dwProcessID)) >= 0)
      {
         wsprintf(szBuf, TEXT("%d"), dwParentProcessID);
         SetDlgItemText(hwndDlg, IDC_EDIT_PARENTPROCESSID, szBuf);
      }
      else
      {
         SetDlgItemText(hwndDlg, IDC_EDIT_PARENTPROCESSID, TEXT(""));
      }

      // Ŀ�괰����Χ��������
      GetWindowRect(hwndTarget, &rect);
      if (rect.left < 0) rect.left = 0;
      if (rect.top < 0) rect.top = 0;
      Rectangle(hdcDesk, rect.left, rect.top, rect.right, rect.bottom);   // �������ɫ����
      Sleep(200);
      Rectangle(hdcDesk, rect.left, rect.top, rect.right, rect.bottom);   // �������ɫ����
      return TRUE;
   }

   return FALSE;
}

HWND SmallestWindowFromPoint(POINT pt)
{
   RECT rect, rcTemp;
   HWND hwnd, hwndParent, hwndTemp;

   hwnd = WindowFromPoint(pt);
   if (hwnd != NULL)
   {
      GetWindowRect(hwnd, &rect);
      hwndParent = GetParent(hwnd);

      // ���hwnd���ھ��и�����
      if (hwndParent != NULL)
      {
         // ���Һ�hwndͬһ�������һ��Z˳�򴰿�
         hwndTemp = hwnd;
         do
         {
            hwndTemp = GetWindow(hwndTemp, GW_HWNDNEXT);

            // ����ҵ��ĺ�hwndͬһ�������һ��Z˳�򴰿� ����ָ���������pt���ҿɼ�
            GetWindowRect(hwndTemp, &rcTemp);
            if (PtInRect(&rcTemp, pt) && IsWindowVisible(hwndTemp))
            {
               // �ҵ��Ĵ����ǲ��Ǳ�hwnd���ڸ�С
               if (((rcTemp.right - rcTemp.left) * (rcTemp.bottom - rcTemp.top)) <
                  ((rect.right - rect.left) * (rect.bottom - rect.top)))
               {
                  hwnd = hwndTemp;
                  GetWindowRect(hwnd, &rect);
               }
            }
         } while (hwndTemp != NULL);
      }
   }

   return hwnd;
}

DWORD GetParentProcessIDByID(DWORD dwProcessId)
{
   HANDLE hSnapshot;
   PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
   BOOL bRet;

   hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
   if (hSnapshot == INVALID_HANDLE_VALUE)
      return -1;

   bRet = Process32First(hSnapshot, &pe);
   while (bRet)
   {
      if (pe.th32ProcessID == dwProcessId)
         return pe.th32ParentProcessID;

      bRet = Process32Next(hSnapshot, &pe);
   }

   CloseHandle(hSnapshot);
   return -1;
}