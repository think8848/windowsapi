#include <Windows.h>
#include <strsafe.h>
#include <tchar.h>
#include "DllSample.h"                  // DllSample.h头文件

#pragma comment(lib, "DllSample.lib")   // DllSample.lib导入库文件

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    TCHAR szBuf[256] = { 0 };
    TCHAR szBuf2[512] = { 0 };

    // 测试导出变量
    wsprintf(szBuf, TEXT("nValue = %d\nps.x = %d, ps.y = %d\n"), nValue, ps.x, ps.y);
    StringCchCopy(szBuf2, _countof(szBuf2), szBuf);

    // 测试导出类
    CStudent student((LPTSTR)TEXT("老王"), 40);
    wsprintf(szBuf, TEXT("姓名：%s， 年龄：%d\n"), student.GetName(), student.GetAge());
    StringCchCat(szBuf2, _countof(szBuf2), szBuf);

    // 测试导出函数
    wsprintf(szBuf, TEXT("funAdd(5, 6) = %d\nfunMul(5, 6) = %d"), funAdd(5, 6), funMul(5, 6));
    StringCchCat(szBuf2, _countof(szBuf2), szBuf);

    MessageBox(NULL, szBuf2, TEXT("提示"), MB_OK);
}