#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>           // Winsock2头文件
#include <ws2tcpip.h>           // inet_pton / inet_ntop需要使用这个头文件
#include <Mswsock.h>
#include <strsafe.h>
#include "resource.h"
#include "SOCKETOBJ.h"
#include "PERIODATA.h"

#pragma comment(lib, "Ws2_32")  // Winsock2导入库
#pragma comment(lib, "Mswsock") // Mswsock导入库

// 全局变量
HWND g_hwnd;            // 窗口句柄
HWND g_hListContent;    // 聊天内容列表框窗口句柄
HWND g_hEditMsg;        // 消息输入框窗口句柄
HWND g_hBtnSend;        // 发送按钮窗口句柄

SOCKET g_socketListen = INVALID_SOCKET; // 监听套接字句柄

WSAEVENT g_eventArray[WSA_MAXIMUM_WAIT_EVENTS]; // 所有事件对象句柄数组
int g_nTotalEvent;                              // 所有事件对象句柄总数

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 对话框初始化
VOID OnInit(HWND hwndDlg);
// 按下启动服务按钮
VOID OnStart();
// 在所有事件对象上循环等待网络事件
DWORD WINAPI WaitProc(LPVOID lpParam);
// 投递接受连接I/O请求
BOOL PostAccept();
// 投递发送数据I/O请求
BOOL PostSend(PSOCKETOBJ pSocketObj, LPTSTR pStr, int nLen);
// 投递接收数据I/O请求
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
            GetWindowText(g_hEditMsg, szBuf, BUF_SIZE);
            wsprintf(szMsg, "服务器说：%s", szBuf);
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szMsg);
            SetWindowText(g_hEditMsg, "");

            // 向每一个客户端发送数据
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
    g_socketListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
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

    // 在所有事件对象上循环等待网络事件，本程序只用了一个线程
    HANDLE hThread = NULL;
    if ((hThread = CreateThread(NULL, 0, WaitProc, NULL, 0, NULL)) != NULL)
        CloseHandle(hThread);

    // 在此先投递几个接受连接I/O请求
    for (int i = 0; i < 2; i++)
        PostAccept();
}

