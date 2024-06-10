#pragma once

// ���������ı�������ͺ���

#ifdef DLL_EXPORT
    #define DLL_VARABLE   extern "C" __declspec(dllexport)
    #define DLL_CLASS     __declspec(dllexport)
    #define DLL_API       extern "C" __declspec(dllexport)
#else
    #define DLL_VARABLE   extern "C" __declspec(dllimport)
    #define DLL_CLASS     __declspec(dllimport)
    #define DLL_API       extern "C" __declspec(dllimport)
#endif

/****************************************************************/
typedef struct _POSITION
{
    int x;
    int y;
}POSITION, * PPOSITION;

// ��������
DLL_VARABLE int nValue;   // ������ͨ����
DLL_VARABLE POSITION ps;  // �����ṹ�����

// ������
class DLL_CLASS CStudent
{
public:
    CStudent(LPTSTR lpName, int nAge);
    ~CStudent();

public:
    LPTSTR  GetName();
    int     GetAge();

private:
    TCHAR   m_szName[64];
    int     m_nAge;
};

// ��������
DLL_API int WINAPI funAdd(int a, int b);
DLL_API int WINAPI funMul(int a, int b);