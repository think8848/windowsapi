#pragma once

// 声明导出的变量、类和函数

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

// 导出变量
DLL_VARABLE int nValue;   // 导出普通变量
DLL_VARABLE POSITION ps;  // 导出结构体变量

// 导出类
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

// 导出函数
DLL_API int WINAPI funAdd(int a, int b);
DLL_API int WINAPI funMul(int a, int b);