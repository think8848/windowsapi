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
// ��ȡ�����б�
BOOL GetServiceList();
// �б���ͼ�ؼ�����ص�����
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