#include <winsock2.h>           // Winsock2ͷ�ļ�
#include <ws2tcpip.h>           // inet_pton / inet_ntop��Ҫʹ�����ͷ�ļ�
#include "resource.h"

#pragma comment(lib, "Ws2_32")  // Winsock2�����

// ��������
const int BUF_SIZE = 1024;

// ȫ�ֱ���
HWND g_hwnd;            // ���ھ��
HWND g_hListContent;    // ���������б�򴰿ھ��
HWND g_hEditMsg;        // ��Ϣ����򴰿ھ��
HWND g_hBtnSend;        // ���Ͱ�ť���ھ��

SOCKET g_socketListen = INVALID_SOCKET;   // �����׽��־��
SOCKET g_socketAccept = INVALID_SOCKET;   // ͨ���׽��־��

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// �Ի����ʼ��
VOID OnInit(HWND hwndDlg);
// ������������ť
VOID OnStart();
// ���·��Ͱ�ť
VOID OnSend();
// ���������������̺߳���
DWORD WINAPI RecvProc(LPVOID lpParam);

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
            if (g_socketAccept != INVALID_SOCKET)
                closesocket(g_socketAccept);
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
    g_hwnd = hwndDlg;
    g_hListContent = GetDlgItem(hwndDlg, IDC_LIST_CONTENT);
    g_hEditMsg = GetDlgItem(hwndDlg, IDC_EDIT_MSG);
    g_hBtnSend = GetDlgItem(hwndDlg, IDC_BTN_SEND);

    EnableWindow(g_hBtnSend, FALSE);

    return;
}

VOID OnStart()
{
    WSADATA wsa = { 0 };

    // 1����ʼ��WinSock��
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        MessageBox(g_hwnd, TEXT("��ʼ��WinSock��ʧ�ܣ�"), TEXT("WSAStartup Error"), MB_OK);
        return;
    }

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
        MessageBox(g_hwnd, TEXT("�������׽�����ָ����IP��ַ���˿ں�����ʧ�ܣ�"), TEXT("bind Error"), MB_OK);
        closesocket(g_socketListen);
        WSACleanup();
        return;
    }

    // 4��ʹ�����׽��ֽ������(�ȴ�������)״̬
    if (listen(g_socketListen, 1) == SOCKET_ERROR)
    {
        MessageBox(g_hwnd, TEXT("ʹ�����׽��ֽ������(�ȴ�������)״̬ʧ�ܣ�"), TEXT("listen Error"), MB_OK);
        closesocket(g_socketListen);
        WSACleanup();
        return;
    }
    // ������������...
    MessageBox(g_hwnd, TEXT("������������..."), TEXT("���������ɹ�"), MB_OK);
    EnableWindow(GetDlgItem(g_hwnd, IDC_BTN_START), FALSE);

    // 5���ȴ��������󣬷������ڷ������ͻ���ͨ�ŵ��׽��־��
    sockaddr_in sockAddrClient;             // ����accept���ؿͻ��˵�IP��ַ���˿ں�
    int nAddrlen = sizeof(sockaddr_in);
    g_socketAccept = accept(g_socketListen, (sockaddr*)&sockAddrClient, &nAddrlen);
    if (g_socketAccept == INVALID_SOCKET)
    {
        MessageBox(g_hwnd, TEXT("������������ʧ�ܣ�"), TEXT("accept Error"), MB_OK);
        closesocket(g_socketListen);
        WSACleanup();
        return;
    }

    // 6�����ܿͻ�����������ɹ����շ�����
    CHAR szBuf[BUF_SIZE] = { 0 };
    CHAR szIP[24] = { 0 };
    inet_ntop(AF_INET, &sockAddrClient.sin_addr.S_un.S_addr, szIP, _countof(szIP));
    wsprintf(szBuf, "�ͻ���[%s:%d]�����ӣ�", szIP, ntohs(sockAddrClient.sin_port));
    SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);

    EnableWindow(g_hBtnSend, TRUE);
    // �����̣߳����տͻ�������
    HANDLE hThread = NULL;
    if ((hThread = CreateThread(NULL, 0, RecvProc, NULL, 0, NULL)) != NULL)
        CloseHandle(hThread);

    return;
}

VOID OnSend()
{
    CHAR szBuf[BUF_SIZE] = { 0 };
    CHAR szShow[BUF_SIZE] = { 0 };

    GetWindowText(g_hEditMsg, szBuf, BUF_SIZE);
    wsprintf(szShow, "������˵��%s", szBuf);
    SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szShow);
    send(g_socketAccept, szShow, strlen(szShow), 0);

    SetWindowText(g_hEditMsg, "");

    return;
}

DWORD WINAPI RecvProc(LPVOID lpParam)
{
    CHAR szBuf[BUF_SIZE] = { 0 };
    int nRet = SOCKET_ERROR;

    while (TRUE)
    {
        ZeroMemory(szBuf, BUF_SIZE);
        nRet = recv(g_socketAccept, szBuf, BUF_SIZE, 0);
        if (nRet > 0)
        {
            // �յ��ͻ�������
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);
        }
    }

    return 0;
}