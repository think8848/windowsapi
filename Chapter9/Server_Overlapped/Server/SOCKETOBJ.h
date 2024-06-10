#pragma once

// �׽��ֶ����������ýṹ��
typedef struct _SOCKETOBJ
{
    SOCKET      m_socket;   // ͨ���׽��־��
    CHAR        m_szIP[16]; // �ͻ���IP
    USHORT      m_usPort;   // �ͻ��˶˿ں�
    _SOCKETOBJ  *m_pNext;   // ��һ���׽��ֶ���ṹ��ָ��
}SOCKETOBJ, *PSOCKETOBJ;

PSOCKETOBJ g_pSocketObjHeader;  // �׽��ֶ��������ͷ
int g_nTotalClient;             // �ͻ���������

CRITICAL_SECTION g_cs;          // �ٽ�����������ͬ�����׽��ֶ�������Ĳ���

// ����һ���׽��ֶ���
PSOCKETOBJ CreateSocketObj(SOCKET s)
{
    PSOCKETOBJ pSocketObj = new SOCKETOBJ;
    if (pSocketObj == NULL)
        return NULL;

    EnterCriticalSection(&g_cs);

    pSocketObj->m_socket = s;

    // ��ӵ�һ�����
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

// �ͷ�һ���׽��ֶ���
VOID FreeSocketObj(PSOCKETOBJ pSocketObj)
{
    EnterCriticalSection(&g_cs);

    PSOCKETOBJ p = g_pSocketObjHeader;
    if (p == pSocketObj)    // �Ƴ�����ͷ���
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

// �����׽��ֲ����׽��ֶ���
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

// �ͷ������׽��ֶ���
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