#include <windows.h>
#include <tchar.h>
#include <Commctrl.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(lib, "Comctl32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

typedef struct _ITEMDATA
{
   TCHAR m_szServiceName[256];     // 服务名称
   TCHAR m_szDisplayName[256];     // 显示名称
   DWORD m_dwCurrentState;         // 服务状态
   DWORD m_dwControlsAccepted;     // 控制代码
   DWORD m_dwStartType;            // 启动类型
}ITEMDATA, * PITEMDATA;

// 全局变量
HINSTANCE g_hInstance;
HWND g_hwndDlg;                     // 对话框窗口句柄
HWND g_hwndList;                    // 列表视图控件句柄

BOOL g_bAscendingDisplayName;       // 是否已按显示名称升序排列
BOOL g_bAscendingCurrentState;      // 是否已按服务状态升序排列
BOOL g_bAscendingStartType;         // 是否已按启动类型升序排列

LPCTSTR arrlpStrCurrentState[] = { TEXT(""), TEXT("已停止"), TEXT("正在启动"), TEXT("正在停止"),
                    TEXT("正在运行"), TEXT("正在继续"), TEXT("正在暂停"), TEXT("已暂停") };
LPCTSTR arrlpStrStartType[] = { TEXT("自动(用于驱动程序服务)"), TEXT("手动(用于驱动程序服务)"),
                    TEXT("自动"), TEXT("手动"), TEXT("禁用") };

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 获取服务列表
BOOL GetServiceList();
// 列表视图控件排序回调函数
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   g_hInstance = hInstance;

   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_INITDIALOG:
      g_hwndDlg = hwndDlg;
      g_hwndList = GetDlgItem(hwndDlg, IDC_LIST_SERVICE);

      // 设置列表视图控件的扩展样式
      SendMessage(g_hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
         LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

      // 设置列标题：服务的显示名称、服务状态、启动类型、文件路径、服务描述
      LVCOLUMN lvc;
      ZeroMemory(&lvc, sizeof(LVCOLUMN));
      lvc.mask = LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
      lvc.iSubItem = 0; lvc.cx = 260; lvc.pszText = (LPTSTR)TEXT("服务的显示名称");
      SendMessage(g_hwndList, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
      lvc.iSubItem = 1; lvc.cx = 80; lvc.pszText = (LPTSTR)TEXT("服务状态");
      SendMessage(g_hwndList, LVM_INSERTCOLUMN, 1, (LPARAM)&lvc);
      lvc.iSubItem = 2; lvc.cx = 80; lvc.pszText = (LPTSTR)TEXT("启动类型");
      SendMessage(g_hwndList, LVM_INSERTCOLUMN, 2, (LPARAM)&lvc);
      lvc.iSubItem = 3; lvc.cx = 300; lvc.pszText = (LPTSTR)TEXT("文件路径");
      SendMessage(g_hwndList, LVM_INSERTCOLUMN, 3, (LPARAM)&lvc);
      lvc.iSubItem = 4; lvc.cx = 300; lvc.pszText = (LPTSTR)TEXT("服务描述");
      SendMessage(g_hwndList, LVM_INSERTCOLUMN, 4, (LPARAM)&lvc);

      // 获取服务列表
      GetServiceList();
      return TRUE;

   case WM_SIZE:
      // 根据父窗口客户区的大小调整列表视图控件的大小
      MoveWindow(g_hwndList, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
      return TRUE;

   case WM_NOTIFY:
      // 当用户点击列标题的时候
      if (((LPNMHDR)lParam)->code == LVN_COLUMNCLICK)
      {
         LPNMLISTVIEW pNMLV = (LPNMLISTVIEW)lParam;
         if (pNMLV->iSubItem < 3)
         {
            SendMessage(g_hwndList, LVM_SORTITEMS, pNMLV->iSubItem, (LPARAM)CompareFunc);

            switch (pNMLV->iSubItem)
            {
            case 0:
               g_bAscendingDisplayName = ~g_bAscendingDisplayName;
               break;
            case 1:
               g_bAscendingCurrentState = ~g_bAscendingCurrentState;
               break;
            case 2:
               g_bAscendingStartType = ~g_bAscendingStartType;
               break;
            }
         }
      }
      // 弹出快捷菜单
      else if (((LPNMHDR)lParam)->code == NM_RCLICK)
      {
         if (((LPNMITEMACTIVATE)lParam)->iItem < 0)
            return FALSE;

         POINT pt = { 0 };
         GetCursorPos(&pt);
         TrackPopupMenu(GetSubMenu(GetMenu(hwndDlg), 0), TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, hwndDlg, NULL);
      }
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDCANCEL:
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;
   }

   return FALSE;
}

BOOL GetServiceList()
{
   SC_HANDLE hSCManager = NULL;    // 服务控制管理器数据库的句柄
   LPBYTE lpServices = NULL;       // 缓冲区指针，返回ENUM_SERVICE_STATUS_PROCESS结构数组
   DWORD dwcbBufSize = 0;          // 上面缓冲区的大小
   DWORD dwcbBytesNeeded;          // lpServices设为NULL，dwcbBufSize设为0，返回所需的缓冲区大小
   DWORD dwServicesReturned;       // 返回服务个数
   DWORD dwResumeHandle = 0;       // 枚举的起点
   LVITEM lvi = { 0 };

   // 打开本地计算机的服务控制管理器数据库，返回服务控制管理器数据库的句柄
   hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
   if (!hSCManager)
      return FALSE;

   // 枚举服务控制管理器数据库中的服务，首先获取所需的缓冲区大小(字节单位)
   EnumServicesStatusEx(hSCManager, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, dwcbBufSize, &dwcbBytesNeeded, &dwServicesReturned, &dwResumeHandle, NULL);
   // 分配合适大小的缓冲区进行枚举服务
   lpServices = new BYTE[dwcbBytesNeeded];
   ZeroMemory(lpServices, dwcbBytesNeeded);
   dwResumeHandle = 0;
   EnumServicesStatusEx(hSCManager, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL, lpServices, dwcbBytesNeeded, &dwcbBytesNeeded, &dwServicesReturned, &dwResumeHandle, NULL);

   // 先释放所有项目数据，然后删除所有列表项
   ZeroMemory(&lvi, sizeof(LVITEM));
   int nCount = SendMessage(g_hwndList, LVM_GETITEMCOUNT, 0, 0);
   for (int i = 0; i < nCount; i++)
   {
      lvi.iItem = i; lvi.mask = LVIF_PARAM;
      SendMessage(g_hwndList, LVM_GETITEM, 0, (LPARAM)&lvi);
      delete (PITEMDATA)(lvi.lParam);
   }
   SendMessage(g_hwndList, LVM_DELETEALLITEMS, 0, 0);

   LPENUM_SERVICE_STATUS_PROCESS pEnumServiceStatus = (LPENUM_SERVICE_STATUS_PROCESS)lpServices;
   ZeroMemory(&lvi, sizeof(LVITEM));
   // 遍历获取到的ENUM_SERVICE_STATUS_PROCESS结构数组
   for (DWORD i = 0; i < dwServicesReturned; i++)
   {
      // 设置与列表项相关联的项目数据
      PITEMDATA pItemData = new ITEMDATA;
      ZeroMemory(pItemData, sizeof(ITEMDATA));
      StringCchCopy(pItemData->m_szServiceName, _countof(pItemData->m_szServiceName), pEnumServiceStatus[i].lpServiceName);
      StringCchCopy(pItemData->m_szDisplayName, _countof(pItemData->m_szDisplayName), pEnumServiceStatus[i].lpDisplayName);
      pItemData->m_dwCurrentState = pEnumServiceStatus[i].ServiceStatusProcess.dwCurrentState;
      pItemData->m_dwControlsAccepted = pEnumServiceStatus[i].ServiceStatusProcess.dwControlsAccepted;

      lvi.iItem = SendMessage(g_hwndList, LVM_GETITEMCOUNT, 0, 0);
      lvi.mask = LVIF_TEXT | LVIF_PARAM;
      lvi.lParam = (LPARAM)pItemData;
      // 第1列，服务的显示名称
      lvi.iSubItem = 0; lvi.pszText = pEnumServiceStatus[i].lpDisplayName;
      SendMessage(g_hwndList, LVM_INSERTITEM, 0, (LPARAM)&lvi);

      lvi.mask = LVIF_TEXT;
      // 第2列，服务状态
      lvi.iSubItem = 1; lvi.pszText = (LPTSTR)arrlpStrCurrentState[pEnumServiceStatus[i].ServiceStatusProcess.dwCurrentState];
      SendMessage(g_hwndList, LVM_SETITEM, 0, (LPARAM)&lvi);

      // 第3列，启动类型
      SC_HANDLE hService;
      LPQUERY_SERVICE_CONFIG lpServiceConfig = NULL;// 返回服务配置参数的缓冲区指针
      DWORD dwcbBufSizeService = 0;                 // 上面缓冲区的大小
      DWORD dwcbBytesNeededService;                 // 上面两参数设为NULL和0,返回所需缓冲区大小
      // 打开服务返回一个服务句柄
      hService = OpenService(hSCManager, pEnumServiceStatus[i].lpServiceName, SERVICE_QUERY_CONFIG);
      // 查询该服务的配置参数，首先获取所需的缓冲区大小(字节单位)
      QueryServiceConfig(hService, NULL, 0, &dwcbBytesNeededService);
      // 分配合适大小的缓冲区查询该服务的配置参数
      lpServiceConfig = (LPQUERY_SERVICE_CONFIG)new BYTE[dwcbBytesNeededService];
      ZeroMemory(lpServiceConfig, dwcbBytesNeededService);
      QueryServiceConfig(hService, lpServiceConfig, dwcbBytesNeededService, &dwcbBytesNeededService);

      lvi.iSubItem = 2; lvi.pszText = (LPTSTR)arrlpStrStartType[lpServiceConfig->dwStartType];
      SendMessage(g_hwndList, LVM_SETITEM, 0, (LPARAM)&lvi);
      pItemData->m_dwStartType = lpServiceConfig->dwStartType;

      // 第4列，文件路径
      lvi.iSubItem = 3; lvi.pszText = lpServiceConfig->lpBinaryPathName;
      SendMessage(g_hwndList, LVM_SETITEM, 0, (LPARAM)&lvi);

      delete[]lpServiceConfig;

      // 第5列，服务描述
      LPBYTE lpBufferService2;        // 返回服务其他配置参数的缓冲区指针
      DWORD dwcbBufSizeService2 = 0;  // 上面缓冲区的大小
      DWORD dwcbBytesNeededService2;  // 上面两参数设为NULL和0,返回所需缓冲区大小
      // 查询该服务的其他配置参数，首先获取所需的缓冲区大小(字节单位)
      QueryServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, NULL, 0, &dwcbBytesNeededService2);
      // 分配合适大小的缓冲区查询该服务的其他配置参数
      lpBufferService2 = new BYTE[dwcbBytesNeededService2];
      ZeroMemory(lpBufferService2, dwcbBytesNeededService2);
      QueryServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, lpBufferService2, dwcbBytesNeededService2, &dwcbBytesNeededService2);

      lvi.iSubItem = 4; lvi.pszText = ((LPSERVICE_DESCRIPTION)lpBufferService2)->lpDescription;
      SendMessage(g_hwndList, LVM_SETITEM, 0, (LPARAM)&lvi);

      delete[]lpBufferService2;

      CloseServiceHandle(hService);
   }

   // 按显示名称升序排列
   SendMessage(g_hwndList, LVM_SORTITEMS, 0, (LPARAM)CompareFunc);
   g_bAscendingDisplayName = ~g_bAscendingDisplayName;

   delete[]lpServices;
   // 关闭服务控制管理器数据库句柄
   CloseServiceHandle(hSCManager);
   return TRUE;
}

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
   LPCTSTR lpStr1, lpStr2;

   switch (lParamSort)
   {
   case 0:
      lpStr1 = ((PITEMDATA)lParam1)->m_szDisplayName;
      lpStr2 = ((PITEMDATA)lParam2)->m_szDisplayName;
      if (!g_bAscendingDisplayName)
         return CompareStringEx(LOCALE_NAME_USER_DEFAULT, LINGUISTIC_IGNORECASE | SORT_DIGITSASNUMBERS, lpStr1, -1, lpStr2, -1, NULL, NULL, NULL) - 2;
      else
         return CompareStringEx(LOCALE_NAME_USER_DEFAULT, LINGUISTIC_IGNORECASE | SORT_DIGITSASNUMBERS, lpStr2, -1, lpStr1, -1, NULL, NULL, NULL) - 2;
      break;

   case 1:
      lpStr1 = arrlpStrCurrentState[((PITEMDATA)lParam1)->m_dwCurrentState];
      lpStr2 = arrlpStrCurrentState[((PITEMDATA)lParam2)->m_dwCurrentState];
      if (!g_bAscendingCurrentState)
         return CompareStringEx(LOCALE_NAME_USER_DEFAULT, LINGUISTIC_IGNORECASE | SORT_DIGITSASNUMBERS, lpStr1, -1, lpStr2, -1, NULL, NULL, NULL) - 2;
      else
         return CompareStringEx(LOCALE_NAME_USER_DEFAULT, LINGUISTIC_IGNORECASE | SORT_DIGITSASNUMBERS, lpStr2, -1, lpStr1, -1, NULL, NULL, NULL) - 2;
      break;

   case 2:
      lpStr1 = arrlpStrStartType[((PITEMDATA)lParam1)->m_dwStartType];
      lpStr2 = arrlpStrStartType[((PITEMDATA)lParam2)->m_dwStartType];
      if (!g_bAscendingStartType)
         return CompareStringEx(LOCALE_NAME_USER_DEFAULT, LINGUISTIC_IGNORECASE | SORT_DIGITSASNUMBERS, lpStr1, -1, lpStr2, -1, NULL, NULL, NULL) - 2;
      else
         return CompareStringEx(LOCALE_NAME_USER_DEFAULT, LINGUISTIC_IGNORECASE | SORT_DIGITSASNUMBERS, lpStr2, -1, lpStr1, -1, NULL, NULL, NULL) - 2;
      break;
   }

   return 0;
}