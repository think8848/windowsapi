#include <windows.h>

// ȫ�ֱ���
int g_n;

// ��������
VOID MyAdd(int a, int b);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int a = 3, b = 5;
    TCHAR szBuf[64] = { 0 };
    MyAdd(a, b);
    wsprintf(szBuf, TEXT("a + b = %d"), g_n);
    MessageBox(NULL, szBuf, TEXT("��ʾ"), MB_OK);

    return 0;
}

VOID MyAdd(int a, int b)
{
    g_n = a + b;
}