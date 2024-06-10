#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32")

// 根据时间协议Time Protocol返回的时间更新系统时间
VOID SetTimeFromTP(ULONG ulTime);

int main()
{
    WSADATA wsa = { 0 };
    SOCKET socketClient = INVALID_SOCKET;
    sockaddr_in addrServer; // 时间服务器的地址
    int nRet;

    // 1、初始化WinSock库
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 0;

    // 2、创建与服务器进行通信的套接字
    if ((socketClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
        return 0;

    // 3、使用connect函数来请求与服务器连接
    addrServer.sin_family = AF_INET;
    inet_pton(AF_INET, "132.163.97.1", (LPVOID)(&addrServer.sin_addr.S_un.S_addr));
    addrServer.sin_port = htons(37);
    if (connect(socketClient, (sockaddr *)&addrServer, sizeof(addrServer)) == SOCKET_ERROR)
        return 0;

    // 4、接收时间协议返回的时间，自1900年1月1日0点0分0秒0毫秒逝去的毫秒数
    ULONG ulTime = 0;
    nRet = recv(socketClient, (PCHAR)&ulTime, sizeof(ulTime), 0);
    if (nRet > 0)
    {
        // 网络字节序到本机字节序
        ulTime = ntohl(ulTime);
        SetTimeFromTP(ulTime);
        printf("成功与时间服务器的时间同步！\n");
    }
    else
    {
        printf("时间服务器未能返回时间！\n");
    }

    closesocket(socketClient);
    WSACleanup();
    return 0;
}

// 根据时间协议Time Protocol返回的时间更新系统时间
VOID SetTimeFromTP(ULONG ulTime)
{
    FILETIME ft;
    SYSTEMTIME st;
    ULARGE_INTEGER uli;

    st.wYear = 1900;
    st.wMonth = 1;
    st.wDay = 1;
    st.wHour = 0;
    st.wMinute = 0;
    st.wSecond = 0;
    st.wMilliseconds = 0;

    // 系统时间转换为文件时间才可以加上已经逝去的时间ulTime
    SystemTimeToFileTime(&st, &ft);

    // 文件时间单位是1/1000 0000秒，即1000万分之1秒(100-nanosecond)
    // 不要将指向FILETIME结构的指针强制转换为ULARGE_INTEGER *或__int64 *值，
    // 因为这可能导致64位Windows上的对齐错误
    uli.HighPart = ft.dwHighDateTime;
    uli.LowPart = ft.dwLowDateTime;
    uli.QuadPart += (ULONGLONG)10000000 * ulTime;
    ft.dwHighDateTime = uli.HighPart;
    ft.dwLowDateTime = uli.LowPart;

    // 再将文件时间转换为系统时间，更新系统时间
    FileTimeToSystemTime(&ft, &st);

    SetSystemTime(&st);
}