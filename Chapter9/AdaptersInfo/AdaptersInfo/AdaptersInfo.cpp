#include <winsock2.h>               // WinSock2头文件
#include <IPHlpApi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32")      // WinSock2导入库
#pragma comment(lib, "IPHlpApi")    // IPHlpApi导入库

int main()
{
    PIP_ADAPTER_INFO pAdapterInfo = NULL;   // IP_ADAPTER_INFO结构体链表缓冲区的指针
    PIP_ADAPTER_INFO pAdapter = NULL;
    ULONG ulOutBufLen = 0;                  // 缓冲区的大小

    // 第一次调用返回所需缓冲区大小，然后分配缓冲区
    GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
    pAdapterInfo = (PIP_ADAPTER_INFO)new CHAR[ulOutBufLen];

    // 第二次调用返回所需的适配器信息
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_SUCCESS)
    {
        pAdapter = pAdapterInfo;
        while (pAdapter)
        {
            // 适配器的名称
            printf("适配器的名称：\t%s\n", pAdapter->AdapterName);

            // 适配器的描述
            printf("适配器的描述：\t%s\n", pAdapter->Description);

            // 适配器的Mac地址
            printf("适配器Mac地址：\t");
            for (UINT i = 0; i < pAdapter->AddressLength; i++)
            {
                if (i == (pAdapter->AddressLength - 1))
                    printf("%02X\n", (int)pAdapter->Address[i]);
                else
                    printf("%02X-", (int)pAdapter->Address[i]);
            }

            // IP地址
            printf("IP地址：\t%s\n", pAdapter->IpAddressList.IpAddress.String);
            // 子网掩码
            printf("子网掩码：\t%s\n", pAdapter->IpAddressList.IpMask.String);

            // 默认网关
            printf("默认网关：\t%s\n", pAdapter->GatewayList.IpAddress.String);

            // 是否为此适配器启用动态主机配置协议(DHCP)
            if (pAdapter->DhcpEnabled)
            {
                printf("启用DHCP：\t是\n");
                printf("DHCP服务器：\t%s\n", pAdapter->DhcpServer.IpAddress.String);
            }
            else
            {
                printf("启用DHCP：\t否\n");
            }

            printf("**********************************************************\n");

            pAdapter = pAdapter->Next;
        }
    }
    else
    {
        printf("GetAdaptersInfo函数调用失败！\n");
    }

    delete[] pAdapterInfo;
    return 0;
}