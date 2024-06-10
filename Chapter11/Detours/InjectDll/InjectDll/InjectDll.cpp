#include <windows.h>
#include <tchar.h>
#include "..\..\..\Detours-master\include\detours.h"

// ����Ϊx86ʱ��Ҫʹ�õ�.lib
#pragma comment(lib, "..\\..\\..\\Detours-master\\lib.X86\\detours.lib")
// ����Ϊx64ʱ��Ҫʹ�õ�.lib
//#pragma comment(lib, "..\\..\\..\\Detours-master\\lib.X64\\detours.lib")

// Ŀ�꺯��ָ��(��static�ؼ���˵�������ڱ��ļ�)
static BOOL(WINAPI* OriginalExtTextOutW)(HDC hdc, int x, int y, UINT options,
    const RECT* lprect, LPCWSTR lpString, UINT c, const INT* lpDx) = ExtTextOutW;

// �Զ��庯��
BOOL WINAPI DetourExtTextOutW(HDC hdc, int x, int y, UINT options,
    RECT* lprect, LPCWSTR lpString, UINT c, INT* lpDx)
{
    TCHAR szText1[] = TEXT("��Ļ");
    TCHAR szText2[] = TEXT("�û���");
    TCHAR szText3[] = TEXT("������");
    TCHAR szTextReplace[] = TEXT("                                                          ");
    LPCTSTR lpStr;

    if ((lpStr = _tcsstr(lpString, szText1)) ||
        (lpStr = _tcsstr(lpString, szText2)) ||
        (lpStr = _tcsstr(lpString, szText3)))
    {
        memcpy((LPVOID)lpStr, szTextReplace, _tcslen(lpStr) * sizeof(TCHAR));
    }

    OriginalExtTextOutW(hdc, x, y, options, lprect, lpString, c, lpDx);

    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    // �����ǰ�����Ǹ���������ִ���κδ���
    if (DetourIsHelperProcess())
        return TRUE;

    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        // �ָ���ǰ���̵ĵ����
        DetourRestoreAfterWith();

        // ����(��ʼ)����
        DetourTransactionBegin();
        // ָ�������߳�
        DetourUpdateThread(GetCurrentThread());
        // ִ��Hook����
        DetourAttach(&(PVOID&)OriginalExtTextOutW, DetourExtTextOutW);
        // �ύ����
        DetourTransactionCommit();
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        // ִ��Unhook����
        DetourDetach(&(PVOID&)OriginalExtTextOutW, DetourExtTextOutW);
        DetourTransactionCommit();
    }

    return TRUE;
}