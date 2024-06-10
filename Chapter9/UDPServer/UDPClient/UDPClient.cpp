#include <winsock2.h>           // WinSock2头文件
#include <ws2tcpip.h>           // inet_pton inet_ntop需要使用这个头文件
#include <stdio.h>

#pragma comment(lib, "Ws2_32")  // WinSock2导入库

// 常量定义
const int BUF_SIZE = 1024;

int main()
{
    WSADATA wsa = { 0 };
    SOCKET socketSendRecv = INVALID_SOCKET; // 客户端的收发数据套接字
    sockaddr_in addrServer;                 // 服务器地址
    int nAddrLen = sizeof(sockaddr_in);     // sockaddr_in结构体的长度
    CHAR szBuf[BUF_SIZE] = "你好，老王！";  // 发送数据缓冲区
    CHAR szIP[24] = { 0 };                  // 服务器IP地址

    // 初始化WinSock库
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // 创建收发数据套接字
    socketSendRecv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // 向服务器发送数据
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(8000);
    inet_pton(AF_INET, "127.0.0.1", &addrServer.sin_addr.S_un.S_addr);
    sendto(socketSendRecv, szBuf, strlen(szBuf), 0, (SOCKADDR *)&addrServer, nAddrLen);

    // 从服务器接收数据
    sockaddr_in addr;   // 看一下recvfrom返回的服务器IP地址和端口号
    ZeroMemory(szBuf, sizeof(szBuf));
    recvfrom(socketSendRecv, szBuf, BUF_SIZE, 0, (sockaddr *)&addr, &nAddrLen);
    inet_ntop(AF_INET, &addr.sin_addr.S_un.S_addr, szIP, _countof(szIP));
    printf("从服务器[%s:%d]返回数据：%s\n", szIP, ntohs(addr.sin_port), szBuf);

    // 关闭收发数据套接字，释放WinSock库
    closesocket(socketSendRecv);
    WSACleanup();

    return 0;
}