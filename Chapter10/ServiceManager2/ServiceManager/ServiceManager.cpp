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
INT_PTR CALLBACK DialogProcCreateService(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 获取服务列表
BOOL GetServiceList();
// 列表视图控件排序回调函数
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

// 获取选中服务的当前状态和配置参数，启用禁用相关菜单项
VOID QueryServiceStatusAndConfig();

// 启动服务
BOOL StartTheService();
// 控制服务状态，停止服务、暂停服务、继续服务
BOOL ControlCurrentState(DWORD dwControl, DWORD dwNewCurrentState);

// 控制启动类型，设为自动启动、设为手动启动、设为已禁用
BOOL ChangeTheServiceConfig(DWORD dwStartType);

// 添加服务
BOOL CreateAService(LPCTSTR lpBinaryPathName, LPCTSTR lpServiceName, LPCTSTR lpDisplayName, DWORD dwStartType, DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS, LPCTSTR lpDescription = NULL, DWORD dwDesiredAccess = SERVICE_ALL_ACCESS, DWORD dwErrorControl = SERVICE_ERROR_NORMAL);
// 删除服务
BOOL DeleteTheService();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   g_hInstance = hInstance;

   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   INT_PTR nResult;

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

   case WM_INITMENUPOPUP:
      // 在显示弹出菜单之前，获取选中服务的当前状态和配置参数，启用禁用相关菜单项
      QueryServiceStatusAndConfig();
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case ID_SERVICE_ADD:
         nResult = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_CREATESERVICE), hwndDlg, DialogProcCreateService, NULL);
         if (nResult == 2)
            MessageBox(hwndDlg, TEXT("创建服务成功"), TEXT("成功提示"), MB_OK);
         else if (nResult == 1)
            MessageBox(hwndDlg, TEXT("创建服务失败"), TEXT("错误提示"), MB_OK);
         break;

      case ID_SERVICE_DELETE:
         if (DeleteTheService())
            MessageBox(hwndDlg, TEXT("删除服务成功"), TEXT("成功提示"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("删除服务失败"), TEXT("错误提示"), MB_OK);
         break;

      case ID_SERVICE_START:
         if (StartTheService())
            MessageBox(hwndDlg, TEXT("启动服务成功"), TEXT("成功提示"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("启动服务失败"), TEXT("错误提示"), MB_OK);
         break;

      case ID_SERVICE_STOP:
         if (ControlCurrentState(SERVICE_CONTROL_STOP, SERVICE_STOPPED))
            MessageBox(hwndDlg, TEXT("停止服务成功"), TEXT("成功提示"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("停止服务失败"), TEXT("错误提示"), MB_OK);
         break;
      case ID_SERVICE_PAUSE:
         if (ControlCurrentState(SERVICE_CONTROL_PAUSE, SERVICE_PAUSED))
            MessageBox(hwndDlg, TEXT("暂停服务成功"), TEXT("成功提示"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("暂停服务失败"), TEXT("错误提示"), MB_OK);
         break;
      case ID_SERVICE_CONTINUE:
         if (ControlCurrentState(SERVICE_CONTROL_CONTINUE, SERVICE_RUNNING))
            MessageBox(hwndDlg, TEXT("继续服务成功"), TEXT("成功提示"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("继续服务失败"), TEXT("错误提示"), MB_OK);
         break;

      case ID_SERVICE_AUTO:
         if (ChangeTheServiceConfig(SERVICE_AUTO_START))
            MessageBox(hwndDlg, TEXT("设为自动启动成功"), TEXT("成功提示"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("设为自动启动失败"), TEXT("错误提示"), MB_OK);
         break;
      case ID_SERVICE_DEMAND:
         if (ChangeTheServiceConfig(SERVICE_DEMAND_START))
            MessageBox(hwndDlg, TEXT("设为手动启动成功"), TEXT("成功提示"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("设为手动启动失败"), TEXT("错误提示"), MB_OK);
         break;
      case ID_SERVICE_DISABLE:
         if (ChangeTheServiceConfig(SERVICE_DISABLED))
            MessageBox(hwndDlg, TEXT("设为已禁用成功"), TEXT("成功提示"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("设为已禁用失败"), TEXT("错误提示"), MB_OK);
         break;

      case ID_SERVICE_REFRESH:
         // 刷新列表的时候按显示名称升序排列
         g_bAscendingDisplayName = FALSE;
         GetServiceList();
         break;

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

VOID QueryServiceStatusAndConfig()
{
   int nSelected;
   LVITEM lvi = { 0 };
   PITEMDATA pItemData;
   HMENU hMenu = GetMenu(g_hwndDlg);

   nSelected = SendMessage(g_hwndList, LVM_GETSELECTIONMARK, 0, 0);
   // 因为是调用GetMenu获取的菜单栏句柄，而不是调用LoadMenu加载的菜单资源，
   // 所以每次都需要设置每个菜单项的状态，只有添加服务和刷新列表这两个菜单项一直处于启用状态
   if (nSelected < 0)
   {
      EnableMenuItem(hMenu, ID_SERVICE_DELETE, MF_BYCOMMAND | MF_DISABLED);
      EnableMenuItem(hMenu, ID_SERVICE_START, MF_BYCOMMAND | MF_DISABLED);
      EnableMenuItem(hMenu, ID_SERVICE_STOP, MF_BYCOMMAND | MF_DISABLED);
      EnableMenuItem(hMenu, ID_SERVICE_PAUSE, MF_BYCOMMAND | MF_DISABLED);
      EnableMenuItem(hMenu, ID_SERVICE_CONTINUE, MF_BYCOMMAND | MF_DISABLED);
      EnableMenuItem(hMenu, ID_SERVICE_AUTO, MF_BYCOMMAND | MF_DISABLED);
      EnableMenuItem(hMenu, ID_SERVICE_DEMAND, MF_BYCOMMAND | MF_DISABLED);
      EnableMenuItem(hMenu, ID_SERVICE_DISABLE, MF_BYCOMMAND | MF_DISABLED);
      return;
   }
   else
   {
      EnableMenuItem(hMenu, ID_SERVICE_DELETE, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hMenu, ID_SERVICE_START, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hMenu, ID_SERVICE_STOP, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hMenu, ID_SERVICE_PAUSE, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hMenu, ID_SERVICE_CONTINUE, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hMenu, ID_SERVICE_AUTO, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hMenu, ID_SERVICE_DEMAND, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hMenu, ID_SERVICE_DISABLE, MF_BYCOMMAND | MF_ENABLED);
   }

   // 获取选中列表项的项目数据
   lvi.mask = LVIF_PARAM; lvi.iItem = nSelected;
   SendMessage(g_hwndList, LVM_GETITEM, 0, (LPARAM)&lvi);
   pItemData = (PITEMDATA)(lvi.lParam);

   // 禁用启动服务、停止服务、暂停服务、继续服务这些菜单项中不应该启用的
   if (pItemData->m_dwCurrentState == SERVICE_START_PENDING || pItemData->m_dwCurrentState == SERVICE_RUNNING)
   {
      EnableMenuItem(hMenu, ID_SERVICE_START, MF_BYCOMMAND | MF_DISABLED);
      if (!(pItemData->m_dwControlsAccepted & SERVICE_ACCEPT_STOP))
         EnableMenuItem(hMenu, ID_SERVICE_STOP, MF_BYCOMMAND | MF_DISABLED);
      if (!(pItemData->m_dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE))
         EnableMenuItem(hMenu, ID_SERVICE_PAUSE, MF_BYCOMMAND | MF_DISABLED);
      EnableMenuItem(hMenu, ID_SERVICE_CONTINUE, MF_BYCOMMAND | MF_DISABLED);
   }
   else if (pItemData->m_dwCurrentState == SERVICE_STOP_PENDING || pItemData->m_dwCurrentState == SERVICE_STOPPED)
   {
      EnableMenuItem(hMenu, ID_SERVICE_STOP, MF_BYCOMMAND | MF_DISABLED);
      EnableMenuItem(hMenu, ID_SERVICE_PAUSE, MF_BYCOMMAND | MF_DISABLED);
      EnableMenuItem(hMenu, ID_SERVICE_CONTINUE, MF_BYCOMMAND | MF_DISABLED);
   }
   else if (pItemData->m_dwCurrentState == SERVICE_PAUSE_PENDING || pItemData->m_dwCurrentState == SERVICE_PAUSED)
   {
      EnableMenuItem(hMenu, ID_SERVICE_START, MF_BYCOMMAND | MF_DISABLED);
      if (!(pItemData->m_dwControlsAccepted & SERVICE_ACCEPT_STOP))
         EnableMenuItem(hMenu, ID_SERVICE_STOP, MF_BYCOMMAND | MF_DISABLED);
      EnableMenuItem(hMenu, ID_SERVICE_PAUSE, MF_BYCOMMAND | MF_DISABLED);
      if (!(pItemData->m_dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE))
         EnableMenuItem(hMenu, ID_SERVICE_CONTINUE, MF_BYCOMMAND | MF_DISABLED);
   }
   else if (pItemData->m_dwCurrentState == SERVICE_CONTINUE_PENDING)
   {
      EnableMenuItem(hMenu, ID_SERVICE_START, MF_BYCOMMAND | MF_DISABLED);
      if (!(pItemData->m_dwControlsAccepted & SERVICE_ACCEPT_STOP))
         EnableMenuItem(hMenu, ID_SERVICE_STOP, MF_BYCOMMAND | MF_DISABLED);
      if (!(pItemData->m_dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE))
         EnableMenuItem(hMenu, ID_SERVICE_PAUSE, MF_BYCOMMAND | MF_DISABLED);
      EnableMenuItem(hMenu, ID_SERVICE_CONTINUE, MF_BYCOMMAND | MF_DISABLED);
   }

   // 禁用设为自动启动、设为手动启动、设为已禁用这些菜单项中不应该启用的
   if (pItemData->m_dwStartType == SERVICE_AUTO_START)
   {
      EnableMenuItem(hMenu, ID_SERVICE_AUTO, MF_BYCOMMAND | MF_DISABLED);
   }
   else if (pItemData->m_dwStartType == SERVICE_DEMAND_START)
   {
      EnableMenuItem(hMenu, ID_SERVICE_DEMAND, MF_BYCOMMAND | MF_DISABLED);
   }
   else if (pItemData->m_dwStartType == SERVICE_DISABLED)
   {
      EnableMenuItem(hMenu, ID_SERVICE_DISABLE, MF_BYCOMMAND | MF_DISABLED);
   }

   return;
}

BOOL StartTheService()
{
   int nSelected;
   LVITEM lvi = { 0 };
   PITEMDATA pItemData;
   SC_HANDLE hSCManager = NULL;    // 服务控制管理器数据库的句柄
   SC_HANDLE hService;             // 服务句柄

   // 获取选中列表项的项目数据
   nSelected = SendMessage(g_hwndList, LVM_GETSELECTIONMARK, 0, 0);
   lvi.mask = LVIF_PARAM; lvi.iItem = nSelected;
   SendMessage(g_hwndList, LVM_GETITEM, 0, (LPARAM)&lvi);
   pItemData = (PITEMDATA)(lvi.lParam);

   hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
   if (!hSCManager)
      return FALSE;

   hService = OpenService(hSCManager, pItemData->m_szServiceName, SERVICE_START);
   if (!hService)
   {
      CloseServiceHandle(hSCManager);
      return FALSE;
   }

   if (!StartService(hService, 0, NULL))
   {
      CloseServiceHandle(hService);
      CloseServiceHandle(hSCManager);
      return FALSE;
   }

   pItemData->m_dwCurrentState = SERVICE_RUNNING;
   lvi.mask = LVIF_TEXT;
   lvi.iItem = nSelected; lvi.iSubItem = 1;
   lvi.pszText = (LPTSTR)arrlpStrCurrentState[SERVICE_RUNNING];
   SendMessage(g_hwndList, LVM_SETITEM, 0, (LPARAM)&lvi);

   CloseServiceHandle(hService);
   CloseServiceHandle(hSCManager);
   return TRUE;
}

BOOL ControlCurrentState(DWORD dwControl, DWORD dwNewCurrentState)
{
   int nSelected;
   LVITEM lvi = { 0 };
   PITEMDATA pItemData;
   SC_HANDLE hSCManager = NULL;    // 服务控制管理器数据库的句柄
   SC_HANDLE hService;             // 服务句柄
   SERVICE_STATUS serviceStatus = { 0 };

   // 获取选中列表项的项目数据
   nSelected = SendMessage(g_hwndList, LVM_GETSELECTIONMARK, 0, 0);
   lvi.mask = LVIF_PARAM; lvi.iItem = nSelected;
   SendMessage(g_hwndList, LVM_GETITEM, 0, (LPARAM)&lvi);
   pItemData = (PITEMDATA)(lvi.lParam);

   hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
   if (!hSCManager)
      return FALSE;

   hService = OpenService(hSCManager, pItemData->m_szServiceName, SERVICE_STOP | SERVICE_PAUSE_CONTINUE);
   if (!hService)
   {
      CloseServiceHandle(hSCManager);
      return FALSE;
   }

   if (!ControlService(hService, dwControl, &serviceStatus))
   {
      CloseServiceHandle(hService);
      CloseServiceHandle(hSCManager);
      return FALSE;
   }

   // 实际编程中应根据返回的最新服务状态信息的SERVICE_STATUS结构进行合理化处理，
   // 而不应该简单地使用传递过来的dwNewCurrentState参数
   pItemData->m_dwCurrentState = dwNewCurrentState;
   lvi.mask = LVIF_TEXT;
   lvi.iItem = nSelected; lvi.iSubItem = 1;
   lvi.pszText = (LPTSTR)arrlpStrCurrentState[dwNewCurrentState];
   SendMessage(g_hwndList, LVM_SETITEM, 0, (LPARAM)&lvi);

   CloseServiceHandle(hService);
   CloseServiceHandle(hSCManager);
   return TRUE;
}

BOOL ChangeTheServiceConfig(DWORD dwStartType)
{
   int nSelected;
   LVITEM lvi = { 0 };
   PITEMDATA pItemData;
   SC_HANDLE hSCManager = NULL;    // 服务控制管理器数据库的句柄
   SC_HANDLE hService;             // 服务句柄

   // 获取选中列表项的项目数据
   nSelected = SendMessage(g_hwndList, LVM_GETSELECTIONMARK, 0, 0);
   lvi.mask = LVIF_PARAM; lvi.iItem = nSelected;
   SendMessage(g_hwndList, LVM_GETITEM, 0, (LPARAM)&lvi);
   pItemData = (PITEMDATA)(lvi.lParam);

   hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
   if (!hSCManager)
      return FALSE;

   hService = OpenService(hSCManager, pItemData->m_szServiceName, SERVICE_CHANGE_CONFIG);
   if (!hService)
   {
      CloseServiceHandle(hSCManager);
      return FALSE;
   }

   if (!ChangeServiceConfig(hService, SERVICE_NO_CHANGE, dwStartType, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
   {
      CloseServiceHandle(hService);
      CloseServiceHandle(hSCManager);
      return FALSE;
   }

   pItemData->m_dwStartType = dwStartType;
   lvi.mask = LVIF_TEXT;
   lvi.iItem = nSelected; lvi.iSubItem = 2;
   lvi.pszText = (LPTSTR)arrlpStrStartType[dwStartType];
   SendMessage(g_hwndList, LVM_SETITEM, 0, (LPARAM)&lvi);

   CloseServiceHandle(hService);
   CloseServiceHandle(hSCManager);
   return TRUE;
}

//////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK DialogProcCreateService(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   TCHAR szFile[MAX_PATH] = { 0 };             // 返回用户选择的文件名的缓冲区
   TCHAR szFileTitle[MAX_PATH] = { 0 };        // 返回用户所选文件的文件名和扩展名的缓冲区
   OPENFILENAME ofn = { sizeof(OPENFILENAME) };
   static HWND hwndCombo;
   int nIndex, nCount;
   BOOL bRet = FALSE;

   TCHAR szBinaryPathName[MAX_PATH] = { 0 };   // 文件路径
   TCHAR szServiceName[256] = { 0 };           // 服务名称
   TCHAR szDisplayName[256] = { 0 };           // 显示名称
   TCHAR szDescription[1024] = { 0 };          // 服务描述
   DWORD dwStartType;                          // 启动类型

   switch (uMsg)
   {
   case WM_INITDIALOG:
      // 添加列表项，设置项目数据为服务的启动类型数值，默认选中自动启动
      hwndCombo = GetDlgItem(hwndDlg, IDC_COMBO_STARTTYPE);
      nIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("自动启动"));
      SendMessage(hwndCombo, CB_SETITEMDATA, nIndex, 0x00000002);
      nIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("手动启动"));
      SendMessage(hwndCombo, CB_SETITEMDATA, nIndex, 0x00000003);
      nIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("禁用"));
      SendMessage(hwndCombo, CB_SETITEMDATA, nIndex, 0x00000004);
      SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);

      // 设置服务名称、显示名称和服务描述编辑控件的最大字符个数
      SendMessage(GetDlgItem(hwndDlg, IDC_EDIT_SERVICENAME), EM_SETLIMITTEXT, _countof(szServiceName) - 1, 0);
      SendMessage(GetDlgItem(hwndDlg, IDC_EDIT_DISPLAYNAME), EM_SETLIMITTEXT, _countof(szDisplayName) - 1, 0);
      SendMessage(GetDlgItem(hwndDlg, IDC_EDIT_DESCRIPTION), EM_SETLIMITTEXT, _countof(szDescription) - 1, 0);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_BROWSE:
         ofn.hwndOwner = hwndDlg;
         ofn.lpstrFilter = TEXT("EXE文件(*.exe)\0*.exe\0所有文件(*.*)\0*.*\0");
         ofn.lpstrFile = szFile;
         ofn.nMaxFile = _countof(szFile);
         ofn.lpstrFileTitle = szFileTitle;
         ofn.nMaxFileTitle = _countof(szFileTitle);
         ofn.lpstrTitle = TEXT("请选择要打开的文件");
         ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
         if (GetOpenFileName(&ofn))
         {
            *(_tcsrchr(szFileTitle, TEXT('.'))) = TEXT('\0');
            SetDlgItemText(hwndDlg, IDC_EDIT_BINARYPATHNAME, szFile);
            SetDlgItemText(hwndDlg, IDC_EDIT_SERVICENAME, szFileTitle);
            SetDlgItemText(hwndDlg, IDC_EDIT_DISPLAYNAME, szFileTitle);
         }
         break;

      case IDOK:
         nCount = GetDlgItemText(hwndDlg, IDC_EDIT_BINARYPATHNAME, szBinaryPathName, _countof(szBinaryPathName));
         if (nCount < 5)
         {
            MessageBox(hwndDlg, TEXT("请输入服务的完整路径"), TEXT("错误提示"), MB_OK);
            return TRUE;
         }
         nCount = GetDlgItemText(hwndDlg, IDC_EDIT_SERVICENAME, szServiceName, _countof(szServiceName));
         if (nCount < 1)
         {
            MessageBox(hwndDlg, TEXT("请输入服务名称"), TEXT("错误提示"), MB_OK);
            return TRUE;
         }
         nCount = GetDlgItemText(hwndDlg, IDC_EDIT_DISPLAYNAME, szDisplayName, _countof(szDisplayName));
         if (nCount < 1)
         {
            MessageBox(hwndDlg, TEXT("请输入显示名称"), TEXT("错误提示"), MB_OK);
            return TRUE;
         }
         GetDlgItemText(hwndDlg, IDC_EDIT_DESCRIPTION, szDescription, _countof(szDescription));

         nIndex = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
         dwStartType = SendMessage(hwndCombo, CB_GETITEMDATA, nIndex, 0);

         // 调用自定义函数CreateAService创建服务
         bRet = CreateAService(szBinaryPathName, szServiceName, szDisplayName, dwStartType, SERVICE_WIN32_OWN_PROCESS, szDescription);
         if (bRet)
            EndDialog(hwndDlg, 2);
         else
            EndDialog(hwndDlg, 1);
         break;

      case IDCANCEL:
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;
   }

   return FALSE;
}

BOOL CreateAService(LPCTSTR lpBinaryPathName, LPCTSTR lpServiceName, LPCTSTR lpDisplayName, DWORD dwStartType, DWORD dwServiceType, LPCTSTR lpDescription, DWORD dwDesiredAccess, DWORD dwErrorControl)
{
   SC_HANDLE hSCManager = NULL;                    // 服务控制管理器数据库的句柄
   SC_HANDLE hService;                             // 服务句柄
   SERVICE_DESCRIPTION serviceDescription = { 0 }; // 服务描述

   hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
   if (!hSCManager)
      return FALSE;

   // 创建服务
   hService = CreateService(hSCManager, lpServiceName, lpDisplayName, dwDesiredAccess, dwServiceType, dwStartType, dwErrorControl, lpBinaryPathName, NULL, NULL, NULL, NULL, NULL);
   if (!hService)
   {
      CloseServiceHandle(hSCManager);
      return FALSE;
   }

   // 设置服务描述
   if (!lpDescription)
   {
      serviceDescription.lpDescription = (LPTSTR)lpDescription;
      ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &serviceDescription);
   }

   // 刷新列表，按显示名称升序排列
   g_bAscendingDisplayName = FALSE;
   GetServiceList();

   // 找到新添加列表项的索引
   LVFINDINFO lvfi = { 0 };
   lvfi.flags = LVFI_STRING;
   lvfi.psz = lpDisplayName;
   int nIndex = SendMessage(g_hwndList, LVM_FINDITEM, -1, (LPARAM)&lvfi);
   // 设置为选中状态
   LVITEM lvi = { 0 }; lvi.state = lvi.stateMask = LVIS_SELECTED;
   SendMessage(g_hwndList, LVM_SETITEMSTATE, nIndex, (LPARAM)&lvi);

   // 把新添加的列表项滚动到可见视图中
   int nCount = SendMessage(g_hwndList, LVM_GETCOUNTPERPAGE, 0, 0);
   if (nIndex > nCount)
   {
      DWORD dw = SendMessage(g_hwndList, LVM_APPROXIMATEVIEWRECT, nIndex - 2, MAKELPARAM(-1, -1));
      SendMessage(g_hwndList, LVM_SCROLL, 0, HIWORD(dw));
   }

   CloseServiceHandle(hService);
   CloseServiceHandle(hSCManager);
   return TRUE;
}

BOOL DeleteTheService()
{
   int nSelected;
   LVITEM lvi = { 0 };
   PITEMDATA pItemData;
   SC_HANDLE hSCManager = NULL;    // 服务控制管理器数据库的句柄
   SC_HANDLE hService;             // 服务句柄

   // 获取选中列表项的项目数据
   nSelected = SendMessage(g_hwndList, LVM_GETSELECTIONMARK, 0, 0);
   lvi.mask = LVIF_PARAM; lvi.iItem = nSelected;
   SendMessage(g_hwndList, LVM_GETITEM, 0, (LPARAM)&lvi);
   pItemData = (PITEMDATA)(lvi.lParam);

   hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
   if (!hSCManager)
      return FALSE;

   hService = OpenService(hSCManager, pItemData->m_szServiceName, DELETE);
   if (!hService)
   {
      CloseServiceHandle(hSCManager);
      return FALSE;
   }

   if (!DeleteService(hService))
   {
      return FALSE;
   }
   else
   {
      delete[]pItemData;
      SendMessage(g_hwndList, LVM_DELETEITEM, nSelected, 0);
   }

   CloseServiceHandle(hService);
   CloseServiceHandle(hSCManager);
   return TRUE;
}