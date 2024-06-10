#include <winsock2.h>           // WinSock2头文件
#include <ws2tcpip.h>           // inet_pton inet_ntop需要使用这个头文件
#include <stdio.h>

#pragma comment(lib, "Ws2_32")  // WinSock2导入库

// 常量定义
const int BUF_SIZE = 1024;

int main()
{
    WSADATA wsa = { 0 };
    SOCKET socketSendRecv = INVALID_SOCKET; // 服务器的收发数据套接字
    sockaddr_in addrServer, addrClient;     // 服务器、客户端地址
    int nAddrLen = sizeof(sockaddr_in);     // sockaddr_in结构体的长度
    CHAR szBuf[BUF_SIZE] = { 0 };           // 接收数据缓冲区
    CHAR szIP[24] = { 0 };                  // 客户端IP地址

    // 初始化WinSock库
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // 创建收发数据套接字
    socketSendRecv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // 将收发数据套接字绑定到任意IP地址和指定端口
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(8000);
    addrServer.sin_addr.S_un.S_addr = INADDR_ANY;
    bind(socketSendRecv, (SOCKADDR *)&addrServer, sizeof(addrServer));

    // 从客户端接收数据，recvfrom函数会在参数addrClient中返回客户端的IP地址和端口号
    recvfrom(socketSendRecv, szBuf, BUF_SIZE, 0, (SOCKADDR *)&addrClient, &nAddrLen);
    inet_ntop(AF_INET, &addrClient.sin_addr.S_un.S_addr, szIP, _countof(szIP));
    printf("从客户端[%s:%d]接收到数据：%s\n", szIP, ntohs(addrClient.sin_port), szBuf);

    // 把收到的数据发送回去
    sendto(socketSendRecv, szBuf, strlen(szBuf), 0, (SOCKADDR *)&addrClient, nAddrLen);

    // 关闭收发数据套接字，释放WinSock库
    closesocket(socketSendRecv);
    WSACleanup();

    return 0;
}