#include <winsock2.h>           // WinSock2ͷ�ļ�
#include <ws2tcpip.h>           // inet_pton inet_ntop��Ҫʹ�����ͷ�ļ�
#include <stdio.h>

#pragma comment(lib, "Ws2_32")  // WinSock2�����

// ��������
const int BUF_SIZE = 1024;

int main()
{
    WSADATA wsa = { 0 };
    SOCKET socketSendRecv = INVALID_SOCKET; // ���������շ������׽���
    sockaddr_in addrServer, addrClient;     // ���������ͻ��˵�ַ
    int nAddrLen = sizeof(sockaddr_in);     // sockaddr_in�ṹ��ĳ���
    CHAR szBuf[BUF_SIZE] = { 0 };           // �������ݻ�����
    CHAR szIP[24] = { 0 };                  // �ͻ���IP��ַ

    // ��ʼ��WinSock��
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // �����շ������׽���
    socketSendRecv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // ���շ������׽��ְ󶨵�����IP��ַ��ָ���˿�
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(8000);
    addrServer.sin_addr.S_un.S_addr = INADDR_ANY;
    bind(socketSendRecv, (SOCKADDR *)&addrServer, sizeof(addrServer));

    // �ӿͻ��˽������ݣ�recvfrom�������ڲ���addrClient�з��ؿͻ��˵�IP��ַ�Ͷ˿ں�
    recvfrom(socketSendRecv, szBuf, BUF_SIZE, 0, (SOCKADDR *)&addrClient, &nAddrLen);
    inet_ntop(AF_INET, &addrClient.sin_addr.S_un.S_addr, szIP, _countof(szIP));
    printf("�ӿͻ���[%s:%d]���յ����ݣ�%s\n", szIP, ntohs(addrClient.sin_port), szBuf);

    // ���յ������ݷ��ͻ�ȥ
    sendto(socketSendRecv, szBuf, strlen(szBuf), 0, (SOCKADDR *)&addrClient, nAddrLen);

    // �ر��շ������׽��֣��ͷ�WinSock��
    closesocket(socketSendRecv);
    WSACleanup();

    return 0;
}