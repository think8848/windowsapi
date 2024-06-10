#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include <winsock2.h>           // Winsock2ͷ�ļ�
#include <ws2tcpip.h>           // inet_pton / inet_ntop��Ҫʹ�����ͷ�ļ�
#include "resource.h"
#include "SOCKETOBJ.h"

#pragma comment(lib, "Ws2_32")  // Winsock2�����

// ��������
const int BUF_SIZE = 4096;
// �Զ��������¼�֪ͨ��Ϣ����
const int WM_SOCKET = WM_APP + 1;

// ȫ�ֱ���
HWND g_hwnd;            // ���ھ��
HWND g_hListContent;    // ���������б�򴰿ھ��
HWND g_hEditMsg;        // ��Ϣ����򴰿ھ��
HWND g_hBtnSend;        // ���Ͱ�ť���ھ��

SOCKET g_socketListen = INVALID_SOCKET; // �����׽��־��

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// �Ի����ʼ��
VOID OnInit(HWND hwndDlg);
// ������������ť
VOID OnStart();
// ���·��Ͱ�ť
VOID OnSend();
// ����������ÿһ���ͻ�������
VOID OnRecv(SOCKET s);
// ���ܿͻ�����������
VOID OnAccept();
// �ͻ��˹ر�����
VOID OnClose(SOCKET s);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SERVER_DIALOG), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SOCKET s;

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

    case WM_SOCKET:
        // wParam������ʶ�˷��������¼����׽��־��
        s = wParam;

        switch (WSAGETSELECTEVENT(lParam))
        {
        case FD_ACCEPT: // ���ܿͻ�������
            OnAccept();
            break;

        case FD_READ:   // ���տͻ�������
            OnRecv(s);
            break;

        case FD_WRITE:  // �������ݣ���������Ҫ������Ϊ�ǰ��·��Ͱ�ť�ŷ���
            break;

        case FD_CLOSE:  // �ͻ������ӹر�
            OnClose(s);
            break;
        }
        return TRUE;
    }

    return 0;
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

    // ���ü����׽���Ϊ�����¼�������Ϣ֪ͨ
    WSAAsyncSelect(g_socketListen, g_hwnd, WM_SOCKET, FD_ACCEPT/* | FD_CLOSE*/);

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

    // 5������һ�����߳�ѭ���ȴ���������
    // CloseHandle(CreateThread(NULL, 0, AcceptProc, NULL, 0, NULL));
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

VOID OnAccept()
{
    SOCKET socketAccept = INVALID_SOCKET;   // ͨ���׽��־��
    sockaddr_in sockAddrClient;
    int nAddrlen = sizeof(sockaddr_in);

    socketAccept = accept(g_socketListen, (sockaddr*)&sockAddrClient, &nAddrlen);
    if (socketAccept == INVALID_SOCKET)
        return;

    // ����ͨ���׽���Ϊ�����¼�������Ϣ֪ͨ����
    WSAAsyncSelect(socketAccept, g_hwnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);

    // 6�����ܿͻ�����������ɹ�
    CHAR szBuf[BUF_SIZE] = { 0 };
    PSOCKETOBJ pSocketObj = CreateSocketObj(socketAccept);
    inet_ntop(AF_INET, &sockAddrClient.sin_addr.S_un.S_addr,
        pSocketObj->m_szIP, _countof(pSocketObj->m_szIP));
    pSocketObj->m_usPort = ntohs(sockAddrClient.sin_port);

    wsprintf(szBuf, "�ͻ���[%s:%d] �����ӣ�", pSocketObj->m_szIP, pSocketObj->m_usPort);
    SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);
    EnableWindow(g_hBtnSend, TRUE);
}

VOID OnRecv(SOCKET s)
{
    PSOCKETOBJ pSocketObj = FindSocketObj(s);
    CHAR szBuf[BUF_SIZE] = { 0 };
    int nRet = SOCKET_ERROR;

    nRet = recv(pSocketObj->m_socket, szBuf, BUF_SIZE, 0);
    if (nRet > 0)
    {
        CHAR szMsg[BUF_SIZE] = { 0 };
        wsprintf(szMsg, "�ͻ���[%s:%d]˵��%s", pSocketObj->m_szIP, pSocketObj->m_usPort, szBuf);
        SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szMsg);

        // ���յ������ݷַ���ÿһ���ͻ���
        PSOCKETOBJ p = g_pSocketObjHeader;
        while (p != NULL)
        {
            if (p->m_socket != pSocketObj->m_socket)
                send(p->m_socket, szMsg, strlen(szMsg), 0);

            p = p->m_pNext;
        }
    }
}

VOID OnClose(SOCKET s)
{
    PSOCKETOBJ pSocketObj = FindSocketObj(s);

    CHAR szBuf[BUF_SIZE] = { 0 };
    wsprintf(szBuf, "�ͻ���[%s:%d] ���˳���", pSocketObj->m_szIP, pSocketObj->m_usPort);
    SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);
    FreeSocketObj(pSocketObj);

    // ���û�пͻ��������ˣ����÷��Ͱ�ť
    if (g_nTotalClient == 0)
        EnableWindow(g_hBtnSend, FALSE);
}