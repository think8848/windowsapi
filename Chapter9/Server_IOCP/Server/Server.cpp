#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>           // Winsock2头文件
#include <ws2tcpip.h>           // inet_pton / inet_ntop需要使用这个头文件
#include <Mswsock.h>
#include <strsafe.h>
#include "resource.h"
#include "SOCKETOBJ.h"

#pragma comment(lib, "Ws2_32")  // Winsock2导入库
#pragma comment(lib, "Mswsock") // Mswsock导入库

// 常量定义
const int BUF_SIZE = 4096;

// I/O操作类型
enum IOOPERATION
{
   IO_ACCEPT, IO_READ, IO_WRITE
};

// OVERLAPPED结构和I/O唯一数据
typedef struct _PERIODATA
{
   OVERLAPPED  m_overlapped;   // 重叠结构
   SOCKET      m_socket;       // 通信套接字句柄
   WSABUF      m_wsaBuf;
   CHAR        m_szBuffer[BUF_SIZE];
   IOOPERATION m_ioOperation;  // 操作类型
}PERIODATA, * PPERIODATA;

// 全局变量
HWND g_hwnd;            // 窗口句柄
HWND g_hListContent;    // 聊天内容列表框窗口句柄
HWND g_hEditMsg;        // 消息输入框窗口句柄
HWND g_hBtnSend;        // 发送按钮窗口句柄

SOCKET g_socketListen;      // 监听套接字句柄
HANDLE g_hCompletionPort;   // 完成端口句柄

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 对话框初始化
VOID OnInit(HWND hwndDlg);
// 按下启动服务按钮
VOID OnStart();
// TrySubmitThreadpoolCallback的回调函数
VOID CALLBACK WorkerThreadProc(PTP_CALLBACK_INSTANCE Instance, PVOID Context);
// 投递接受连接I/O请求
BOOL PostAccept();
// 投递发送数据I/O请求
BOOL PostSend(SOCKET s, LPTSTR pStr, int nLen);
// 投递接收数据I/O请求
BOOL PostRecv(SOCKET s);
// 按下发送按钮
VOID OnSend();
// 获取逻辑处理器和物理处理器个数
ULARGE_INTEGER GetProcessorInformation();

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
         // 关闭完成端口
         if (g_hCompletionPort)
            CloseHandle(g_hCompletionPort);
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
   ULARGE_INTEGER uli = { 0 };

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

   // 5、
   // 创建I/O完成端口，当GetQueuedCompletionStatus函数返回FALSE
   // 并且错误代码为ERROR_ABANDONED_WAIT_0的时候可以确定完成端口已关闭
   g_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

   // 设置线程池中工作线程的回调函数，线程池会决定如何管理工作线程
   TrySubmitThreadpoolCallback(WorkerThreadProc, NULL, NULL);

   // 添加监听套接字节点
   PSOCKETOBJ pSocketObj = CreateSocketObj(g_socketListen);
   // 将监听套接字与完成端口g_hCompletionPort相关联，pSocket作为句柄唯一数据
   CreateIoCompletionPort((HANDLE)g_socketListen, g_hCompletionPort, (ULONG_PTR)pSocketObj, 0);

   // 在此先投递物理处理器 * 2个接受连接I/O请求，GetProcessorInformation是自定义函数
   uli = GetProcessorInformation();
   for (DWORD i = 0; i < uli.HighPart * 2; i++)
      PostAccept();
}

