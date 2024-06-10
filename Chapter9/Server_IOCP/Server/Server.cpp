#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>           // Winsock2ͷ�ļ�
#include <ws2tcpip.h>           // inet_pton / inet_ntop��Ҫʹ�����ͷ�ļ�
#include <Mswsock.h>
#include <strsafe.h>
#include "resource.h"
#include "SOCKETOBJ.h"

#pragma comment(lib, "Ws2_32")  // Winsock2�����
#pragma comment(lib, "Mswsock") // Mswsock�����

// ��������
const int BUF_SIZE = 4096;

// I/O��������
enum IOOPERATION
{
   IO_ACCEPT, IO_READ, IO_WRITE
};

// OVERLAPPED�ṹ��I/OΨһ����
typedef struct _PERIODATA
{
   OVERLAPPED  m_overlapped;   // �ص��ṹ
   SOCKET      m_socket;       // ͨ���׽��־��
   WSABUF      m_wsaBuf;
   CHAR        m_szBuffer[BUF_SIZE];
   IOOPERATION m_ioOperation;  // ��������
}PERIODATA, * PPERIODATA;

// ȫ�ֱ���
HWND g_hwnd;            // ���ھ��
HWND g_hListContent;    // ���������б�򴰿ھ��
HWND g_hEditMsg;        // ��Ϣ����򴰿ھ��
HWND g_hBtnSend;        // ���Ͱ�ť���ھ��

SOCKET g_socketListen;      // �����׽��־��
HANDLE g_hCompletionPort;   // ��ɶ˿ھ��

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// �Ի����ʼ��
VOID OnInit(HWND hwndDlg);
// ������������ť
VOID OnStart();
// TrySubmitThreadpoolCallback�Ļص�����
VOID CALLBACK WorkerThreadProc(PTP_CALLBACK_INSTANCE Instance, PVOID Context);
// Ͷ�ݽ�������I/O����
BOOL PostAccept();
// Ͷ�ݷ�������I/O����
BOOL PostSend(SOCKET s, LPTSTR pStr, int nLen);
// Ͷ�ݽ�������I/O����
BOOL PostRecv(SOCKET s);
// ���·��Ͱ�ť
VOID OnSend();
// ��ȡ�߼���������������������
ULARGE_INTEGER GetProcessorInformation();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SERVER_DIALOG), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_INITDIALOG:
      OnInit(hwndDlg);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDCANCEL:
         // �ر���ɶ˿�
         if (g_hCompletionPort)
            CloseHandle(g_hCompletionPort);
         // �ر��׽��֣��ͷ�WinSock��
         if (g_socketListen != INVALID_SOCKET)
            closesocket(g_socketListen);
         WSACleanup();
         EndDialog(hwndDlg, IDCANCEL);
         break;

      case IDC_BTN_START:
         OnStart();
         break;

      case IDC_BTN_SEND:
         OnSend();
         break;
      }
      return TRUE;
   }

   return FALSE;
}

//////////////////////////////////////////////////////////////////////////
VOID OnInit(HWND hwndDlg)
{
   WSADATA wsa = { 0 };

   // 1����ʼ��WinSock��
   if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
   {
      MessageBox(g_hwnd, TEXT("��ʼ��WinSock��ʧ�ܣ�"), TEXT("WSAStartup Error"), MB_OK);
      return;
   }

   // ��ʼ���ٽ�����������ͬ�����׽��ֶ���ķ���
   InitializeCriticalSection(&g_cs);

   g_hwnd = hwndDlg;
   g_hListContent = GetDlgItem(hwndDlg, IDC_LIST_CONTENT);
   g_hEditMsg = GetDlgItem(hwndDlg, IDC_EDIT_MSG);
   g_hBtnSend = GetDlgItem(hwndDlg, IDC_BTN_SEND);

   EnableWindow(g_hBtnSend, FALSE);
}

