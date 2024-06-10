#include <winsock2.h>           // Winsock2头文件
#include <ws2tcpip.h>           // inet_pton / inet_ntop需要使用这个头文件
#include "resource.h"
#include "SOCKETOBJ.h"

#pragma comment(lib, "Ws2_32")  // Winsock2导入库

// 常量定义
const int BUF_SIZE = 4096;

// 全局变量
HWND g_hwnd;            // 窗口句柄
HWND g_hListContent;    // 聊天内容列表框窗口句柄
HWND g_hEditMsg;        // 消息输入框窗口句柄
HWND g_hBtnSend;        // 发送按钮窗口句柄

SOCKET g_socketListen = INVALID_SOCKET;   // 监听套接字句柄

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 对话框初始化
VOID OnInit(HWND hwndDlg);
// 按下启动服务按钮
VOID OnStart();
// 按下发送按钮
VOID OnSend();
// 循环等待客户端连接请求的线程函数
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
            // 关闭套接字，释放WinSock库
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
    // 1、初始化WinSock库
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        MessageBox(g_hwnd, TEXT("初始化WinSock库失败！"), TEXT("WSAStartup Error"), MB_OK);
        return;
    }

    // 初始化临界区对象，用于同步对套接字对象的访问
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
        MessageBox(g_hwnd, TEXT("将监听套接字与指定的IP地址、端口号捆绑失败！"),
            TEXT("bind Error"), MB_OK);
        closesocket(g_socketListen);
        WSACleanup();
        return;
    }

    // 4、使监听套节字进入监听(等待被连接)状态
    if (listen(g_socketListen, SOMAXCONN) == SOCKET_ERROR)
    {
        MessageBox(g_hwnd, TEXT("使监听套节字进入监听(等待被连接)状态失败！"),
            TEXT("listen Error"), MB_OK);
        closesocket(g_socketListen);
        WSACleanup();
        return;
    }
    // 服务器监听中...
    MessageBox(g_hwnd, TEXT("服务器监听中..."), TEXT("服务启动成功"), MB_OK);
    EnableWindow(GetDlgItem(g_hwnd, IDC_BTN_START), FALSE);

    // 5、创建一个新线程循环等待连接请求
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
    wsprintf(szMsg, "服务器说：%s", szBuf);
    SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szMsg);
    SetWindowText(g_hEditMsg, "");

    // 向每一个客户端发送数据
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
    SOCKET socketAccept = INVALID_SOCKET;   // 通信套接字句柄
    sockaddr_in sockAddrClient;
    int nAddrlen = sizeof(sockaddr_in);
    CHAR szBuf[BUF_SIZE] = { 0 };
    CHAR szMsg[BUF_SIZE] = { 0 };
    fd_set fd;
    PSOCKETOBJ pSocketObj, p;
    int nRet = SOCKET_ERROR;

    // (1)、初始化一个可读性套节字集合readfds，添加监听套节字句柄到这个集合
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(g_socketListen, &readfds);

    while (TRUE)
    {
        // 复制一份套接字集合
        fd = readfds;
        // (2)、把timeout参数设置为NULL，select调用将无限期阻塞
        nRet = select(0, &fd, NULL, NULL, NULL);
        if (nRet <= 0)
            continue;

        // (3)、将原readfds集合与经过select函数处理过的fd集合比较
        for (UINT i = 0; i < readfds.fd_count; i++)
        {
            if (!FD_ISSET(readfds.fd_array[i], &fd))
                continue;

            if (readfds.fd_array[i] == g_socketListen)  // ①-是监听套接字，可读性表示需接受新连接
            {
                if (readfds.fd_count < FD_SETSIZE)
                {
                    socketAccept = accept(g_socketListen, (sockaddr*)&sockAddrClient, &nAddrlen);
                    if (socketAccept == INVALID_SOCKET)
                        continue;
                    // (4)、把通信套接字添加到原readfds集合
                    FD_SET(socketAccept, &readfds);

                    // 6、接受客户的连接请求成功
                    ZeroMemory(szBuf, BUF_SIZE);
                    // 创建一个套接字对象，保存客户端IP地址、端口号
                    pSocketObj = CreateSocketObj(socketAccept);
                    inet_ntop(AF_INET, &sockAddrClient.sin_addr.S_un.S_addr,
                        pSocketObj->m_szIP, _countof(pSocketObj->m_szIP));
                    pSocketObj->m_usPort = ntohs(sockAddrClient.sin_port);
                    wsprintf(szBuf, "客户端[%s:%d] 已连接！",
                        pSocketObj->m_szIP, pSocketObj->m_usPort);
                    SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);
                    EnableWindow(g_hBtnSend, TRUE);
                }
                else
                {
                    MessageBox(g_hwnd, TEXT("客户端连接数太多！"), TEXT("accept Error"), MB_OK);
                    continue;
                }
            }
            else
            {
                pSocketObj = FindSocketObj(readfds.fd_array[i]);

                ZeroMemory(szBuf, BUF_SIZE);
                nRet = SOCKET_ERROR;
                nRet = recv(pSocketObj->m_socket, szBuf, BUF_SIZE, 0);
                if (nRet > 0)                           // ②-是通信套接字，接收到客户端数据
                {
                    ZeroMemory(szMsg, BUF_SIZE);
                    wsprintf(szMsg, "客户端[%s:%d]说：%s",
                        pSocketObj->m_szIP, pSocketObj->m_usPort, szBuf);
                    SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szMsg);

                    // 把收到的数据分发到每一个客户端
                    p = g_pSocketObjHeader;
                    while (p != NULL)
                    {
                        if (p->m_socket != pSocketObj->m_socket)
                            send(p->m_socket, szMsg, strlen(szMsg), 0);

                        p = p->m_pNext;
                    }
                }
                else                                    // ③-是通信套接字，连接已关闭
                {
                    ZeroMemory(szMsg, BUF_SIZE);
                    wsprintf(szMsg, "客户端[%s:%d] 已退出！",
                        pSocketObj->m_szIP, pSocketObj->m_usPort);
                    SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szMsg);
                    FD_CLR(readfds.fd_array[i], &readfds);
                    FreeSocketObj(pSocketObj);

                    // 如果没有客户端在线了，禁用发送按钮
                    if (g_nTotalClient == 0)
                        EnableWindow(g_hBtnSend, FALSE);
                }
            }
        }   // for循环
    }       // while循环
}