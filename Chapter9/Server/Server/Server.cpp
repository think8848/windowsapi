#include <winsock2.h>           // Winsock2头文件
#include <ws2tcpip.h>           // inet_pton / inet_ntop需要使用这个头文件
#include "resource.h"

#pragma comment(lib, "Ws2_32")  // Winsock2导入库

// 常量定义
const int BUF_SIZE = 1024;

// 全局变量
HWND g_hwnd;            // 窗口句柄
HWND g_hListContent;    // 聊天内容列表框窗口句柄
HWND g_hEditMsg;        // 消息输入框窗口句柄
HWND g_hBtnSend;        // 发送按钮窗口句柄

SOCKET g_socketListen = INVALID_SOCKET;   // 监听套接字句柄
SOCKET g_socketAccept = INVALID_SOCKET;   // 通信套接字句柄

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 对话框初始化
VOID OnInit(HWND hwndDlg);
// 按下启动服务按钮
VOID OnStart();
// 按下发送按钮
VOID OnSend();
// 服务器接收数据线程函数
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
            // 关闭套接字，释放WinSock库
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

    // 1、初始化WinSock库
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        MessageBox(g_hwnd, TEXT("初始化WinSock库失败！"), TEXT("WSAStartup Error"), MB_OK);
        return;
    }

    // 2、创建用于监听所有客户端请求的套接字
    g_socketListen = socket(AF_INET, SOCK_STREAM, 0);
    if (g_socketListen == INVALID_SOCKET)
    {
        MessageBox(g_hwnd, TEXT("创建监听套接字失败！"), TEXT("socket Error"), MB_OK);
        WSACleanup();
        return;
    }

    // 3、将监听套接字与指定的IP地址、端口号捆绑
    sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(8000);
    sockAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    if (bind(g_socketListen, (sockaddr*)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR)
    {
        MessageBox(g_hwnd, TEXT("将监听套接字与指定的IP地址、端口号捆绑失败！"), TEXT("bind Error"), MB_OK);
        closesocket(g_socketListen);
        WSACleanup();
        return;
    }

    // 4、使监听套节字进入监听(等待被连接)状态
    if (listen(g_socketListen, 1) == SOCKET_ERROR)
    {
        MessageBox(g_hwnd, TEXT("使监听套节字进入监听(等待被连接)状态失败！"), TEXT("listen Error"), MB_OK);
        closesocket(g_socketListen);
        WSACleanup();
        return;
    }
    // 服务器监听中...
    MessageBox(g_hwnd, TEXT("服务器监听中..."), TEXT("服务启动成功"), MB_OK);
    EnableWindow(GetDlgItem(g_hwnd, IDC_BTN_START), FALSE);

    // 5、等待连接请求，返回用于服务器客户端通信的套接字句柄
    sockaddr_in sockAddrClient;             // 调用accept返回客户端的IP地址、端口号
    int nAddrlen = sizeof(sockaddr_in);
    g_socketAccept = accept(g_socketListen, (sockaddr*)&sockAddrClient, &nAddrlen);
    if (g_socketAccept == INVALID_SOCKET)
    {
        MessageBox(g_hwnd, TEXT("接受连接请求失败！"), TEXT("accept Error"), MB_OK);
        closesocket(g_socketListen);
        WSACleanup();
        return;
    }

    // 6、接受客户的连接请求成功，收发数据
    CHAR szBuf[BUF_SIZE] = { 0 };
    CHAR szIP[24] = { 0 };
    inet_ntop(AF_INET, &sockAddrClient.sin_addr.S_un.S_addr, szIP, _countof(szIP));
    wsprintf(szBuf, "客户端[%s:%d]已连接！", szIP, ntohs(sockAddrClient.sin_port));
    SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);

    EnableWindow(g_hBtnSend, TRUE);
    // 创建线程，接收客户端数据
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
    wsprintf(szShow, "服务器说：%s", szBuf);
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
            // 收到客户端数据
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);
        }
    }

    return 0;
}