// ����DLL�ĵ�����������ͺ���

#include <Windows.h>
#include <strsafe.h>
#include <tchar.h>

#define DLL_EXPORT
#include "DllSample.h"

// ����
int nValue;   // ��ͨ����
POSITION ps;  // �ṹ�����

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        nValue = 5;
        ps.x = 6;
        ps.y = 7;
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

// ��
CStudent::CStudent(LPTSTR lpName, int nAge)
{
    if (m_szName)
        StringCchCopy(m_szName, _countof(m_szName), lpName);

    m_nAge = nAge;
}

CStudent::~CStudent()
{}

LPTSTR CStudent::GetName()
{
    return m_szName;
}

int    CStudent::GetAge()
{
    return m_nAge;
}

// ����
int WINAPI funAdd(int a, int b)
{
    return a + b;
}

int WINAPI funMul(int a, int b)
{
    return a * b;
}