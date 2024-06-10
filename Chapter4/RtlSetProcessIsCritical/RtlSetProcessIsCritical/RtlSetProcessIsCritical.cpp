#include <windows.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// º¯ÊýÉùÃ÷
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

typedef NTSTATUS(__cdecl* pfnRtlSetProcessIsCritical)(_In_ BOOL NewValue, _Out_opt_ PBOOL OldValue, _In_ BOOL CheckFlag);
pfnRtlSetProcessIsCritical pRtlSetProcessIsCritical;

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HMODULE hNtdll;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        hNtdll = LoadLibrary(TEXT("Ntdll.dll"));
        if (hNtdll)
        {
            pRtlSetProcessIsCritical = (pfnRtlSetProcessIsCritical)GetProcAddress(hNtdll, "RtlSetProcessIsCritical");
            if (pRtlSetProcessIsCritical)
                pRtlSetProcessIsCritical(TRUE, NULL, FALSE);
            else
                FreeLibrary(hNtdll);
        }
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            if (pRtlSetProcessIsCritical)
                pRtlSetProcessIsCritical(FALSE, NULL, FALSE);
            if (hNtdll)
                FreeLibrary(hNtdll);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}