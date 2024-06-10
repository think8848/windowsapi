#include <Windows.h>
#include "StaticLinkLibrary.h"                  // StaticLinkLibrary.h头文件

#pragma comment(lib, "StaticLinkLibrary.lib")   // StaticLinkLibrary.lib对象库

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    TCHAR szBuf[256] = { 0 };

    wsprintf(szBuf, TEXT("funAdd(5, 6) = %d\nfunMul(5, 6) = %d"), funAdd(5, 6), funMul(5, 6));
    MessageBox(NULL, szBuf, TEXT("提示"), MB_OK);
}