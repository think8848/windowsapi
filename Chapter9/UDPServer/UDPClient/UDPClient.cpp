#include <winsock2.h>           // WinSock2ͷ�ļ�
#include <ws2tcpip.h>           // inet_pton inet_ntop��Ҫʹ�����ͷ�ļ�
#include <stdio.h>

#pragma comment(lib, "Ws2_32")  // WinSock2�����

// ��������
const int BUF_SIZE = 1024;

int main()
{
    WSADATA wsa = { 0 };
    SOCKET socketSendRecv = INVALID_SOCKET; // �ͻ��˵��շ������׽���
    sockaddr_in addrServer;                 // ��������ַ
    int nAddrLen = sizeof(sockaddr_in);     // sockaddr_in�ṹ��ĳ���
    CHAR szBuf[BUF_SIZE] = "��ã�������";  // �������ݻ�����
    CHAR szIP[24] = { 0 };                  // ������IP��ַ

    // ��ʼ��WinSock��
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // �����շ������׽���
    socketSendRecv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // ���������������
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(8000);
    inet_pton(AF_INET, "127.0.0.1", &addrServer.sin_addr.S_un.S_addr);
    sendto(socketSendRecv, szBuf, strlen(szBuf), 0, (SOCKADDR *)&addrServer, nAddrLen);

    // �ӷ�������������
    sockaddr_in addr;   // ��һ��recvfrom���صķ�����IP��ַ�Ͷ˿ں�
    ZeroMemory(szBuf, sizeof(szBuf));
    recvfrom(socketSendRecv, szBuf, BUF_SIZE, 0, (sockaddr *)&addr, &nAddrLen);
    inet_ntop(AF_INET, &addr.sin_addr.S_un.S_addr, szIP, _countof(szIP));
    printf("�ӷ�����[%s:%d]�������ݣ�%s\n", szIP, ntohs(addr.sin_port), szBuf);

    // �ر��շ������׽��֣��ͷ�WinSock��
    closesocket(socketSendRecv);
    WSACleanup();

    return 0;
}