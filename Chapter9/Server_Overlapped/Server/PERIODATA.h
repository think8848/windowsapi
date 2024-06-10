#pragma once

// 常量定义
const int BUF_SIZE = 4096;

// I/O操作类型，接受连接、接收数据、发送数据
enum IOOPERATION
{
    IO_UNKNOWN, IO_ACCEPT, IO_READ, IO_WRITE
};

// 自定义重叠结构，OVERLAPPED结构和I/O唯一数据
typedef struct _PERIODATA
{
    OVERLAPPED  m_overlapped;           // 重叠结构
    SOCKET      m_socket;               // 通信套接字句柄
    WSABUF      m_wsaBuf;               // 缓冲区结构
    CHAR        m_szBuffer[BUF_SIZE];   // 缓冲区
    IOOPERATION m_ioOperation;          // 操作类型
    _PERIODATA  *m_pNext;
}PERIODATA, *PPERIODATA;

PPERIODATA g_pPerIODataHeader;          // 自定义重叠结构链表表头

// 创建一个自定义重叠结构
PPERIODATA CreatePerIOData(SOCKET s)
{
    PPERIODATA pPerIOData = new PERIODATA;
    if (pPerIOData == NULL)
        return NULL;

    ZeroMemory(pPerIOData, sizeof(PERIODATA));
    pPerIOData->m_socket = s;
    pPerIOData->m_overlapped.hEvent = WSACreateEvent();

    EnterCriticalSection(&g_cs);
    // 添加第一个结点
    if (g_pPerIODataHeader == NULL)
    {
        g_pPerIODataHeader = pPerIOData;
        g_pPerIODataHeader->m_pNext = NULL;
    }
    else
    {
        pPerIOData->m_pNext = g_pPerIODataHeader;
        g_pPerIODataHeader = pPerIOData;
    }
    LeaveCriticalSection(&g_cs);

    return pPerIOData;
}

// 释放一个自定义重叠结构
VOID FreePerIOData(PPERIODATA pPerIOData)
{
    EnterCriticalSection(&g_cs);

    PPERIODATA p = g_pPerIODataHeader;
    if (p == pPerIOData)    // 移除的是头结点
    {
        g_pPerIODataHeader = g_pPerIODataHeader->m_pNext;
    }
    else
    {
        while (p != NULL)
        {
            if (p->m_pNext == pPerIOData)
            {
                p->m_pNext = pPerIOData->m_pNext;
                break;
            }

            p = p->m_pNext;
        }
    }

    if (pPerIOData->m_overlapped.hEvent)
        WSACloseEvent(pPerIOData->m_overlapped.hEvent);
    delete pPerIOData;
    LeaveCriticalSection(&g_cs);
}

// 根据事件对象查找自定义重叠结构
PPERIODATA FindPerIOData(HANDLE hEvent)
{
    EnterCriticalSection(&g_cs);

    PPERIODATA pPerIOData = g_pPerIODataHeader;
    while (pPerIOData != NULL)
    {
        if (pPerIOData->m_overlapped.hEvent == hEvent)
        {
            LeaveCriticalSection(&g_cs);
            return pPerIOData;
        }

        pPerIOData = pPerIOData->m_pNext;
    }

    LeaveCriticalSection(&g_cs);
    return NULL;
}

// 释放所有自定义重叠结构
VOID DeleteAllPerIOData()
{
    PERIODATA perIOData;

    PPERIODATA pPerIOData = g_pPerIODataHeader;
    while (pPerIOData != NULL)
    {
        perIOData = *pPerIOData;

        if (pPerIOData->m_overlapped.hEvent)
            WSACloseEvent(pPerIOData->m_overlapped.hEvent);
        delete pPerIOData;

        pPerIOData = perIOData.m_pNext;
    }
}