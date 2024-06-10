#include <windows.h>
#include "resource.h"

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CreateProcessAndInjectDll();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_CREATE:
            CreateProcessAndInjectDll();
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

BOOL CreateProcessAndInjectDll()
{
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };
    TCHAR szExePath[MAX_PATH] = TEXT("ThreeThousandYears.exe");
    TCHAR szDllPath[MAX_PATH] = TEXT("MessageBoxDll.dll");
    BOOL bRet;

    // 29�ֽڵĻ���ָ���MAX_PATH * sizeof(TCHAR)�ֽڵ�Ҫע���dll������
    BYTE ShellCode[29 + MAX_PATH * sizeof(TCHAR)] =
    {
        0x60,                           // pushad
        0x9C,                           // pushfd
        0x68,0xAA,0xBB,0xCC,0xDD,       // push [0xDDCCBBAA](0xDDCCBBAA��Ŀ�������Ҫע���dll������)
        0xFF,0x15,0xDD,0xCC,0xBB,0xAA,  // call [0xDDCCBBAA](0xDDCCBBAA��LoadLibraryW�����ĵ�ַ)
        0x9D,                           // popfd
        0x61,                           // popad
        0xFF,0x25,0xAA,0xBB,0xCC,0xDD,  // jmp [0xDDCCBBAA](0xDDCCBBAAΪĿ�����ԭ��ڵ�)
        0xAA,0xAA,0xAA,0xAA,            // ����loadlibraryW������ַ��4�ֽ���������
        0xAA,0xAA,0xAA,0xAA,            // ����Ŀ�����ԭ��ڵ��ַ��4�ֽ���������
        0,                              // ����ʼ���Ǵ��Ҫע���dll���Ƶ���������
    };

    // �Թ���ģʽ����һ������
    bRet = CreateProcess(szExePath, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
    if (!bRet)
        return FALSE;

    // ��ȡĿ��������̻߳���(EIP)
    CONTEXT context;
    context.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext(pi.hThread, &context))
        return FALSE;

    // ���LoadLibraryW�����ĵ�ַ
    DWORD dwLoadLibraryWAddr = (DWORD)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), 
        "LoadLibraryW");
    if (!dwLoadLibraryWAddr)
        return FALSE;

    // ��Ŀ������з����ڴ棬���ShellCode
    LPVOID lpMemoryRemote = VirtualAllocEx(pi.hProcess, NULL, 29 + MAX_PATH * sizeof(TCHAR),
        MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!lpMemoryRemote)
        return FALSE;

    // push [0xDDCCBBAA](0xDDCCBBAA��Ŀ�������Ҫע���dll������) ƫ��ShellCode + 3
    *(DWORD*)(ShellCode + 3) = (DWORD)lpMemoryRemote + 29;

    // call [0xDDCCBBAA](0xDDCCBBAA��LoadLibraryW�����ĵ�ַ)      ƫ��ShellCode + 9
    *(DWORD*)(ShellCode + 9) = (DWORD)lpMemoryRemote + 21;

    // jmp [0xDDCCBBAA](0xDDCCBBAAΪĿ�����ԭ��ڵ�)             ƫ��ShellCode + 17
    *(DWORD*)(ShellCode + 17) = (DWORD)lpMemoryRemote + 25;

    // ����loadlibraryW������ַ��4�ֽ���������                    ƫ��ShellCode + 21
    *(DWORD*)(ShellCode + 21) = dwLoadLibraryWAddr;

    // ����Ŀ�����ԭ��ڵ��ַ��4�ֽ���������                    ƫ��ShellCode + 25
    *(DWORD*)(ShellCode + 25) = context.Eip;

    // ����ʼ���Ǵ��Ҫע���dll���Ƶ���������                  ƫ��ShellCode + 29
    memcpy_s(ShellCode + 29, MAX_PATH * sizeof(TCHAR), szDllPath, sizeof(szDllPath));

    // ��shellcodeд��Ŀ�����
    if (!WriteProcessMemory(pi.hProcess, lpMemoryRemote, ShellCode, 
        29 + MAX_PATH * sizeof(TCHAR), NULL))
        return FALSE;

    // �޸�Ŀ����̵�EIP��ִ�б�ע��Ĵ���
    context.Eip = (DWORD)lpMemoryRemote;
    if (!SetThreadContext(pi.hThread, &context))
        return FALSE;

    // �ָ�Ŀ����̵�ִ��
    ResumeThread(pi.hThread);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return TRUE;
}