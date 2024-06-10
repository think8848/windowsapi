#include <windows.h>
#include <TlHelp32.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 全局变量
HINSTANCE g_hInstance;

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 获取目标窗口句柄
HWND SmallestWindowFromPoint(POINT pt);
// 获取父进程ID
DWORD GetParentProcessIDByID(DWORD dwProcessId);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   g_hInstance = hInstance;

   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   static HCURSOR hCursorDrag;     // 拖动时的光标句柄
   static HICON hIconNormal;       // 正常情况下图像静态控件所用的图标句柄
   static HICON hIconDrag;         // 拖动时的图像静态控件所用的图标句柄
   static HWND hwndTarget;         // 目标窗口句柄
   static HDC hdcDesk;             // 桌面设备环境句柄，用于在目标窗口周围绘制闪动矩形
   RECT rect;
   POINT pt;
   DWORD dwProcessID, dwParentProcessID, dwCtrlID;
   TCHAR szBuf[128] = { 0 };
   LPTSTR lpBuf = NULL;
   int nLen;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      // 为对话框程序左上角设置一个图标
      SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(g_hInstance,
         MAKEINTRESOURCE(IDI_ICON_MAIN)));

      // 拖动时的光标句柄，正常情况下和拖动时的图像静态控件所用的图标句柄
      hCursorDrag = LoadCursor(g_hInstance, MAKEINTRESOURCE(IDC_CURSOR_DRAG));
      hIconNormal = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_NORMAL));
      hIconDrag = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON_DRAG));

      // 桌面设备环境，用于在目标窗口周围绘制闪动矩形
      hdcDesk = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
      SelectObject(hdcDesk, CreatePen(PS_SOLID, 2, RGB(255, 0, 255)));
      SetROP2(hdcDesk, R2_NOTXORPEN);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_CHK_TOPMOST:
         // 窗口置顶
         if (IsDlgButtonChecked(hwndDlg, IDC_CHK_TOPMOST) == BST_CHECKED)
            SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
         else
            SetWindowPos(hwndDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
         break;

      case IDC_BTN_MODIFYTITLE:
         // 修改标题
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
      // 开始拖动
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
      // 停止拖动
      ReleaseCapture();
      SendMessage(GetDlgItem(hwndDlg, IDC_STATIC_ICON), STM_SETIMAGE,
         IMAGE_ICON, (LPARAM)hIconNormal);
      KillTimer(hwndDlg, 1);
      return TRUE;

   case WM_TIMER:
      GetCursorPos(&pt);
      hwndTarget = SmallestWindowFromPoint(pt);
      // 显示窗口句柄
      wsprintf(szBuf, TEXT("0x%08X"), (UINT_PTR)hwndTarget);
      SetDlgItemText(hwndDlg, IDC_EDIT_WINDOWHANDLE, szBuf);

      // 显示窗口类名
      GetClassName(hwndTarget, szBuf, _countof(szBuf));
      SetDlgItemText(hwndDlg, IDC_EDIT_CLASSNAME, szBuf);

      // 显示窗口过程
      wsprintf(szBuf, TEXT("0x%08X"), (ULONG_PTR)GetClassLongPtr(hwndTarget, GCLP_WNDPROC));
      SetDlgItemText(hwndDlg, IDC_EDIT_WNDPROC, szBuf);

      // 如果是子窗口控件，显示ID
      if (dwCtrlID = GetDlgCtrlID(hwndTarget))
      {
         wsprintf(szBuf, TEXT("%d"), dwCtrlID);
         SetDlgItemText(hwndDlg, IDC_EDIT_CONTROLID, szBuf);
      }
      else
      {
         SetDlgItemText(hwndDlg, IDC_EDIT_CONTROLID, TEXT(""));
      }

      // 显示窗口标题
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

      // 显示进程ID
      GetWindowThreadProcessId(hwndTarget, &dwProcessID);
      wsprintf(szBuf, TEXT("%d"), dwProcessID);
      SetDlgItemText(hwndDlg, IDC_EDIT_PROCESSID, szBuf);

      // 显示父进程ID
      if ((dwParentProcessID = GetParentProcessIDByID(dwProcessID)) >= 0)
      {
         wsprintf(szBuf, TEXT("%d"), dwParentProcessID);
         SetDlgItemText(hwndDlg, IDC_EDIT_PARENTPROCESSID, szBuf);
      }
      else
      {
         SetDlgItemText(hwndDlg, IDC_EDIT_PARENTPROCESSID, TEXT(""));
      }

      // 目标窗口周围矩形闪动
      GetWindowRect(hwndTarget, &rect);
      if (rect.left < 0) rect.left = 0;
      if (rect.top < 0) rect.top = 0;
      Rectangle(hdcDesk, rect.left, rect.top, rect.right, rect.bottom);   // 绘制洋红色矩形
      Sleep(200);
      Rectangle(hdcDesk, rect.left, rect.top, rect.right, rect.bottom);   // 擦除洋红色矩形
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

      // 如果hwnd窗口具有父窗口
      if (hwndParent != NULL)
      {
         // 查找和hwnd同一级别的下一个Z顺序窗口
         hwndTemp = hwnd;
         do
         {
            hwndTemp = GetWindow(hwndTemp, GW_HWNDNEXT);

            // 如果找到的和hwnd同一级别的下一个Z顺序窗口 包含指定的坐标点pt并且可见
            GetWindowRect(hwndTemp, &rcTemp);
            if (PtInRect(&rcTemp, pt) && IsWindowVisible(hwndTemp))
            {
               // 找到的窗口是不是比hwnd窗口更小
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