VOID OnStart()
{
   ULARGE_INTEGER uli = { 0 };

   // 2���������ڼ������пͻ���������׽���
   g_socketListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
   if (g_socketListen == INVALID_SOCKET)
   {
      MessageBox(g_hwnd, TEXT("���������׽���ʧ�ܣ�"), TEXT("socket Error"), MB_OK);
      WSACleanup();
      return;
   }

   // 3���������׽�����ָ����IP��ַ���˿ں�����
   sockaddr_in sockAddr;
   sockAddr.sin_family = AF_INET;
   sockAddr.sin_port = htons(8000);
   sockAddr.sin_addr.S_un.S_addr = INADDR_ANY;
   if (bind(g_socketListen, (sockaddr*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR)
   {
      MessageBox(g_hwnd, TEXT("�������׽�����ָ����IP��ַ���˿ں�����ʧ�ܣ�"),
         TEXT("bind Error"), MB_OK);
      closesocket(g_socketListen);
      WSACleanup();
      return;
   }

   // 4��ʹ�����׽��ֽ������(�ȴ�������)״̬
   if (listen(g_socketListen, SOMAXCONN) == SOCKET_ERROR)
   {
      MessageBox(g_hwnd, TEXT("ʹ�����׽��ֽ������(�ȴ�������)״̬ʧ�ܣ�"),
         TEXT("listen Error"), MB_OK);
      closesocket(g_socketListen);
      WSACleanup();
      return;
   }
   // ������������...
   MessageBox(g_hwnd, TEXT("������������..."), TEXT("���������ɹ�"), MB_OK);
   EnableWindow(GetDlgItem(g_hwnd, IDC_BTN_START), FALSE);

   // 5��
   // ����I/O��ɶ˿ڣ���GetQueuedCompletionStatus��������FALSE
   // ���Ҵ������ΪERROR_ABANDONED_WAIT_0��ʱ�����ȷ����ɶ˿��ѹر�
   g_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

   // �����̳߳��й����̵߳Ļص��������̳߳ػ������ι������߳�
   TrySubmitThreadpoolCallback(WorkerThreadProc, NULL, NULL);

   // ��Ӽ����׽��ֽڵ�
   PSOCKETOBJ pSocketObj = CreateSocketObj(g_socketListen);
   // �������׽�������ɶ˿�g_hCompletionPort�������pSocket��Ϊ���Ψһ����
   CreateIoCompletionPort((HANDLE)g_socketListen, g_hCompletionPort, (ULONG_PTR)pSocketObj, 0);

   // �ڴ���Ͷ���������� * 2����������I/O����GetProcessorInformation���Զ��庯��
   uli = GetProcessorInformation();
   for (DWORD i = 0; i < uli.HighPart * 2; i++)
      PostAccept();
}

// TrySubmitThreadpoolCallback�Ļص�����
VOID CALLBACK WorkerThreadProc(PTP_CALLBACK_INSTANCE Instance, PVOID Context)
{
   sockaddr_in* pRemoteSockaddr;
   sockaddr_in* pLocalSockaddr;
   int nAddrlen = sizeof(sockaddr_in);
   PSOCKETOBJ pSocketObj = NULL;   // �������׽���������ĵ��������
   PPERIODATA pPerIOData = NULL;   // ����I/O��������ָ����OVERLAPPED�ṹ�ĵ�ַ
   DWORD dwTrans;                  // ��������ɵ�I/O���������ͻ���յ��ֽ���
   PSOCKETOBJ pSocket;             // �������ӳɹ��Ժ����һ���׽�����Ϣ�ڵ�
   BOOL bRet;
   PSOCKETOBJ p;
   CHAR szBuf[BUF_SIZE] = { 0 };

   while (TRUE)
   {
      bRet = GetQueuedCompletionStatus(g_hCompletionPort, &dwTrans,
         (PULONG_PTR)&pSocketObj, (LPOVERLAPPED*)&pPerIOData, INFINITE);
      if (!bRet)
      {
         if (GetLastError() == ERROR_ABANDONED_WAIT_0)   // ��ɶ˿��ѹر�
         {
            break;
         }
         else                                            // �ͻ����ѹر�
         {
            ZeroMemory(szBuf, BUF_SIZE);
            wsprintf(szBuf, "�ͻ���[%s:%d] ���˳���", pSocketObj->m_szIP, pSocketObj->m_usPort);
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);

            // �ͷž��Ψһ����
            FreeSocketObj(pSocketObj);

            // �ͷ�I/OΨһ����
            delete pPerIOData;

            // ���û�пͻ��������ˣ����÷��Ͱ�ť
            if (g_nTotalClient == 1)                    // �����׽���ռ����һ���ṹ��������1
               EnableWindow(g_hBtnSend, FALSE);

            continue;
         }
      }

      // ������ɵ�I/O�������д������е�����������ӻ������շ������Ѿ������
      switch (pPerIOData->m_ioOperation)
      {
      case IO_ACCEPT:
      {
         // ���������ѳɹ��������׽�����Ϣ�ṹ�����һ���ڵ�
         pSocket = CreateSocketObj(pPerIOData->m_socket);

         // ��ͨ���׽�������ɶ˿�g_hCompletionPort�������pSocket��Ϊ���Ψһ����
         CreateIoCompletionPort((HANDLE)pSocket->m_socket,
            g_hCompletionPort, (ULONG_PTR)pSocket, 0);

         ZeroMemory(szBuf, BUF_SIZE);
         GetAcceptExSockaddrs(pPerIOData->m_szBuffer, 0, sizeof(sockaddr_in) + 16,
            sizeof(sockaddr_in) + 16, (LPSOCKADDR*)&pLocalSockaddr, &nAddrlen,
            (LPSOCKADDR*)&pRemoteSockaddr, &nAddrlen);
         inet_ntop(AF_INET, &pRemoteSockaddr->sin_addr.S_un.S_addr,
            pSocket->m_szIP, _countof(pSocket->m_szIP));
         pSocket->m_usPort = ntohs(pRemoteSockaddr->sin_port);
         wsprintf(szBuf, "�ͻ���[%s:%d] �����ӣ�", pSocket->m_szIP, pSocket->m_usPort);
         SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);
         EnableWindow(g_hBtnSend, TRUE);

         // �ͷ�I/OΨһ����
         delete pPerIOData;

         // Ͷ��һ��������������
         PostRecv(pSocket->m_socket);

         // ����Ͷ��һ��������������
         PostAccept();
      }
      break;

      case IO_READ:
      {
         ZeroMemory(szBuf, BUF_SIZE);
         wsprintf(szBuf, "�ͻ���[%s:%d]˵��%s", pSocketObj->m_szIP,
            pSocketObj->m_usPort, pPerIOData->m_szBuffer);
         SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);

         // �ͷ�I/OΨһ����
         delete pPerIOData;

         // ���յ������ݷַ���ÿһ���ͻ���
         p = g_pSocketObjHeader;
         while (p != NULL)
         {
            if (p->m_socket != g_socketListen && p->m_socket != pSocketObj->m_socket)
               PostSend(p->m_socket, szBuf, strlen(szBuf));

            p = p->m_pNext;
         }

         // ����Ͷ�ݽ�����������
         PostRecv(pSocketObj->m_socket);
      }
      break;

      case IO_WRITE:
         // �ͷ�I/OΨһ����
         delete pPerIOData;
         break;
      }
   }
}

