#include <Windows.h>
#include "StaticLinkLibrary.h"                  // StaticLinkLibrary.hͷ�ļ�

#pragma comment(lib, "StaticLinkLibrary.lib")   // StaticLinkLibrary.lib�����

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    TCHAR szBuf[256] = { 0 };

    wsprintf(szBuf, TEXT("funAdd(5, 6) = %d\nfunMul(5, 6) = %d"), funAdd(5, 6), funMul(5, 6));
    MessageBox(NULL, szBuf, TEXT("��ʾ"), MB_OK);
}