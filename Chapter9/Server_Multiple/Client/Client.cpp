#include <winsock2.h>           // Winsock2头文件
#include <ws2tcpip.h>           // inet_pton / inet_ntop需要使用这个头文件
#include "resource.h"

#pragma comment(lib, "Ws2_32")  // Winsock2导入库

// 常量定义
const int BUF_SIZE = 4096;

// 全局变量
HWND g_hwnd;            // 窗口句柄
HWND g_hListContent;    // 聊天内容列表框窗口句柄
HWND g_hEditMsg;        // 消息输入框窗口句柄
HWND g_hBtnSend;        // 发送按钮窗口句柄
HWND g_hBtnConnect;     // 连接按钮窗口句柄

SOCKET g_socketClient = INVALID_SOCKET; // 通信套接字句柄

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 对话框初始化
VOID OnInit(HWND hwndDlg);
// 按下连接按钮
VOID OnConnect();
// 按下发送按钮
VOID OnSend();
// 客户端接收数据的线程函数
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

// 按下连接按钮
VOID OnConnect()
{
    WSADATA wsa = { 0 };
    sockaddr_in sockAddrServer;
    HANDLE hThread = NULL;

    // 1、初始化WinSock库
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        MessageBox(g_hwnd, TEXT("初始化WinSock库失败！"), TEXT("WSAStartup Error"), MB_OK);
        return;
    }

    // 2、创建与服务器的通信套接字
    g_socketClient = socket(AF_INET, SOCK_STREAM, 0);
    if (g_socketClient == INVALID_SOCKET)
    {
        MessageBox(g_hwnd, TEXT("创建与服务器的通信套接字失败！"), TEXT("socket Error"), MB_OK);
        WSACleanup();
        return;
    }

    // 3、与服务器建立连接
    sockAddrServer.sin_family = AF_INET;
    sockAddrServer.sin_port = htons(8000);
    inet_pton(AF_INET, "127.0.0.1", &sockAddrServer.sin_addr.S_un.S_addr);
    if (connect(g_socketClient, (sockaddr*)&sockAddrServer, sizeof(sockAddrServer)) == SOCKET_ERROR)
    {
        MessageBox(g_hwnd, TEXT("与服务器建立连接失败！"), TEXT("connect Error"), MB_OK);
        closesocket(g_socketClient);
        WSACleanup();
        return;
    }

    // 4、建立连接成功，收发数据
    SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)"已连接到服务器！");
    EnableWindow(g_hBtnConnect, FALSE);
    EnableWindow(g_hBtnSend, TRUE);

    // 创建线程，接收服务器数据
    if ((hThread = CreateThread(NULL, 0, RecvProc, NULL, 0, NULL)) != NULL)
        CloseHandle(hThread);

    return;
}

// 按下发送按钮
VOID OnSend()
{
    CHAR szBuf[BUF_SIZE] = { 0 };
    CHAR szMsg[BUF_SIZE] = { 0 };

    GetWindowText(g_hEditMsg, szBuf, BUF_SIZE);
    wsprintf(szMsg, "我说：%s", szBuf);
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
        if (nRet > 0)   // 收到服务器数据
        {
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);
        }
        else            // 与服务器连接已关闭
        {
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)"与服务器连接已关闭！");
            EnableWindow(g_hBtnConnect, TRUE);
            EnableWindow(g_hBtnSend, FALSE);
            closesocket(g_socketClient);
            WSACleanup();

            return 0;
        }
    }

    return 0;
}