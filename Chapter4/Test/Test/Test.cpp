#include <Windows.h>

BOOL g_bLegalCopy = FALSE;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // �ж�����Ƿ�Ϊ����Ĵ��룬�����������ȫ�ֱ���g_bLegalCopyΪTRUE

    if (g_bLegalCopy)
        MessageBox(NULL, TEXT("�������"), TEXT("��ӭ"), MB_OK);
    else
        MessageBox(NULL, TEXT("�������"), TEXT("����"), MB_OK);

    return 0;
}