// TrySubmitThreadpoolCallback的回调函数
VOID CALLBACK WorkerThreadProc(PTP_CALLBACK_INSTANCE Instance, PVOID Context)
{
   sockaddr_in* pRemoteSockaddr;
   sockaddr_in* pLocalSockaddr;
   int nAddrlen = sizeof(sockaddr_in);
   PSOCKETOBJ pSocketObj = NULL;   // 返回与套接字相关联的单句柄数据
   PPERIODATA pPerIOData = NULL;   // 返回I/O操作函数指定的OVERLAPPED结构的地址
   DWORD dwTrans;                  // 返回已完成的I/O操作所发送或接收的字节数
   PSOCKETOBJ pSocket;             // 接受连接成功以后，添加一个套接字信息节点
   BOOL bRet;
   PSOCKETOBJ p;
   CHAR szBuf[BUF_SIZE] = { 0 };

   while (TRUE)
   {
      bRet = GetQueuedCompletionStatus(g_hCompletionPort, &dwTrans,
         (PULONG_PTR)&pSocketObj, (LPOVERLAPPED*)&pPerIOData, INFINITE);
      if (!bRet)
      {
         if (GetLastError() == ERROR_ABANDONED_WAIT_0)   // 完成端口已关闭
         {
            break;
         }
         else                                            // 客户端已关闭
         {
            ZeroMemory(szBuf, BUF_SIZE);
            wsprintf(szBuf, "客户端[%s:%d] 已退出！", pSocketObj->m_szIP, pSocketObj->m_usPort);
            SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);

            // 释放句柄唯一数据
            FreeSocketObj(pSocketObj);

            // 释放I/O唯一数据
            delete pPerIOData;

            // 如果没有客户端在线了，禁用发送按钮
            if (g_nTotalClient == 1)                    // 监听套接字占用了一个结构，所以是1
               EnableWindow(g_hBtnSend, FALSE);

            continue;
         }
      }

      // 对已完成的I/O操作进行处理，进行到这里，接受连接或数据收发工作已经完成了
      switch (pPerIOData->m_ioOperation)
      {
      case IO_ACCEPT:
      {
         // 接受连接已成功，创建套接字信息结构，添加一个节点
         pSocket = CreateSocketObj(pPerIOData->m_socket);

         // 将通信套接字与完成端口g_hCompletionPort相关联，pSocket作为句柄唯一数据
         CreateIoCompletionPort((HANDLE)pSocket->m_socket,
            g_hCompletionPort, (ULONG_PTR)pSocket, 0);

         ZeroMemory(szBuf, BUF_SIZE);
         GetAcceptExSockaddrs(pPerIOData->m_szBuffer, 0, sizeof(sockaddr_in) + 16,
            sizeof(sockaddr_in) + 16, (LPSOCKADDR*)&pLocalSockaddr, &nAddrlen,
            (LPSOCKADDR*)&pRemoteSockaddr, &nAddrlen);
         inet_ntop(AF_INET, &pRemoteSockaddr->sin_addr.S_un.S_addr,
            pSocket->m_szIP, _countof(pSocket->m_szIP));
         pSocket->m_usPort = ntohs(pRemoteSockaddr->sin_port);
         wsprintf(szBuf, "客户端[%s:%d] 已连接！", pSocket->m_szIP, pSocket->m_usPort);
         SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);
         EnableWindow(g_hBtnSend, TRUE);

         // 释放I/O唯一数据
         delete pPerIOData;

         // 投递一个接收数据请求
         PostRecv(pSocket->m_socket);

         // 继续投递一个接受连接请求
         PostAccept();
      }
      break;

      case IO_READ:
      {
         ZeroMemory(szBuf, BUF_SIZE);
         wsprintf(szBuf, "客户端[%s:%d]说：%s", pSocketObj->m_szIP,
            pSocketObj->m_usPort, pPerIOData->m_szBuffer);
         SendMessage(g_hListContent, LB_ADDSTRING, 0, (LPARAM)szBuf);

         // 释放I/O唯一数据
         delete pPerIOData;

         // 把收到的数据分发到每一个客户端
         p = g_pSocketObjHeader;
         while (p != NULL)
         {
            if (p->m_socket != g_socketListen && p->m_socket != pSocketObj->m_socket)
               PostSend(p->m_socket, szBuf, strlen(szBuf));

            p = p->m_pNext;
         }

         // 继续投递接收数据请求
         PostRecv(pSocketObj->m_socket);
      }
      break;

      case IO_WRITE:
         // 释放I/O唯一数据
         delete pPerIOData;
         break;
      }
   }
}

// 投递接受连接I/O请求
BOOL PostAccept()
{
   PPERIODATA pPerIOData = new PERIODATA;
   ZeroMemory(&pPerIOData->m_overlapped, sizeof(OVERLAPPED));
   pPerIOData->m_ioOperation = IO_ACCEPT;

   SOCKET socketAccept = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
   pPerIOData->m_socket = socketAccept;

   BOOL bRet = AcceptEx(g_socketListen, socketAccept, pPerIOData->m_szBuffer, 0,
      sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, NULL, (LPOVERLAPPED)pPerIOData);
   if (!bRet)
   {
      if (WSAGetLastError() != WSA_IO_PENDING)
         return FALSE;
   }

   return TRUE;
}