// Ͷ�ݽ�������I/O����
BOOL PostAccept()
{
   PPERIODATA pPerIOData = new PERIODATA;
   ZeroMemory(&pPerIOData->m_overlapped, sizeof(OVERLAPPED));
   pPerIOData->m_ioOperation = IO_ACCEPT;

   SOCKET socketAccept = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
   pPerIOData->m_socket = socketAccept;

   BOOL bRet = AcceptEx(g_socketListen, socketAccept, pPerIOData->m_szBuffer, 0,
      sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, (LPOVERLAPPED)pPerIOData);
   if (!bRet)
   {
      if (WSAGetLastError() != WSA_IO_PENDING)
         return FALSE;
   }

   return TRUE;
}

// Ͷ�ݷ�������I/O����
BOOL PostSend(SOCKET s, LPTSTR pStr, int nLen)
{
   DWORD dwFlags = 0;
   PPERIODATA pPerIOData = new PERIODATA;
   ZeroMemory(&pPerIOData->m_overlapped, sizeof(OVERLAPPED));
   pPerIOData->m_ioOperation = IO_WRITE;
   ZeroMemory(pPerIOData->m_szBuffer, BUF_SIZE);
   StringCchCopy(pPerIOData->m_szBuffer, BUF_SIZE, pStr);
   pPerIOData->m_wsaBuf.buf = pPerIOData->m_szBuffer;
   pPerIOData->m_wsaBuf.len = BUF_SIZE;

   int nRet = WSASend(s, &pPerIOData->m_wsaBuf, 1, NULL, dwFlags, (LPOVERLAPPED)pPerIOData, NULL);
   if (nRet == SOCKET_ERROR)
   {
      if (WSAGetLastError() != WSA_IO_PENDING)
         return FALSE;
   }

   return TRUE;
}

