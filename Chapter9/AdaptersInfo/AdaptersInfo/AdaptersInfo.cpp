#include <winsock2.h>               // WinSock2ͷ�ļ�
#include <IPHlpApi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32")      // WinSock2�����
#pragma comment(lib, "IPHlpApi")    // IPHlpApi�����

int main()
{
    PIP_ADAPTER_INFO pAdapterInfo = NULL;   // IP_ADAPTER_INFO�ṹ������������ָ��
    PIP_ADAPTER_INFO pAdapter = NULL;
    ULONG ulOutBufLen = 0;                  // �������Ĵ�С

    // ��һ�ε��÷������軺������С��Ȼ����仺����
    GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
    pAdapterInfo = (PIP_ADAPTER_INFO)new CHAR[ulOutBufLen];

    // �ڶ��ε��÷����������������Ϣ
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_SUCCESS)
    {
        pAdapter = pAdapterInfo;
        while (pAdapter)
        {
            // ������������
            printf("�����������ƣ�\t%s\n", pAdapter->AdapterName);

            // ������������
            printf("��������������\t%s\n", pAdapter->Description);

            // ��������Mac��ַ
            printf("������Mac��ַ��\t");
            for (UINT i = 0; i < pAdapter->AddressLength; i++)
            {
                if (i == (pAdapter->AddressLength - 1))
                    printf("%02X\n", (int)pAdapter->Address[i]);
                else
                    printf("%02X-", (int)pAdapter->Address[i]);
            }

            // IP��ַ
            printf("IP��ַ��\t%s\n", pAdapter->IpAddressList.IpAddress.String);
            // ��������
            printf("�������룺\t%s\n", pAdapter->IpAddressList.IpMask.String);

            // Ĭ������
            printf("Ĭ�����أ�\t%s\n", pAdapter->GatewayList.IpAddress.String);

            // �Ƿ�Ϊ�����������ö�̬��������Э��(DHCP)
            if (pAdapter->DhcpEnabled)
            {
                printf("����DHCP��\t��\n");
                printf("DHCP��������\t%s\n", pAdapter->DhcpServer.IpAddress.String);
            }
            else
            {
                printf("����DHCP��\t��\n");
            }

            printf("**********************************************************\n");

            pAdapter = pAdapter->Next;
        }
    }
    else
    {
        printf("GetAdaptersInfo��������ʧ�ܣ�\n");
    }

    delete[] pAdapterInfo;
    return 0;
}