DWORD WINAPI WaitProc(LPVOID lpParam)
{
    sockaddr_in* pRemoteSockaddr;
    sockaddr_in* pLocalSockaddr;
    int nAddrlen = sizeof(sockaddr_in);
    int nIndex;                             // WSAWaitForMultipleEvents返回值
    PPERIODATA pPerIOData = NULL;           // 自定义重叠结构指针
    PSOCKETOBJ pSocketObj = NULL;           // 套接字对象结构指针
    PSOCKETOBJ pSocketObjAccept = NULL;     // 套接字对象结构指针，接受连接成功以后创建的
    DWORD dwTransfer;                       // WSAGetOverlappedResult函数参数
    DWORD dwFlags = 0;                      // WSAGetOverlappedResult函数参数
    BOOL bRet = FALSE;
    CHAR szBuf[BUF_SIZE] = { 0 };

    while (TRUE)
    {
        // 在所有事件对象上等待，有任何一个事件对象触发，函数就返回
        nIndex = WSAWaitForMultipleEvents(g_nTotalEvent, g_eventArray, FALSE, 1000, FALSE);
        if (nIndex == WSA_WAIT_TIMEOUT || nIndex == WSA_WAIT_FAILED)
            continue;

        nIndex = nIndex - WSA_WAIT_EVENT_0;
        WSAResetEvent(g_eventArray[nIndex]);

        // 获取指定套接字上重叠I/O操作的结果
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
                wsprintf(szBuf, "客户端[%s:%d] 已退出！", pSocketObj->m_szIP, pSocketObj->m_usPort);
                SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);

                FreeSocketObj(pSocketObj);
            }

            // 释放自定义重叠结构
            FreePerIOData(pPerIOData);
            // 更新事件对象数组
            for (int j = nIndex; j < g_nTotalEvent - 1; j++)
                g_eventArray[j] = g_eventArray[j + 1];
            g_nTotalEvent--;

            // 如果没有客户端在线了，禁用发送按钮
            if (g_nTotalClient == 0)
                EnableWindow(g_hBtnSend, FALSE);

            continue;
        }

        // 处理已成功完成的I/O请求
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
            wsprintf(szBuf, "客户端[%s:%d] 已连接！", pSocketObjAccept->m_szIP,
                pSocketObjAccept->m_usPort);
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);
            EnableWindow(g_hBtnSend, TRUE);

            // 释放自定义重叠结构
            FreePerIOData(pPerIOData);
            // 更新事件对象数组
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
                wsprintf(szBuf, "客户端[%s:%d]说：%s", pSocketObj->m_szIP,
                    pSocketObj->m_usPort, pPerIOData->m_szBuffer);
                SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);

                // 把收到的数据分发到每一个客户端
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
                wsprintf(szBuf, "客户端[%s:%d] 已退出！", pSocketObj->m_szIP, pSocketObj->m_usPort);
                SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);

                FreeSocketObj(pSocketObj);

                // 如果没有客户端在线了，禁用发送按钮
                if (g_nTotalClient == 0)
                    EnableWindow(g_hBtnSend, FALSE);
            }

            // 释放自定义重叠结构
            FreePerIOData(pPerIOData);
            // 更新事件对象数组
            for (int j = nIndex; j < g_nTotalEvent - 1; j++)
                g_eventArray[j] = g_eventArray[j + 1];
            g_nTotalEvent--;
            break;

        case IO_WRITE:
            if (dwTransfer <= 0)
            {
                ZeroMemory(szBuf, BUF_SIZE);
                wsprintf(szBuf, "客户端[%s:%d] 已退出！", pSocketObj->m_szIP, pSocketObj->m_usPort);
                SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);

                FreeSocketObj(pSocketObj);

                // 如果没有客户端在线了，禁用发送按钮
                if (g_nTotalClient == 0)
                    EnableWindow(g_hBtnSend, FALSE);
            }

            // 释放自定义重叠结构
            FreePerIOData(pPerIOData);
            // 更新事件对象数组
            for (int j = nIndex; j < g_nTotalEvent - 1; j++)
                g_eventArray[j] = g_eventArray[j + 1];
            g_nTotalEvent--;
            break;
        }
    }
}

// 投递接受连接I/O请求
BOOL PostAccept()
{
    SOCKET socketAccept = INVALID_SOCKET;   // 通信套接字句柄
    BOOL bRet = FALSE;

    socketAccept = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (socketAccept == INVALID_SOCKET)
        return FALSE;

    // 为接受连接创建一个自定义重叠结构
    PPERIODATA pPerIOData = CreatePerIOData(socketAccept);
    pPerIOData->m_ioOperation = IO_ACCEPT;

    // 事件对象数组
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

// 投递发送数据I/O请求
BOOL PostSend(PSOCKETOBJ pSocketObj, LPTSTR pStr, int nLen)
{
    DWORD dwFlags = 0;

    // 为发送数据创建一个自定义重叠结构
    PPERIODATA pPerIOData = CreatePerIOData(pSocketObj->m_socket);
    ZeroMemory(pPerIOData->m_szBuffer, BUF_SIZE);
    StringCchCopy(pPerIOData->m_szBuffer, BUF_SIZE, pStr);
    pPerIOData->m_wsaBuf.buf = pPerIOData->m_szBuffer;
    pPerIOData->m_wsaBuf.len = nLen;

    pPerIOData->m_ioOperation = IO_WRITE;

    // 事件对象数组
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

// 投递接收数据I/O请求
BOOL PostRecv(PSOCKETOBJ pSocketObj)
{
    DWORD dwFlags = 0;

    // 为接收数据创建一个自定义重叠结构
    PPERIODATA pPerIOData = CreatePerIOData(pSocketObj->m_socket);
    ZeroMemory(pPerIOData->m_szBuffer, BUF_SIZE);
    pPerIOData->m_wsaBuf.buf = pPerIOData->m_szBuffer;
    pPerIOData->m_wsaBuf.len = BUF_SIZE;

    pPerIOData->m_ioOperation = IO_READ;

    // 事件对象数组
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