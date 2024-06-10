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

SOCKET g_socketListen = INVALID_SOCKET;         // 监听套接字句柄

WSAEVENT g_eventArray[WSA_MAXIMUM_WAIT_EVENTS]; // 所有事件对象句柄数组
SOCKET g_socketArray[WSA_MAXIMUM_WAIT_EVENTS];  // 所有套接字句柄数组
int g_nTotalEvent;                              // 所有事件对象句柄总数

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 对话框初始化
VOID OnInit(HWND hwndDlg);
// 按下启动服务按钮
VOID OnStart();
// 按下发送按钮
VOID OnSend();
// 在所有事件对象上循环等待网络事件
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

    // 创建事件对象，为监听套接字把该事件对象与一些网络事件相关联
    WSAEVENT hEvent = WSACreateEvent();
    WSAEventSelect(g_socketListen, hEvent, FD_ACCEPT/* | FD_CLOSE*/);
    // 把事件对象和监听套接字放入相关数组中
    g_eventArray[g_nTotalEvent] = hEvent;
    g_socketArray[g_nTotalEvent] = g_socketListen;
    g_nTotalEvent++;

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

    // 创建一个新线程在所有事件对象上循环等待网络事件
    HANDLE hThread = NULL;
    if ((hThread = CreateThread(NULL, 0, WaitProc, NULL, 0, NULL)) != NULL)
        CloseHandle(hThread);
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
}

DWORD WINAPI WaitProc(LPVOID lpParam)
{
    SOCKET socketAccept = INVALID_SOCKET;   // 通信套接字句柄
    sockaddr_in sockAddrClient;
    int nAddrlen = sizeof(sockaddr_in);
    int nIndex;                             // WSAWaitForMultipleEvents返回值
    WSANETWORKEVENTS networkEvents;         // WSAEnumNetworkEvents函数用的结构
    WSAEVENT hEvent = NULL;
    PSOCKETOBJ pSocketObj;
    int nRet = SOCKET_ERROR;                // I/O操作返回值
    CHAR szBuf[BUF_SIZE] = { 0 };
    CHAR szMsg[BUF_SIZE] = { 0 };

    while (TRUE)
    {
        // 在所有事件对象上等待，有任何一个事件对象触发，函数就返回
        nIndex = WSAWaitForMultipleEvents(g_nTotalEvent, g_eventArray, FALSE, WSA_INFINITE, FALSE);
        nIndex = nIndex - WSA_WAIT_EVENT_0;

        // 查看触发的事件对象对应的套接字发生了哪些网络事件
        WSAEnumNetworkEvents(g_socketArray[nIndex], g_eventArray[nIndex], &networkEvents);
        // 接受客户端连接FD_ACCEPT网络事件
        if (networkEvents.lNetworkEvents & FD_ACCEPT)
        {
            if (g_nTotalEvent >= WSA_MAXIMUM_WAIT_EVENTS)
            {
                MessageBox(g_hwnd, TEXT("客户端连接数太多！"), TEXT("accept Error"), MB_OK);
                continue;
            }

            socketAccept = accept(g_socketListen, (sockaddr*)&sockAddrClient, &nAddrlen);
            if (socketAccept == INVALID_SOCKET)
            {
                Sleep(100);
                continue;
            }

            // 6、接受客户的连接请求成功
            // 创建事件对象，为通信套接字把该事件对象与一些网络事件相关联
            hEvent = WSACreateEvent();
            WSAEventSelect(socketAccept, hEvent, FD_READ | FD_WRITE | FD_CLOSE);
            // 把事件对象和通信套接字放入相关数组中
            g_eventArray[g_nTotalEvent] = hEvent;
            g_socketArray[g_nTotalEvent] = socketAccept;
            g_nTotalEvent++;

            ZeroMemory(szBuf, BUF_SIZE);
            // 创建一个套接字对象，保存客户端IP地址、端口号
            PSOCKETOBJ pSocketObj = CreateSocketObj(socketAccept);
            inet_ntop(AF_INET, &sockAddrClient.sin_addr.S_un.S_addr,
                pSocketObj->m_szIP, _countof(pSocketObj->m_szIP));
            pSocketObj->m_usPort = ntohs(sockAddrClient.sin_port);
            wsprintf(szBuf, "客户端[%s:%d] 已连接！",
                pSocketObj->m_szIP, pSocketObj->m_usPort);
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);
            EnableWindow(g_hBtnSend, TRUE);
        }
        // 接收客户端数据FD_READ网络事件
        else if (networkEvents.lNetworkEvents & FD_READ)
        {
            pSocketObj = FindSocketObj(g_socketArray[nIndex]);
            ZeroMemory(szBuf, BUF_SIZE);
            nRet = SOCKET_ERROR;
            nRet = recv(g_socketArray[nIndex], szBuf, BUF_SIZE, 0);
            if (nRet > 0)   // 接收到客户端数据
            {
                ZeroMemory(szMsg, BUF_SIZE);
                wsprintf(szMsg, "客户端[%s:%d]说：%s",
                    pSocketObj->m_szIP, pSocketObj->m_usPort, szBuf);
                SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szMsg);

                // 把收到的数据分发到每一个客户端
                PSOCKETOBJ p = g_pSocketObjHeader;
                while (p != NULL)
                {
                    if (p->m_socket != g_socketArray[nIndex])
                        send(p->m_socket, szMsg, strlen(szMsg), 0);

                    p = p->m_pNext;
                }
            }
        }
        // 发送数据FD_WRITE网络事件，本例不需要处理，因为是按下发送按钮才发送
        else if (networkEvents.lNetworkEvents & FD_WRITE)
        {
        }
        // 客户端连接关闭FD_CLOSE网络事件
        else if (networkEvents.lNetworkEvents & FD_CLOSE)
        {
            ZeroMemory(szMsg, BUF_SIZE);
            pSocketObj = FindSocketObj(g_socketArray[nIndex]);
            wsprintf(szMsg, "客户端[%s:%d] 已退出！", pSocketObj->m_szIP, pSocketObj->m_usPort);
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szMsg);
            FreeSocketObj(pSocketObj);

            // 更新事件对象、套接字数组
            for (int j = nIndex; j < g_nTotalEvent - 1; j++)
            {
                g_eventArray[j] = g_eventArray[j + 1];
                g_socketArray[j] = g_socketArray[j + 1];
            }
            g_nTotalEvent--;

            // 如果没有客户端在线了，禁用发送按钮
            if (g_nTotalClient == 0)
                EnableWindow(g_hBtnSend, FALSE);
        }
    }
}