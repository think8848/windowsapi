#include <winsock2.h>           // Winsock2ͷ�ļ�
#include <ws2tcpip.h>           // inet_pton / inet_ntop��Ҫʹ�����ͷ�ļ�
#include "resource.h"
#include "SOCKETOBJ.h"

#pragma comment(lib, "Ws2_32")  // Winsock2�����

// ��������
const int BUF_SIZE = 4096;

// ȫ�ֱ���
HWND g_hwnd;            // ���ھ��
HWND g_hListContent;    // ���������б�򴰿ھ��
HWND g_hEditMsg;        // ��Ϣ����򴰿ھ��
HWND g_hBtnSend;        // ���Ͱ�ť���ھ��

SOCKET g_socketListen = INVALID_SOCKET;         // �����׽��־��

WSAEVENT g_eventArray[WSA_MAXIMUM_WAIT_EVENTS]; // �����¼�����������
SOCKET g_socketArray[WSA_MAXIMUM_WAIT_EVENTS];  // �����׽��־������
int g_nTotalEvent;                              // �����¼�����������

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// �Ի����ʼ��
VOID OnInit(HWND hwndDlg);
// ������������ť
VOID OnStart();
// ���·��Ͱ�ť
VOID OnSend();
// �������¼�������ѭ���ȴ������¼�
DWORD WINAPI WaitProc(LPVOID lpParam);

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
    // 2���������ڼ������пͻ���������׽���
    g_socketListen = socket(AF_INET, SOCK_STREAM, 0);
    if (g_socketListen == INVALID_SOCKET)
    {
        MessageBox(g_hwnd, TEXT("���������׽���ʧ�ܣ�"), TEXT("socket Error"), MB_OK);
        WSACleanup();
        return;
    }

    // �����¼�����Ϊ�����׽��ְѸ��¼�������һЩ�����¼������
    WSAEVENT hEvent = WSACreateEvent();
    WSAEventSelect(g_socketListen, hEvent, FD_ACCEPT/* | FD_CLOSE*/);
    // ���¼�����ͼ����׽��ַ������������
    g_eventArray[g_nTotalEvent] = hEvent;
    g_socketArray[g_nTotalEvent] = g_socketListen;
    g_nTotalEvent++;

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

    // ����һ�����߳��������¼�������ѭ���ȴ������¼�
    HANDLE hThread = NULL;
    if ((hThread = CreateThread(NULL, 0, WaitProc, NULL, 0, NULL)) != NULL)
        CloseHandle(hThread);
}

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
        send(p->m_socket, szMsg, strlen(szMsg), 0);

        p = p->m_pNext;
    }
}

