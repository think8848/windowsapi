#include <windows.h>

// 全局变量
int g_n;

// 函数声明
VOID MyAdd(int a, int b);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int a = 3, b = 5;
    TCHAR szBuf[64] = { 0 };
    MyAdd(a, b);
    wsprintf(szBuf, TEXT("a + b = %d"), g_n);
    MessageBox(NULL, szBuf, TEXT("提示"), MB_OK);

    return 0;
}

VOID MyAdd(int a, int b)
{
    g_n = a + b;
}