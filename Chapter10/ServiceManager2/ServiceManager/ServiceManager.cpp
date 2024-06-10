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
   TCHAR m_szServiceName[256];     // ��������
   TCHAR m_szDisplayName[256];     // ��ʾ����
   DWORD m_dwCurrentState;         // ����״̬
   DWORD m_dwControlsAccepted;     // ���ƴ���
   DWORD m_dwStartType;            // ��������
}ITEMDATA, * PITEMDATA;

// ȫ�ֱ���
HINSTANCE g_hInstance;
HWND g_hwndDlg;                     // �Ի��򴰿ھ��
HWND g_hwndList;                    // �б���ͼ�ؼ����

BOOL g_bAscendingDisplayName;       // �Ƿ��Ѱ���ʾ������������
BOOL g_bAscendingCurrentState;      // �Ƿ��Ѱ�����״̬��������
BOOL g_bAscendingStartType;         // �Ƿ��Ѱ�����������������

LPCTSTR arrlpStrCurrentState[] = { TEXT(""), TEXT("��ֹͣ"), TEXT("��������"), TEXT("����ֹͣ"),
                    TEXT("��������"), TEXT("���ڼ���"), TEXT("������ͣ"), TEXT("����ͣ") };
LPCTSTR arrlpStrStartType[] = { TEXT("�Զ�(���������������)"), TEXT("�ֶ�(���������������)"),
                    TEXT("�Զ�"), TEXT("�ֶ�"), TEXT("����") };

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DialogProcCreateService(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// ��ȡ�����б�
BOOL GetServiceList();
// �б���ͼ�ؼ�����ص�����
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

// ��ȡѡ�з���ĵ�ǰ״̬�����ò��������ý�����ز˵���
VOID QueryServiceStatusAndConfig();

// ��������
BOOL StartTheService();
// ���Ʒ���״̬��ֹͣ������ͣ���񡢼�������
BOOL ControlCurrentState(DWORD dwControl, DWORD dwNewCurrentState);

// �����������ͣ���Ϊ�Զ���������Ϊ�ֶ���������Ϊ�ѽ���
BOOL ChangeTheServiceConfig(DWORD dwStartType);

// ��ӷ���
BOOL CreateAService(LPCTSTR lpBinaryPathName, LPCTSTR lpServiceName, LPCTSTR lpDisplayName, DWORD dwStartType, DWORD dwServiceType = SERVICE_WIN32_OWN_PROCESS, LPCTSTR lpDescription = NULL, DWORD dwDesiredAccess = SERVICE_ALL_ACCESS, DWORD dwErrorControl = SERVICE_ERROR_NORMAL);
// ɾ������
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

      // �����б���ͼ�ؼ�����չ��ʽ
      SendMessage(g_hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
         LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

      // �����б��⣺�������ʾ���ơ�����״̬���������͡��ļ�·������������
      LVCOLUMN lvc;
      ZeroMemory(&lvc, sizeof(LVCOLUMN));
      lvc.mask = LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
      lvc.iSubItem = 0; lvc.cx = 260; lvc.pszText = (LPTSTR)TEXT("�������ʾ����");
      SendMessage(g_hwndList, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
      lvc.iSubItem = 1; lvc.cx = 80; lvc.pszText = (LPTSTR)TEXT("����״̬");
      SendMessage(g_hwndList, LVM_INSERTCOLUMN, 1, (LPARAM)&lvc);
      lvc.iSubItem = 2; lvc.cx = 80; lvc.pszText = (LPTSTR)TEXT("��������");
      SendMessage(g_hwndList, LVM_INSERTCOLUMN, 2, (LPARAM)&lvc);
      lvc.iSubItem = 3; lvc.cx = 300; lvc.pszText = (LPTSTR)TEXT("�ļ�·��");
      SendMessage(g_hwndList, LVM_INSERTCOLUMN, 3, (LPARAM)&lvc);
      lvc.iSubItem = 4; lvc.cx = 300; lvc.pszText = (LPTSTR)TEXT("��������");
      SendMessage(g_hwndList, LVM_INSERTCOLUMN, 4, (LPARAM)&lvc);

      // ��ȡ�����б�
      GetServiceList();
      return TRUE;

   case WM_SIZE:
      // ���ݸ����ڿͻ����Ĵ�С�����б���ͼ�ؼ��Ĵ�С
      MoveWindow(g_hwndList, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
      return TRUE;

   case WM_NOTIFY:
      // ���û�����б����ʱ��
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
      // ������ݲ˵�
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
      // ����ʾ�����˵�֮ǰ����ȡѡ�з���ĵ�ǰ״̬�����ò��������ý�����ز˵���
      QueryServiceStatusAndConfig();
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case ID_SERVICE_ADD:
         nResult = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_CREATESERVICE), hwndDlg, DialogProcCreateService, NULL);
         if (nResult == 2)
            MessageBox(hwndDlg, TEXT("��������ɹ�"), TEXT("�ɹ���ʾ"), MB_OK);
         else if (nResult == 1)
            MessageBox(hwndDlg, TEXT("��������ʧ��"), TEXT("������ʾ"), MB_OK);
         break;

      case ID_SERVICE_DELETE:
         if (DeleteTheService())
            MessageBox(hwndDlg, TEXT("ɾ������ɹ�"), TEXT("�ɹ���ʾ"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("ɾ������ʧ��"), TEXT("������ʾ"), MB_OK);
         break;

      case ID_SERVICE_START:
         if (StartTheService())
            MessageBox(hwndDlg, TEXT("��������ɹ�"), TEXT("�ɹ���ʾ"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("��������ʧ��"), TEXT("������ʾ"), MB_OK);
         break;

      case ID_SERVICE_STOP:
         if (ControlCurrentState(SERVICE_CONTROL_STOP, SERVICE_STOPPED))
            MessageBox(hwndDlg, TEXT("ֹͣ����ɹ�"), TEXT("�ɹ���ʾ"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("ֹͣ����ʧ��"), TEXT("������ʾ"), MB_OK);
         break;
      case ID_SERVICE_PAUSE:
         if (ControlCurrentState(SERVICE_CONTROL_PAUSE, SERVICE_PAUSED))
            MessageBox(hwndDlg, TEXT("��ͣ����ɹ�"), TEXT("�ɹ���ʾ"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("��ͣ����ʧ��"), TEXT("������ʾ"), MB_OK);
         break;
      case ID_SERVICE_CONTINUE:
         if (ControlCurrentState(SERVICE_CONTROL_CONTINUE, SERVICE_RUNNING))
            MessageBox(hwndDlg, TEXT("��������ɹ�"), TEXT("�ɹ���ʾ"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("��������ʧ��"), TEXT("������ʾ"), MB_OK);
         break;

      case ID_SERVICE_AUTO:
         if (ChangeTheServiceConfig(SERVICE_AUTO_START))
            MessageBox(hwndDlg, TEXT("��Ϊ�Զ������ɹ�"), TEXT("�ɹ���ʾ"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("��Ϊ�Զ�����ʧ��"), TEXT("������ʾ"), MB_OK);
         break;
      case ID_SERVICE_DEMAND:
         if (ChangeTheServiceConfig(SERVICE_DEMAND_START))
            MessageBox(hwndDlg, TEXT("��Ϊ�ֶ������ɹ�"), TEXT("�ɹ���ʾ"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("��Ϊ�ֶ�����ʧ��"), TEXT("������ʾ"), MB_OK);
         break;
      case ID_SERVICE_DISABLE:
         if (ChangeTheServiceConfig(SERVICE_DISABLED))
            MessageBox(hwndDlg, TEXT("��Ϊ�ѽ��óɹ�"), TEXT("�ɹ���ʾ"), MB_OK);
         else
            MessageBox(hwndDlg, TEXT("��Ϊ�ѽ���ʧ��"), TEXT("������ʾ"), MB_OK);
         break;

      case ID_SERVICE_REFRESH:
         // ˢ���б��ʱ����ʾ������������
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
   SC_HANDLE hSCManager = NULL;    // ������ƹ��������ݿ�ľ��
   LPBYTE lpServices = NULL;       // ������ָ�룬����ENUM_SERVICE_STATUS_PROCESS�ṹ����
   DWORD dwcbBufSize = 0;          // ���滺�����Ĵ�С
   DWORD dwcbBytesNeeded;          // lpServices��ΪNULL��dwcbBufSize��Ϊ0����������Ļ�������С
   DWORD dwServicesReturned;       // ���ط������
   DWORD dwResumeHandle = 0;       // ö�ٵ����
   LVITEM lvi = { 0 };

   // �򿪱��ؼ�����ķ�����ƹ��������ݿ⣬���ط�����ƹ��������ݿ�ľ��
   hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
   if (!hSCManager)
      return FALSE;

   // ö�ٷ�����ƹ��������ݿ��еķ������Ȼ�ȡ����Ļ�������С(�ֽڵ�λ)
   EnumServicesStatusEx(hSCManager, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, dwcbBufSize, &dwcbBytesNeeded, &dwServicesReturned, &dwResumeHandle, NULL);
   // ������ʴ�С�Ļ���������ö�ٷ���
   lpServices = new BYTE[dwcbBytesNeeded];
   ZeroMemory(lpServices, dwcbBytesNeeded);
   dwResumeHandle = 0;
   EnumServicesStatusEx(hSCManager, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL, lpServices, dwcbBytesNeeded, &dwcbBytesNeeded, &dwServicesReturned, &dwResumeHandle, NULL);

   // ���ͷ�������Ŀ���ݣ�Ȼ��ɾ�������б���
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
   // ������ȡ����ENUM_SERVICE_STATUS_PROCESS�ṹ����
   for (DWORD i = 0; i < dwServicesReturned; i++)
   {
      // �������б������������Ŀ����
      PITEMDATA pItemData = new ITEMDATA;
      ZeroMemory(pItemData, sizeof(ITEMDATA));
      StringCchCopy(pItemData->m_szServiceName, _countof(pItemData->m_szServiceName), pEnumServiceStatus[i].lpServiceName);
      StringCchCopy(pItemData->m_szDisplayName, _countof(pItemData->m_szDisplayName), pEnumServiceStatus[i].lpDisplayName);
      pItemData->m_dwCurrentState = pEnumServiceStatus[i].ServiceStatusProcess.dwCurrentState;
      pItemData->m_dwControlsAccepted = pEnumServiceStatus[i].ServiceStatusProcess.dwControlsAccepted;

      lvi.iItem = SendMessage(g_hwndList, LVM_GETITEMCOUNT, 0, 0);
      lvi.mask = LVIF_TEXT | LVIF_PARAM;
      lvi.lParam = (LPARAM)pItemData;
      // ��1�У��������ʾ����
      lvi.iSubItem = 0; lvi.pszText = pEnumServiceStatus[i].lpDisplayName;
      SendMessage(g_hwndList, LVM_INSERTITEM, 0, (LPARAM)&lvi);

      lvi.mask = LVIF_TEXT;
      // ��2�У�����״̬
      lvi.iSubItem = 1; lvi.pszText = (LPTSTR)arrlpStrCurrentState[pEnumServiceStatus[i].ServiceStatusProcess.dwCurrentState];
      SendMessage(g_hwndList, LVM_SETITEM, 0, (LPARAM)&lvi);

      // ��3�У���������
      SC_HANDLE hService;
      LPQUERY_SERVICE_CONFIG lpServiceConfig = NULL;// ���ط������ò����Ļ�����ָ��
      DWORD dwcbBufSizeService = 0;                 // ���滺�����Ĵ�С
      DWORD dwcbBytesNeededService;                 // ������������ΪNULL��0,�������軺������С
      // �򿪷��񷵻�һ��������
      hService = OpenService(hSCManager, pEnumServiceStatus[i].lpServiceName, SERVICE_QUERY_CONFIG);
      // ��ѯ�÷�������ò��������Ȼ�ȡ����Ļ�������С(�ֽڵ�λ)
      QueryServiceConfig(hService, NULL, 0, &dwcbBytesNeededService);
      // ������ʴ�С�Ļ�������ѯ�÷�������ò���
      lpServiceConfig = (LPQUERY_SERVICE_CONFIG)new BYTE[dwcbBytesNeededService];
      ZeroMemory(lpServiceConfig, dwcbBytesNeededService);
      QueryServiceConfig(hService, lpServiceConfig, dwcbBytesNeededService, &dwcbBytesNeededService);

      lvi.iSubItem = 2; lvi.pszText = (LPTSTR)arrlpStrStartType[lpServiceConfig->dwStartType];
      SendMessage(g_hwndList, LVM_SETITEM, 0, (LPARAM)&lvi);
      pItemData->m_dwStartType = lpServiceConfig->dwStartType;

      // ��4�У��ļ�·��
      lvi.iSubItem = 3; lvi.pszText = lpServiceConfig->lpBinaryPathName;
      SendMessage(g_hwndList, LVM_SETITEM, 0, (LPARAM)&lvi);

      delete[]lpServiceConfig;

      // ��5�У���������
      LPBYTE lpBufferService2;        // ���ط����������ò����Ļ�����ָ��
      DWORD dwcbBufSizeService2 = 0;  // ���滺�����Ĵ�С
      DWORD dwcbBytesNeededService2;  // ������������ΪNULL��0,�������軺������С
      // ��ѯ�÷�����������ò��������Ȼ�ȡ����Ļ�������С(�ֽڵ�λ)
      QueryServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, NULL, 0, &dwcbBytesNeededService2);
      // ������ʴ�С�Ļ�������ѯ�÷�����������ò���
      lpBufferService2 = new BYTE[dwcbBytesNeededService2];
      ZeroMemory(lpBufferService2, dwcbBytesNeededService2);
      QueryServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, lpBufferService2, dwcbBytesNeededService2, &dwcbBytesNeededService2);

      lvi.iSubItem = 4; lvi.pszText = ((LPSERVICE_DESCRIPTION)lpBufferService2)->lpDescription;
      SendMessage(g_hwndList, LVM_SETITEM, 0, (LPARAM)&lvi);

      delete[]lpBufferService2;

      CloseServiceHandle(hService);
   }

   // ����ʾ������������
   SendMessage(g_hwndList, LVM_SORTITEMS, 0, (LPARAM)CompareFunc);
   g_bAscendingDisplayName = ~g_bAscendingDisplayName;

   delete[]lpServices;
   // �رշ�����ƹ��������ݿ���
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
   // ��Ϊ�ǵ���GetMenu��ȡ�Ĳ˵�������������ǵ���LoadMenu���صĲ˵���Դ��
   // ����ÿ�ζ���Ҫ����ÿ���˵����״̬��ֻ����ӷ����ˢ���б��������˵���һֱ��������״̬
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

   // ��ȡѡ���б������Ŀ����
   lvi.mask = LVIF_PARAM; lvi.iItem = nSelected;
   SendMessage(g_hwndList, LVM_GETITEM, 0, (LPARAM)&lvi);
   pItemData = (PITEMDATA)(lvi.lParam);

   // ������������ֹͣ������ͣ���񡢼���������Щ�˵����в�Ӧ�����õ�
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

   // ������Ϊ�Զ���������Ϊ�ֶ���������Ϊ�ѽ�����Щ�˵����в�Ӧ�����õ�
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
   SC_HANDLE hSCManager = NULL;    // ������ƹ��������ݿ�ľ��
   SC_HANDLE hService;             // ������

   // ��ȡѡ���б������Ŀ����
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
   SC_HANDLE hSCManager = NULL;    // ������ƹ��������ݿ�ľ��
   SC_HANDLE hService;             // ������
   SERVICE_STATUS serviceStatus = { 0 };

   // ��ȡѡ���б������Ŀ����
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

   // ʵ�ʱ����Ӧ���ݷ��ص����·���״̬��Ϣ��SERVICE_STATUS�ṹ���к�������
   // ����Ӧ�ü򵥵�ʹ�ô��ݹ�����dwNewCurrentState����
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
   SC_HANDLE hSCManager = NULL;    // ������ƹ��������ݿ�ľ��
   SC_HANDLE hService;             // ������

   // ��ȡѡ���б������Ŀ����
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
   TCHAR szFile[MAX_PATH] = { 0 };             // �����û�ѡ����ļ����Ļ�����
   TCHAR szFileTitle[MAX_PATH] = { 0 };        // �����û���ѡ�ļ����ļ�������չ���Ļ�����
   OPENFILENAME ofn = { sizeof(OPENFILENAME) };
   static HWND hwndCombo;
   int nIndex, nCount;
   BOOL bRet = FALSE;

   TCHAR szBinaryPathName[MAX_PATH] = { 0 };   // �ļ�·��
   TCHAR szServiceName[256] = { 0 };           // ��������
   TCHAR szDisplayName[256] = { 0 };           // ��ʾ����
   TCHAR szDescription[1024] = { 0 };          // ��������
   DWORD dwStartType;                          // ��������

   switch (uMsg)
   {
   case WM_INITDIALOG:
      // ����б��������Ŀ����Ϊ���������������ֵ��Ĭ��ѡ���Զ�����
      hwndCombo = GetDlgItem(hwndDlg, IDC_COMBO_STARTTYPE);
      nIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("�Զ�����"));
      SendMessage(hwndCombo, CB_SETITEMDATA, nIndex, 0x00000002);
      nIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("�ֶ�����"));
      SendMessage(hwndCombo, CB_SETITEMDATA, nIndex, 0x00000003);
      nIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("����"));
      SendMessage(hwndCombo, CB_SETITEMDATA, nIndex, 0x00000004);
      SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);

      // ���÷������ơ���ʾ���ƺͷ��������༭�ؼ�������ַ�����
      SendMessage(GetDlgItem(hwndDlg, IDC_EDIT_SERVICENAME), EM_SETLIMITTEXT, _countof(szServiceName) - 1, 0);
      SendMessage(GetDlgItem(hwndDlg, IDC_EDIT_DISPLAYNAME), EM_SETLIMITTEXT, _countof(szDisplayName) - 1, 0);
      SendMessage(GetDlgItem(hwndDlg, IDC_EDIT_DESCRIPTION), EM_SETLIMITTEXT, _countof(szDescription) - 1, 0);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_BROWSE:
         ofn.hwndOwner = hwndDlg;
         ofn.lpstrFilter = TEXT("EXE�ļ�(*.exe)\0*.exe\0�����ļ�(*.*)\0*.*\0");
         ofn.lpstrFile = szFile;
         ofn.nMaxFile = _countof(szFile);
         ofn.lpstrFileTitle = szFileTitle;
         ofn.nMaxFileTitle = _countof(szFileTitle);
         ofn.lpstrTitle = TEXT("��ѡ��Ҫ�򿪵��ļ�");
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
            MessageBox(hwndDlg, TEXT("��������������·��"), TEXT("������ʾ"), MB_OK);
            return TRUE;
         }
         nCount = GetDlgItemText(hwndDlg, IDC_EDIT_SERVICENAME, szServiceName, _countof(szServiceName));
         if (nCount < 1)
         {
            MessageBox(hwndDlg, TEXT("�������������"), TEXT("������ʾ"), MB_OK);
            return TRUE;
         }
         nCount = GetDlgItemText(hwndDlg, IDC_EDIT_DISPLAYNAME, szDisplayName, _countof(szDisplayName));
         if (nCount < 1)
         {
            MessageBox(hwndDlg, TEXT("��������ʾ����"), TEXT("������ʾ"), MB_OK);
            return TRUE;
         }
         GetDlgItemText(hwndDlg, IDC_EDIT_DESCRIPTION, szDescription, _countof(szDescription));

         nIndex = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
         dwStartType = SendMessage(hwndCombo, CB_GETITEMDATA, nIndex, 0);

         // �����Զ��庯��CreateAService��������
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
   SC_HANDLE hSCManager = NULL;                    // ������ƹ��������ݿ�ľ��
   SC_HANDLE hService;                             // ������
   SERVICE_DESCRIPTION serviceDescription = { 0 }; // ��������

   hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
   if (!hSCManager)
      return FALSE;

   // ��������
   hService = CreateService(hSCManager, lpServiceName, lpDisplayName, dwDesiredAccess, dwServiceType, dwStartType, dwErrorControl, lpBinaryPathName, NULL, NULL, NULL, NULL, NULL);
   if (!hService)
   {
      CloseServiceHandle(hSCManager);
      return FALSE;
   }

   // ���÷�������
   if (!lpDescription)
   {
      serviceDescription.lpDescription = (LPTSTR)lpDescription;
      ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &serviceDescription);
   }

   // ˢ���б�����ʾ������������
   g_bAscendingDisplayName = FALSE;
   GetServiceList();

   // �ҵ�������б��������
   LVFINDINFO lvfi = { 0 };
   lvfi.flags = LVFI_STRING;
   lvfi.psz = lpDisplayName;
   int nIndex = SendMessage(g_hwndList, LVM_FINDITEM, -1, (LPARAM)&lvfi);
   // ����Ϊѡ��״̬
   LVITEM lvi = { 0 }; lvi.state = lvi.stateMask = LVIS_SELECTED;
   SendMessage(g_hwndList, LVM_SETITEMSTATE, nIndex, (LPARAM)&lvi);

   // ������ӵ��б���������ɼ���ͼ��
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
   SC_HANDLE hSCManager = NULL;    // ������ƹ��������ݿ�ľ��
   SC_HANDLE hService;             // ������

   // ��ȡѡ���б������Ŀ����
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