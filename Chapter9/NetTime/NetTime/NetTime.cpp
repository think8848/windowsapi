#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32")

// ����ʱ��Э��Time Protocol���ص�ʱ�����ϵͳʱ��
VOID SetTimeFromTP(ULONG ulTime);

int main()
{
    WSADATA wsa = { 0 };
    SOCKET socketClient = INVALID_SOCKET;
    sockaddr_in addrServer; // ʱ��������ĵ�ַ
    int nRet;

    // 1����ʼ��WinSock��
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 0;

    // 2�����������������ͨ�ŵ��׽���
    if ((socketClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
        return 0;

    // 3��ʹ��connect���������������������
    addrServer.sin_family = AF_INET;
    inet_pton(AF_INET, "132.163.97.1", (LPVOID)(&addrServer.sin_addr.S_un.S_addr));
    addrServer.sin_port = htons(37);
    if (connect(socketClient, (sockaddr *)&addrServer, sizeof(addrServer)) == SOCKET_ERROR)
        return 0;

    // 4������ʱ��Э�鷵�ص�ʱ�䣬��1900��1��1��0��0��0��0������ȥ�ĺ�����
    ULONG ulTime = 0;
    nRet = recv(socketClient, (PCHAR)&ulTime, sizeof(ulTime), 0);
    if (nRet > 0)
    {
        // �����ֽ��򵽱����ֽ���
        ulTime = ntohl(ulTime);
        SetTimeFromTP(ulTime);
        printf("�ɹ���ʱ���������ʱ��ͬ����\n");
    }
    else
    {
        printf("ʱ�������δ�ܷ���ʱ�䣡\n");
    }

    closesocket(socketClient);
    WSACleanup();
    return 0;
}

// ����ʱ��Э��Time Protocol���ص�ʱ�����ϵͳʱ��
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

    // ϵͳʱ��ת��Ϊ�ļ�ʱ��ſ��Լ����Ѿ���ȥ��ʱ��ulTime
    SystemTimeToFileTime(&st, &ft);

    // �ļ�ʱ�䵥λ��1/1000 0000�룬��1000���֮1��(100-nanosecond)
    // ��Ҫ��ָ��FILETIME�ṹ��ָ��ǿ��ת��ΪULARGE_INTEGER *��__int64 *ֵ��
    // ��Ϊ����ܵ���64λWindows�ϵĶ������
    uli.HighPart = ft.dwHighDateTime;
    uli.LowPart = ft.dwLowDateTime;
    uli.QuadPart += (ULONGLONG)10000000 * ulTime;
    ft.dwHighDateTime = uli.HighPart;
    ft.dwLowDateTime = uli.LowPart;

    // �ٽ��ļ�ʱ��ת��Ϊϵͳʱ�䣬����ϵͳʱ��
    FileTimeToSystemTime(&ft, &st);

    SetSystemTime(&st);
}