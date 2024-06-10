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

SOCKET g_socketListen = INVALID_SOCKET;   // �����׽��־��

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// �Ի����ʼ��
VOID OnInit(HWND hwndDlg);
// ������������ť
VOID OnStart();
// ���·��Ͱ�ť
VOID OnSend();
// ѭ���ȴ��ͻ�������������̺߳���
DWORD WINAPI AcceptProc(LPVOID lpParam);

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

    return;
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
    HANDLE hThread = NULL;
    if ((hThread = CreateThread(NULL, 0, AcceptProc, NULL, 0, NULL)) != NULL)
        CloseHandle(hThread);

    return;
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

    return;
}

DWORD WINAPI AcceptProc(LPVOID lpParam)
{
    SOCKET socketAccept = INVALID_SOCKET;   // ͨ���׽��־��
    sockaddr_in sockAddrClient;
    int nAddrlen = sizeof(sockaddr_in);
    CHAR szBuf[BUF_SIZE] = { 0 };
    CHAR szMsg[BUF_SIZE] = { 0 };
    fd_set fd;
    PSOCKETOBJ pSocketObj, p;
    int nRet = SOCKET_ERROR;

    // (1)����ʼ��һ���ɶ����׽��ּ���readfds����Ӽ����׽��־�����������
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(g_socketListen, &readfds);

    while (TRUE)
    {
        // ����һ���׽��ּ���
        fd = readfds;
        // (2)����timeout��������ΪNULL��select���ý�����������
        nRet = select(0, &fd, NULL, NULL, NULL);
        if (nRet <= 0)
            continue;

        // (3)����ԭreadfds�����뾭��select�����������fd���ϱȽ�
        for (UINT i = 0; i < readfds.fd_count; i++)
        {
            if (!FD_ISSET(readfds.fd_array[i], &fd))
                continue;

            if (readfds.fd_array[i] == g_socketListen)  // ��-�Ǽ����׽��֣��ɶ��Ա�ʾ�����������
            {
                if (readfds.fd_count < FD_SETSIZE)
                {
                    socketAccept = accept(g_socketListen, (sockaddr*)&sockAddrClient, &nAddrlen);
                    if (socketAccept == INVALID_SOCKET)
                        continue;
                    // (4)����ͨ���׽�����ӵ�ԭreadfds����
                    FD_SET(socketAccept, &readfds);

                    // 6�����ܿͻ�����������ɹ�
                    ZeroMemory(szBuf, BUF_SIZE);
                    // ����һ���׽��ֶ��󣬱���ͻ���IP��ַ���˿ں�
                    pSocketObj = CreateSocketObj(socketAccept);
                    inet_ntop(AF_INET, &sockAddrClient.sin_addr.S_un.S_addr,
                        pSocketObj->m_szIP, _countof(pSocketObj->m_szIP));
                    pSocketObj->m_usPort = ntohs(sockAddrClient.sin_port);
                    wsprintf(szBuf, "�ͻ���[%s:%d] �����ӣ�",
                        pSocketObj->m_szIP, pSocketObj->m_usPort);
                    SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);
                    EnableWindow(g_hBtnSend, TRUE);
                }
                else
                {
                    MessageBox(g_hwnd, TEXT("�ͻ���������̫�࣡"), TEXT("accept Error"), MB_OK);
                    continue;
                }
            }
            else
            {
                pSocketObj = FindSocketObj(readfds.fd_array[i]);

                ZeroMemory(szBuf, BUF_SIZE);
                nRet = SOCKET_ERROR;
                nRet = recv(pSocketObj->m_socket, szBuf, BUF_SIZE, 0);
                if (nRet > 0)                           // ��-��ͨ���׽��֣����յ��ͻ�������
                {
                    ZeroMemory(szMsg, BUF_SIZE);
                    wsprintf(szMsg, "�ͻ���[%s:%d]˵��%s",
                        pSocketObj->m_szIP, pSocketObj->m_usPort, szBuf);
                    SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szMsg);

                    // ���յ������ݷַ���ÿһ���ͻ���
                    p = g_pSocketObjHeader;
                    while (p != NULL)
                    {
                        if (p->m_socket != pSocketObj->m_socket)
                            send(p->m_socket, szMsg, strlen(szMsg), 0);

                        p = p->m_pNext;
                    }
                }
                else                                    // ��-��ͨ���׽��֣������ѹر�
                {
                    ZeroMemory(szMsg, BUF_SIZE);
                    wsprintf(szMsg, "�ͻ���[%s:%d] ���˳���",
                        pSocketObj->m_szIP, pSocketObj->m_usPort);
                    SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szMsg);
                    FD_CLR(readfds.fd_array[i], &readfds);
                    FreeSocketObj(pSocketObj);

                    // ���û�пͻ��������ˣ����÷��Ͱ�ť
                    if (g_nTotalClient == 0)
                        EnableWindow(g_hBtnSend, FALSE);
                }
            }
        }   // forѭ��
    }       // whileѭ��
}