DWORD WINAPI WaitProc(LPVOID lpParam)
{
    SOCKET socketAccept = INVALID_SOCKET;   // ͨ���׽��־��
    sockaddr_in sockAddrClient;
    int nAddrlen = sizeof(sockaddr_in);
    int nIndex;                             // WSAWaitForMultipleEvents����ֵ
    WSANETWORKEVENTS networkEvents;         // WSAEnumNetworkEvents�����õĽṹ
    WSAEVENT hEvent = NULL;
    PSOCKETOBJ pSocketObj;
    int nRet = SOCKET_ERROR;                // I/O��������ֵ
    CHAR szBuf[BUF_SIZE] = { 0 };
    CHAR szMsg[BUF_SIZE] = { 0 };

    while (TRUE)
    {
        // �������¼������ϵȴ������κ�һ���¼����󴥷��������ͷ���
        nIndex = WSAWaitForMultipleEvents(g_nTotalEvent, g_eventArray, FALSE, WSA_INFINITE, FALSE);
        nIndex = nIndex - WSA_WAIT_EVENT_0;

        // �鿴�������¼������Ӧ���׽��ַ�������Щ�����¼�
        WSAEnumNetworkEvents(g_socketArray[nIndex], g_eventArray[nIndex], &networkEvents);
        // ���ܿͻ�������FD_ACCEPT�����¼�
        if (networkEvents.lNetworkEvents & FD_ACCEPT)
        {
            if (g_nTotalEvent >= WSA_MAXIMUM_WAIT_EVENTS)
            {
                MessageBox(g_hwnd, TEXT("�ͻ���������̫�࣡"), TEXT("accept Error"), MB_OK);
                continue;
            }

            socketAccept = accept(g_socketListen, (sockaddr*)&sockAddrClient, &nAddrlen);
            if (socketAccept == INVALID_SOCKET)
            {
                Sleep(100);
                continue;
            }

            // 6�����ܿͻ�����������ɹ�
            // �����¼�����Ϊͨ���׽��ְѸ��¼�������һЩ�����¼������
            hEvent = WSACreateEvent();
            WSAEventSelect(socketAccept, hEvent, FD_READ | FD_WRITE | FD_CLOSE);
            // ���¼������ͨ���׽��ַ������������
            g_eventArray[g_nTotalEvent] = hEvent;
            g_socketArray[g_nTotalEvent] = socketAccept;
            g_nTotalEvent++;

            ZeroMemory(szBuf, BUF_SIZE);
            // ����һ���׽��ֶ��󣬱���ͻ���IP��ַ���˿ں�
            PSOCKETOBJ pSocketObj = CreateSocketObj(socketAccept);
            inet_ntop(AF_INET, &sockAddrClient.sin_addr.S_un.S_addr,
                pSocketObj->m_szIP, _countof(pSocketObj->m_szIP));
            pSocketObj->m_usPort = ntohs(sockAddrClient.sin_port);
            wsprintf(szBuf, "�ͻ���[%s:%d] �����ӣ�",
                pSocketObj->m_szIP, pSocketObj->m_usPort);
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);
            EnableWindow(g_hBtnSend, TRUE);
        }
        // ���տͻ�������FD_READ�����¼�
        else if (networkEvents.lNetworkEvents & FD_READ)
        {
            pSocketObj = FindSocketObj(g_socketArray[nIndex]);
            ZeroMemory(szBuf, BUF_SIZE);
            nRet = SOCKET_ERROR;
            nRet = recv(g_socketArray[nIndex], szBuf, BUF_SIZE, 0);
            if (nRet > 0)   // ���յ��ͻ�������
            {
                ZeroMemory(szMsg, BUF_SIZE);
                wsprintf(szMsg, "�ͻ���[%s:%d]˵��%s",
                    pSocketObj->m_szIP, pSocketObj->m_usPort, szBuf);
                SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szMsg);

                // ���յ������ݷַ���ÿһ���ͻ���
                PSOCKETOBJ p = g_pSocketObjHeader;
                while (p != NULL)
                {
                    if (p->m_socket != g_socketArray[nIndex])
                        send(p->m_socket, szMsg, strlen(szMsg), 0);

                    p = p->m_pNext;
                }
            }
        }
        // ��������FD_WRITE�����¼�����������Ҫ������Ϊ�ǰ��·��Ͱ�ť�ŷ���
        else if (networkEvents.lNetworkEvents & FD_WRITE)
        {
        }
        // �ͻ������ӹر�FD_CLOSE�����¼�
        else if (networkEvents.lNetworkEvents & FD_CLOSE)
        {
            ZeroMemory(szMsg, BUF_SIZE);
            pSocketObj = FindSocketObj(g_socketArray[nIndex]);
            wsprintf(szMsg, "�ͻ���[%s:%d] ���˳���", pSocketObj->m_szIP, pSocketObj->m_usPort);
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szMsg);
            FreeSocketObj(pSocketObj);

            // �����¼������׽�������
            for (int j = nIndex; j < g_nTotalEvent - 1; j++)
            {
                g_eventArray[j] = g_eventArray[j + 1];
                g_socketArray[j] = g_socketArray[j + 1];
            }
            g_nTotalEvent--;

            // ���û�пͻ��������ˣ����÷��Ͱ�ť
            if (g_nTotalClient == 0)
                EnableWindow(g_hBtnSend, FALSE);
        }
    }
}