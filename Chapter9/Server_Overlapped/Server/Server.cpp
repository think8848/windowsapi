#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>           // Winsock2ͷ�ļ�
#include <ws2tcpip.h>           // inet_pton / inet_ntop��Ҫʹ�����ͷ�ļ�
#include <Mswsock.h>
#include <strsafe.h>
#include "resource.h"
#include "SOCKETOBJ.h"
#include "PERIODATA.h"

#pragma comment(lib, "Ws2_32")  // Winsock2�����
#pragma comment(lib, "Mswsock") // Mswsock�����

// ȫ�ֱ���
HWND g_hwnd;            // ���ھ��
HWND g_hListContent;    // ���������б�򴰿ھ��
HWND g_hEditMsg;        // ��Ϣ����򴰿ھ��
HWND g_hBtnSend;        // ���Ͱ�ť���ھ��

SOCKET g_socketListen = INVALID_SOCKET; // �����׽��־��

WSAEVENT g_eventArray[WSA_MAXIMUM_WAIT_EVENTS]; // �����¼�����������
int g_nTotalEvent;                              // �����¼�����������

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// �Ի����ʼ��
VOID OnInit(HWND hwndDlg);
// ������������ť
VOID OnStart();
// �������¼�������ѭ���ȴ������¼�
DWORD WINAPI WaitProc(LPVOID lpParam);
// Ͷ�ݽ�������I/O����
BOOL PostAccept();
// Ͷ�ݷ�������I/O����
BOOL PostSend(PSOCKETOBJ pSocketObj, LPTSTR pStr, int nLen);
// Ͷ�ݽ�������I/O����
BOOL PostRecv(PSOCKETOBJ pSocketObj);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SERVER_DIALOG), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CHAR szBuf[BUF_SIZE] = { 0 };
    CHAR szMsg[BUF_SIZE] = { 0 };
    PSOCKETOBJ p;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        OnInit(hwndDlg);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
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
            GetWindowText(g_hEditMsg, szBuf, BUF_SIZE);
            wsprintf(szMsg, "������˵��%s", szBuf);
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szMsg);
            SetWindowText(g_hEditMsg, "");

            // ��ÿһ���ͻ��˷�������
            p = g_pSocketObjHeader;
            while (p != NULL)
            {
                PostSend(p, szMsg, strlen(szMsg));

                p = p->m_pNext;
            }
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

    // �������¼�������ѭ���ȴ������¼���������ֻ����һ���߳�
    HANDLE hThread = NULL;
    if ((hThread = CreateThread(NULL, 0, WaitProc, NULL, 0, NULL)) != NULL)
        CloseHandle(hThread);

    // �ڴ���Ͷ�ݼ�����������I/O����
    for (int i = 0; i < 2; i++)
        PostAccept();
}

