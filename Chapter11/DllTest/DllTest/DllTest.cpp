#include <Windows.h>

#define DLL_EXPORT
#include "DllTest.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MessageBox(NULL, TEXT("����ִ��DllMain��ڵ㺯��"), TEXT("��ʾ"), MB_OK);
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

// ��������
VOID ShowMessage()
{
    MessageBox(NULL, TEXT("���ǵ�������"), TEXT("��ʾ"), MB_OK);
}