// 投递发送数据I/O请求
BOOL PostSend(SOCKET s, LPTSTR pStr, int nLen)
{
   DWORD dwFlags = 0;
   PPERIODATA pPerIOData = new PERIODATA;
   ZeroMemory(&pPerIOData->m_overlapped, sizeof(OVERLAPPED));
   pPerIOData->m_ioOperation = IO_WRITE;
   ZeroMemory(pPerIOData->m_szBuffer, BUF_SIZE);
   StringCchCopy(pPerIOData->m_szBuffer, BUF_SIZE, pStr);
   pPerIOData->m_wsaBuf.buf = pPerIOData->m_szBuffer;
   pPerIOData->m_wsaBuf.len = BUF_SIZE;

   int nRet = WSASend(s, &pPerIOData->m_wsaBuf, 1, NULL, dwFlags, (LPOVERLAPPED)pPerIOData, NULL);
   if (nRet == SOCKET_ERROR)
   {
      if (WSAGetLastError() != WSA_IO_PENDING)
         return FALSE;
   }

   return TRUE;
}

// 投递接收数据I/O请求
BOOL PostRecv(SOCKET s)
{
   DWORD dwFlags = 0;
   PPERIODATA pPerIOData = new PERIODATA;
   ZeroMemory(&pPerIOData->m_overlapped, sizeof(OVERLAPPED));
   pPerIOData->m_ioOperation = IO_READ;
   ZeroMemory(pPerIOData->m_szBuffer, BUF_SIZE);
   pPerIOData->m_wsaBuf.buf = pPerIOData->m_szBuffer;
   pPerIOData->m_wsaBuf.len = BUF_SIZE;

   int nRet = WSARecv(s, &pPerIOData->m_wsaBuf, 1, NULL, &dwFlags, (LPOVERLAPPED)pPerIOData, NULL);
   if (nRet == SOCKET_ERROR)
   {
      if (WSAGetLastError() != WSA_IO_PENDING)
         return FALSE;
   }

   return TRUE;
}

// 按下发送按钮
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
      if (p->m_socket != g_socketListen)
         PostSend(p->m_socket, szMsg, strlen(szMsg));

      p = p->m_pNext;
   }
}

//////////////////////////////////////////////////////////////////////////
DWORD CountSetBits(ULONG_PTR bitMask)
{
   DWORD     LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
   DWORD     bitSetCount = 0;
   ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;

   for (DWORD i = 0; i <= LSHIFT; ++i)
   {
      bitSetCount += ((bitMask & bitTest) ? 1 : 0);
      bitTest /= 2;
   }

   return bitSetCount;
}

ULARGE_INTEGER GetProcessorInformation()
{
   PSYSTEM_LOGICAL_PROCESSOR_INFORMATION pBuf = NULL;    // 缓冲区指针
   PSYSTEM_LOGICAL_PROCESSOR_INFORMATION pTemp = NULL;   // 临时指针
   DWORD          dwReturnedLength = 0;       // GetLogicalProcessorInformation函数所用参数
   DWORD          dwLogicalProcessorCount = 0;// 逻辑处理器个数
   DWORD          dwProcessorCoreCount = 0;   // 物理处理器个数
   DWORD          dwByteOffset = 0;           // 
   ULARGE_INTEGER uli = { 0 };                // 返回值，低、高DWORD分别表示逻辑、物理处理器个数

   // 第1次调用
   GetLogicalProcessorInformation(pBuf, &dwReturnedLength);
   pBuf = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)new BYTE[dwReturnedLength];

   // 第2次调用
   GetLogicalProcessorInformation(pBuf, &dwReturnedLength);

   pTemp = pBuf;
   while (dwByteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= dwReturnedLength)
   {
      if (pTemp->Relationship == RelationProcessorCore)
      {
         // 物理处理器个数
         dwProcessorCoreCount++;
         // 逻辑处理器个数，一个超线程核心可以有多个逻辑处理器
         dwLogicalProcessorCount += CountSetBits(pTemp->ProcessorMask);
      }

      dwByteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
      pTemp++;
   }

   delete[] pBuf;
   uli.LowPart = dwLogicalProcessorCount;
   uli.HighPart = dwProcessorCoreCount;
   return uli;
}