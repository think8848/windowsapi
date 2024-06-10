#pragma once

// 套接字对象链表所用结构体
typedef struct _SOCKETOBJ
{
    SOCKET      m_socket;   // 通信套接字句柄
    CHAR        m_szIP[16]; // 客户端IP
    USHORT      m_usPort;   // 客户端端口号
    _SOCKETOBJ  *m_pNext;   // 下一个套接字对象结构体指针
}SOCKETOBJ, *PSOCKETOBJ;

PSOCKETOBJ g_pSocketObjHeader;  // 套接字对象链表表头
int g_nTotalClient;             // 客户端总数量

CRITICAL_SECTION g_cs;          // 临界区对象，用于同步对套接字对象链表的操作

// 创建一个套接字对象
PSOCKETOBJ CreateSocketObj(SOCKET s)
{
    PSOCKETOBJ pSocketObj = new SOCKETOBJ;
    if (pSocketObj == NULL)
        return NULL;

    EnterCriticalSection(&g_cs);

    pSocketObj->m_socket = s;

    // 添加第一个结点
    if (g_pSocketObjHeader == NULL)
    {
        g_pSocketObjHeader = pSocketObj;
        g_pSocketObjHeader->m_pNext = NULL;
    }
    else
    {
        pSocketObj->m_pNext = g_pSocketObjHeader;
        g_pSocketObjHeader = pSocketObj;
    }

    g_nTotalClient++;

    LeaveCriticalSection(&g_cs);
    return pSocketObj;
}

// 释放一个套接字对象
VOID FreeSocketObj(PSOCKETOBJ pSocketObj)
{
    EnterCriticalSection(&g_cs);

    PSOCKETOBJ p = g_pSocketObjHeader;
    if (p == pSocketObj)    // 移除的是头结点
    {
        g_pSocketObjHeader = g_pSocketObjHeader->m_pNext;
    }
    else
    {
        while (p != NULL)
        {
            if (p->m_pNext == pSocketObj)
            {
                p->m_pNext = pSocketObj->m_pNext;
                break;
            }

            p = p->m_pNext;
        }
    }

    if (pSocketObj->m_socket != INVALID_SOCKET)
        closesocket(pSocketObj->m_socket);
    delete pSocketObj;

    g_nTotalClient--;

    LeaveCriticalSection(&g_cs);
}

// 根据套接字查找套接字对象
PSOCKETOBJ FindSocketObj(SOCKET s)
{
    EnterCriticalSection(&g_cs);

    PSOCKETOBJ pSocketObj = g_pSocketObjHeader;
    while (pSocketObj != NULL)
    {
        if (pSocketObj->m_socket == s)
        {
            LeaveCriticalSection(&g_cs);
            return pSocketObj;
        }

        pSocketObj = pSocketObj->m_pNext;
    }

    LeaveCriticalSection(&g_cs);
    return NULL;
}

// 释放所有套接字对象
VOID DeleteAllSocketObj()
{
    SOCKETOBJ socketObj;
    PSOCKETOBJ pSocketObj = g_pSocketObjHeader;

    while (pSocketObj != NULL)
    {
        socketObj = *pSocketObj;

        if (pSocketObj->m_socket != INVALID_SOCKET)
            closesocket(pSocketObj->m_socket);
        delete pSocketObj;

        pSocketObj = socketObj.m_pNext;
    }
}