// Ͷ�ݽ�������I/O����
BOOL PostRecv(SOCKET s)
{
   DWORD dwFlags = 0;
   PPERIODATA pPerIOData = new PERIODATA;
   ZeroMemory(&pPerIOData->m_overlapped, sizeof(OVERLAPPED));
   pPerIOData->m_ioOperation = IO_READ;
   ZeroMemory(pPerIOData->m_szBuffer, BUF_SIZE);
   pPerIOData->m_wsaBuf.buf = pPerIOData->m_szBuffer;
   pPerIOData->m_wsaBuf.len = BUF_SIZE;

   int nRet = WSARecv(s, &pPerIOData->m_wsaBuf, 1, NULL, &dwFlags, (LPOVERLAPPED)pPerIOData, NULL);
   if (nRet == SOCKET_ERROR)
   {
      if (WSAGetLastError() != WSA_IO_PENDING)
         return FALSE;
   }

   return TRUE;
}

// ���·��Ͱ�ť
VOID OnSend()
{
   CHAR szBuf[BUF_SIZE] = { 0 };
   CHAR szMsg[BUF_SIZE] = { 0 };

   GetWindowText(g_hEditMsg, szBuf, BUF_SIZE);
   wsprintf(szMsg, "������˵��%s", szBuf);
   SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szMsg);
   SetWindowText(g_hEditMsg, "");

   // ��ÿһ���ͻ��˷�������
   PSOCKETOBJ p = g_pSocketObjHeader;
   while (p != NULL)
   {
      if (p->m_socket != g_socketListen)
         PostSend(p->m_socket, szMsg, strlen(szMsg));

      p = p->m_pNext;
   }
}

//////////////////////////////////////////////////////////////////////////
DWORD CountSetBits(ULONG_PTR bitMask)
{
   DWORD     LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
   DWORD     bitSetCount = 0;
   ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;

   for (DWORD i = 0; i <= LSHIFT; ++i)
   {
      bitSetCount += ((bitMask & bitTest) ? 1 : 0);
      bitTest /= 2;
   }

   return bitSetCount;
}

ULARGE_INTEGER GetProcessorInformation()
{
   PSYSTEM_LOGICAL_PROCESSOR_INFORMATION pBuf = NULL;    // ������ָ��
   PSYSTEM_LOGICAL_PROCESSOR_INFORMATION pTemp = NULL;   // ��ʱָ��
   DWORD          dwReturnedLength = 0;       // GetLogicalProcessorInformation�������ò���
   DWORD          dwLogicalProcessorCount = 0;// �߼�����������
   DWORD          dwProcessorCoreCount = 0;   // ������������
   DWORD          dwByteOffset = 0;           // 
   ULARGE_INTEGER uli = { 0 };                // ����ֵ���͡���DWORD�ֱ��ʾ�߼���������������

   // ��1�ε���
   GetLogicalProcessorInformation(pBuf, &dwReturnedLength);
   pBuf = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)new BYTE[dwReturnedLength];

   // ��2�ε���
   GetLogicalProcessorInformation(pBuf, &dwReturnedLength);

   pTemp = pBuf;
   while (dwByteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= dwReturnedLength)
   {
      if (pTemp->Relationship == RelationProcessorCore)
      {
         // ������������
         dwProcessorCoreCount++;
         // �߼�������������һ�����̺߳��Ŀ����ж���߼�������
         dwLogicalProcessorCount += CountSetBits(pTemp->ProcessorMask);
      }

      dwByteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
      pTemp++;
   }

   delete[] pBuf;
   uli.LowPart = dwLogicalProcessorCount;
   uli.HighPart = dwProcessorCoreCount;
   return uli;
}