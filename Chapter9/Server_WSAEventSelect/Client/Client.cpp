#include <winsock2.h>           // Winsock2ͷ�ļ�
#include <ws2tcpip.h>           // inet_pton / inet_ntop��Ҫʹ�����ͷ�ļ�
#include "resource.h"

#pragma comment(lib, "Ws2_32")  // Winsock2�����

// ��������
const int BUF_SIZE = 4096;

// ȫ�ֱ���
HWND g_hwnd;            // ���ھ��
HWND g_hListContent;    // ���������б�򴰿ھ��
HWND g_hEditMsg;        // ��Ϣ����򴰿ھ��
HWND g_hBtnSend;        // ���Ͱ�ť���ھ��
HWND g_hBtnConnect;     // ���Ӱ�ť���ھ��

SOCKET g_socketClient = INVALID_SOCKET; // ͨ���׽��־��

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// �Ի����ʼ��
VOID OnInit(HWND hwndDlg);
// �������Ӱ�ť
VOID OnConnect();
// ���·��Ͱ�ť
VOID OnSend();
// �ͻ��˽������ݵ��̺߳���
DWORD WINAPI RecvProc(LPVOID lpParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CLIENT_DIALOG), NULL, DialogProc, NULL);
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
            if (g_socketClient != INVALID_SOCKET)
                closesocket(g_socketClient);
            WSACleanup();
            EndDialog(hwndDlg, IDCANCEL);
            break;

        case IDC_BTN_CONNECT:
            OnConnect();
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
    g_hBtnConnect = GetDlgItem(hwndDlg, IDC_BTN_CONNECT);

    EnableWindow(g_hBtnSend, FALSE);

    return;
}

// �������Ӱ�ť
VOID OnConnect()
{
    WSADATA wsa = { 0 };
    sockaddr_in sockAddrServer;
    HANDLE hThread = NULL;

    // 1����ʼ��WinSock��
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        MessageBox(g_hwnd, TEXT("��ʼ��WinSock��ʧ�ܣ�"), TEXT("WSAStartup Error"), MB_OK);
        return;
    }

    // 2���������������ͨ���׽���
    g_socketClient = socket(AF_INET, SOCK_STREAM, 0);
    if (g_socketClient == INVALID_SOCKET)
    {
        MessageBox(g_hwnd, TEXT("�������������ͨ���׽���ʧ�ܣ�"), TEXT("socket Error"), MB_OK);
        WSACleanup();
        return;
    }

    // 3�����������������
    sockAddrServer.sin_family = AF_INET;
    sockAddrServer.sin_port = htons(8000);
    inet_pton(AF_INET, "127.0.0.1", &sockAddrServer.sin_addr.S_un.S_addr);
    if (connect(g_socketClient, (sockaddr*)&sockAddrServer, sizeof(sockAddrServer)) == SOCKET_ERROR)
    {
        MessageBox(g_hwnd, TEXT("���������������ʧ�ܣ�"), TEXT("connect Error"), MB_OK);
        closesocket(g_socketClient);
        WSACleanup();
        return;
    }

    // 4���������ӳɹ����շ�����
    SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)"�����ӵ���������");
    EnableWindow(g_hBtnConnect, FALSE);
    EnableWindow(g_hBtnSend, TRUE);

    // �����̣߳����շ���������
    if ((hThread = CreateThread(NULL, 0, RecvProc, NULL, 0, NULL)) != NULL)
        CloseHandle(hThread);

    return;
}

// ���·��Ͱ�ť
VOID OnSend()
{
    CHAR szBuf[BUF_SIZE] = { 0 };
    CHAR szMsg[BUF_SIZE] = { 0 };

    GetWindowText(g_hEditMsg, szBuf, BUF_SIZE);
    wsprintf(szMsg, "��˵��%s", szBuf);
    SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szMsg);

    send(g_socketClient, szBuf, strlen(szBuf), 0);

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
        nRet = recv(g_socketClient, szBuf, BUF_SIZE, 0);
        if (nRet > 0)   // �յ�����������
        {
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);
        }
        else            // ������������ѹر�
        {
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)"������������ѹرգ�");
            EnableWindow(g_hBtnConnect, TRUE);
            EnableWindow(g_hBtnSend, FALSE);
            closesocket(g_socketClient);
            WSACleanup();

            return 0;
        }
    }

    return 0;
}