DWORD WINAPI WaitProc(LPVOID lpParam)
{
    sockaddr_in* pRemoteSockaddr;
    sockaddr_in* pLocalSockaddr;
    int nAddrlen = sizeof(sockaddr_in);
    int nIndex;                             // WSAWaitForMultipleEvents����ֵ
    PPERIODATA pPerIOData = NULL;           // �Զ����ص��ṹָ��
    PSOCKETOBJ pSocketObj = NULL;           // �׽��ֶ���ṹָ��
    PSOCKETOBJ pSocketObjAccept = NULL;     // �׽��ֶ���ṹָ�룬�������ӳɹ��Ժ󴴽���
    DWORD dwTransfer;                       // WSAGetOverlappedResult��������
    DWORD dwFlags = 0;                      // WSAGetOverlappedResult��������
    BOOL bRet = FALSE;
    CHAR szBuf[BUF_SIZE] = { 0 };

    while (TRUE)
    {
        // �������¼������ϵȴ������κ�һ���¼����󴥷��������ͷ���
        nIndex = WSAWaitForMultipleEvents(g_nTotalEvent, g_eventArray, FALSE, 1000, FALSE);
        if (nIndex == WSA_WAIT_TIMEOUT || nIndex == WSA_WAIT_FAILED)
            continue;

        nIndex = nIndex - WSA_WAIT_EVENT_0;
        WSAResetEvent(g_eventArray[nIndex]);

        // ��ȡָ���׽������ص�I/O�����Ľ��
        pPerIOData = FindPerIOData(g_eventArray[nIndex]);
        pSocketObj = FindSocketObj(pPerIOData->m_socket);

        bRet = FALSE;
        bRet = WSAGetOverlappedResult(pPerIOData->m_socket, &pPerIOData->m_overlapped,
            &dwTransfer, TRUE, &dwFlags);
        if (!bRet)
        {
            if (pSocketObj != NULL)
            {
                ZeroMemory(szBuf, BUF_SIZE);
                wsprintf(szBuf, "�ͻ���[%s:%d] ���˳���", pSocketObj->m_szIP, pSocketObj->m_usPort);
                SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);

                FreeSocketObj(pSocketObj);
            }

            // �ͷ��Զ����ص��ṹ
            FreePerIOData(pPerIOData);
            // �����¼���������
            for (int j = nIndex; j < g_nTotalEvent - 1; j++)
                g_eventArray[j] = g_eventArray[j + 1];
            g_nTotalEvent--;

            // ���û�пͻ��������ˣ����÷��Ͱ�ť
            if (g_nTotalClient == 0)
                EnableWindow(g_hBtnSend, FALSE);

            continue;
        }

        // �����ѳɹ���ɵ�I/O����
        switch (pPerIOData->m_ioOperation)
        {
        case IO_ACCEPT:
        {
            pSocketObjAccept = CreateSocketObj(pPerIOData->m_socket);

            ZeroMemory(szBuf, BUF_SIZE);
            GetAcceptExSockaddrs(pPerIOData->m_szBuffer, 0, sizeof(sockaddr_in) + 16,
                sizeof(sockaddr_in) + 16, (LPSOCKADDR*)&pLocalSockaddr, &nAddrlen,
                (LPSOCKADDR*)&pRemoteSockaddr, &nAddrlen);
            inet_ntop(AF_INET, &pRemoteSockaddr->sin_addr.S_un.S_addr,
                pSocketObjAccept->m_szIP, _countof(pSocketObjAccept->m_szIP));
            pSocketObjAccept->m_usPort = ntohs(pRemoteSockaddr->sin_port);
            wsprintf(szBuf, "�ͻ���[%s:%d] �����ӣ�", pSocketObjAccept->m_szIP,
                pSocketObjAccept->m_usPort);
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);
            EnableWindow(g_hBtnSend, TRUE);

            // �ͷ��Զ����ص��ṹ
            FreePerIOData(pPerIOData);
            // �����¼���������
            for (int j = nIndex; j < g_nTotalEvent - 1; j++)
                g_eventArray[j] = g_eventArray[j + 1];
            g_nTotalEvent--;

            PostRecv(pSocketObjAccept);
            PostAccept();
        }
        break;

        case IO_READ:
            if (dwTransfer > 0)
            {
                ZeroMemory(szBuf, BUF_SIZE);
                wsprintf(szBuf, "�ͻ���[%s:%d]˵��%s", pSocketObj->m_szIP,
                    pSocketObj->m_usPort, pPerIOData->m_szBuffer);
                SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);

                // ���յ������ݷַ���ÿһ���ͻ���
                PSOCKETOBJ p = g_pSocketObjHeader;
                while (p != NULL)
                {
                    if (p->m_socket != pPerIOData->m_socket)
                        PostSend(p, szBuf, strlen(szBuf));

                    p = p->m_pNext;
                }

                PostRecv(pSocketObj);
            }
            else
            {
                ZeroMemory(szBuf, BUF_SIZE);
                wsprintf(szBuf, "�ͻ���[%s:%d] ���˳���", pSocketObj->m_szIP, pSocketObj->m_usPort);
                SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);

                FreeSocketObj(pSocketObj);

                // ���û�пͻ��������ˣ����÷��Ͱ�ť
                if (g_nTotalClient == 0)
                    EnableWindow(g_hBtnSend, FALSE);
            }

            // �ͷ��Զ����ص��ṹ
            FreePerIOData(pPerIOData);
            // �����¼���������
            for (int j = nIndex; j < g_nTotalEvent - 1; j++)
                g_eventArray[j] = g_eventArray[j + 1];
            g_nTotalEvent--;
            break;

        case IO_WRITE:
            if (dwTransfer <= 0)
            {
                ZeroMemory(szBuf, BUF_SIZE);
                wsprintf(szBuf, "�ͻ���[%s:%d] ���˳���", pSocketObj->m_szIP, pSocketObj->m_usPort);
                SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);

                FreeSocketObj(pSocketObj);

                // ���û�пͻ��������ˣ����÷��Ͱ�ť
                if (g_nTotalClient == 0)
                    EnableWindow(g_hBtnSend, FALSE);
            }

            // �ͷ��Զ����ص��ṹ
            FreePerIOData(pPerIOData);
            // �����¼���������
            for (int j = nIndex; j < g_nTotalEvent - 1; j++)
                g_eventArray[j] = g_eventArray[j + 1];
            g_nTotalEvent--;
            break;
        }
    }
}

