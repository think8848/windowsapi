#pragma once

// ��������
const int BUF_SIZE = 4096;

// I/O�������ͣ��������ӡ��������ݡ���������
enum IOOPERATION
{
    IO_UNKNOWN, IO_ACCEPT, IO_READ, IO_WRITE
};

// �Զ����ص��ṹ��OVERLAPPED�ṹ��I/OΨһ����
typedef struct _PERIODATA
{
    OVERLAPPED  m_overlapped;           // �ص��ṹ
    SOCKET      m_socket;               // ͨ���׽��־��
    WSABUF      m_wsaBuf;               // �������ṹ
    CHAR        m_szBuffer[BUF_SIZE];   // ������
    IOOPERATION m_ioOperation;          // ��������
    _PERIODATA  *m_pNext;
}PERIODATA, *PPERIODATA;

PPERIODATA g_pPerIODataHeader;          // �Զ����ص��ṹ�����ͷ

// ����һ���Զ����ص��ṹ
PPERIODATA CreatePerIOData(SOCKET s)
{
    PPERIODATA pPerIOData = new PERIODATA;
    if (pPerIOData == NULL)
        return NULL;

    ZeroMemory(pPerIOData, sizeof(PERIODATA));
    pPerIOData->m_socket = s;
    pPerIOData->m_overlapped.hEvent = WSACreateEvent();

    EnterCriticalSection(&g_cs);
    // ��ӵ�һ�����
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

// �ͷ�һ���Զ����ص��ṹ
VOID FreePerIOData(PPERIODATA pPerIOData)
{
    EnterCriticalSection(&g_cs);

    PPERIODATA p = g_pPerIODataHeader;
    if (p == pPerIOData)    // �Ƴ�����ͷ���
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

// �����¼���������Զ����ص��ṹ
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

// �ͷ������Զ����ص��ṹ
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