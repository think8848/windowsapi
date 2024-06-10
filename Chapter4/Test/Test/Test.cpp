#include <Windows.h>

BOOL g_bLegalCopy = FALSE;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 判断软件是否为正版的代码，如果是则设置全局变量g_bLegalCopy为TRUE

    if (g_bLegalCopy)
        MessageBox(NULL, TEXT("正版软件"), TEXT("欢迎"), MB_OK);
    else
        MessageBox(NULL, TEXT("盗版软件"), TEXT("鄙视"), MB_OK);

    return 0;
}