// Ͷ�ݽ�������I/O����
BOOL PostAccept()
{
    SOCKET socketAccept = INVALID_SOCKET;   // ͨ���׽��־��
    BOOL bRet = FALSE;

    socketAccept = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (socketAccept == INVALID_SOCKET)
        return FALSE;

    // Ϊ�������Ӵ���һ���Զ����ص��ṹ
    PPERIODATA pPerIOData = CreatePerIOData(socketAccept);
    pPerIOData->m_ioOperation = IO_ACCEPT;

    // �¼���������
    g_eventArray[g_nTotalEvent] = pPerIOData->m_overlapped.hEvent;
    g_nTotalEvent++;

    bRet = AcceptEx(g_socketListen, socketAccept, pPerIOData->m_szBuffer, 0,
        sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, (LPOVERLAPPED)pPerIOData);
    if (!bRet)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
            return FALSE;
    }

    return TRUE;
}

// Ͷ�ݷ�������I/O����
BOOL PostSend(PSOCKETOBJ pSocketObj, LPTSTR pStr, int nLen)
{
    DWORD dwFlags = 0;

    // Ϊ�������ݴ���һ���Զ����ص��ṹ
    PPERIODATA pPerIOData = CreatePerIOData(pSocketObj->m_socket);
    ZeroMemory(pPerIOData->m_szBuffer, BUF_SIZE);
    StringCchCopy(pPerIOData->m_szBuffer, BUF_SIZE, pStr);
    pPerIOData->m_wsaBuf.buf = pPerIOData->m_szBuffer;
    pPerIOData->m_wsaBuf.len = nLen;

    pPerIOData->m_ioOperation = IO_WRITE;

    // �¼���������
    g_eventArray[g_nTotalEvent] = pPerIOData->m_overlapped.hEvent;
    g_nTotalEvent++;

    int nRet = WSASend(pSocketObj->m_socket, &pPerIOData->m_wsaBuf, 1,
        NULL, dwFlags, (LPOVERLAPPED)pPerIOData, NULL);
    if (nRet == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
            return FALSE;
    }

    return TRUE;
}

// Ͷ�ݽ�������I/O����
BOOL PostRecv(PSOCKETOBJ pSocketObj)
{
    DWORD dwFlags = 0;

    // Ϊ�������ݴ���һ���Զ����ص��ṹ
    PPERIODATA pPerIOData = CreatePerIOData(pSocketObj->m_socket);
    ZeroMemory(pPerIOData->m_szBuffer, BUF_SIZE);
    pPerIOData->m_wsaBuf.buf = pPerIOData->m_szBuffer;
    pPerIOData->m_wsaBuf.len = BUF_SIZE;

    pPerIOData->m_ioOperation = IO_READ;

    // �¼���������
    g_eventArray[g_nTotalEvent] = pPerIOData->m_overlapped.hEvent;
    g_nTotalEvent++;

    int nRet = WSARecv(pSocketObj->m_socket, &pPerIOData->m_wsaBuf, 1,
        NULL, &dwFlags, (LPOVERLAPPED)pPerIOData, NULL);
    if (nRet == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
            return FALSE;
    }

    return TRUE;
}