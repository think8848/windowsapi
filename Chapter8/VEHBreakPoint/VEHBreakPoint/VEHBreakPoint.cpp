#include <Windows.h>

// ȫ�ֱ���
LPVOID g_pfnLoadLibraryExWAddress;  // LoadLibraryExW������ַ
BYTE g_bOriginalCodeByte;           // ����LoadLibraryExW�����ĵ�һ��ָ����
HWND g_hwndDlg;                     // CreateProcessInjectDll���򴰿ھ��

// ��������
// ����int 3�ϵ�(����ԭָ����)
BYTE SetBreakPoint(LPVOID lpCodeAddr);
// �Ƴ�int 3�ϵ�
VOID RemoveBreakPoint(LPVOID lpCodeAddr, BYTE bOriginalCodeByte);

// ΪLoadLibraryExW������int 3�ϵ�ע��һ���������쳣�������
LONG CALLBACK LoadLibraryExWBPHandler(PEXCEPTION_POINTERS ExceptionInfo);

// LoadLibraryExW����int 3�ж��Ժ�ִ���û�������Զ������
VOID LoadLibraryExWCustomActions(LPVOID lpCodeAddr, LPVOID lpStackAddr);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hwndDlg = FindWindow(TEXT("#32770"), TEXT("CreateProcessInjectDll"));

        // ��ȡKernelBase.LoadLibraryExW�����ĵ�ַ
        g_pfnLoadLibraryExWAddress = (LPVOID)GetProcAddress(
            GetModuleHandle(TEXT("KernelBase.dll")), "LoadLibraryExW");

        // ΪLoadLibraryExW������int 3�ϵ�ע��һ���������쳣�������
        AddVectoredExceptionHandler(1, LoadLibraryExWBPHandler);
        // ��LoadLibraryExW����������һ��int 3�ϵ�
        g_bOriginalCodeByte = SetBreakPoint(g_pfnLoadLibraryExWAddress);
        break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}

// �ڲ�����
LONG CALLBACK LoadLibraryExWBPHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
    DWORD dwExceptionCode = ExceptionInfo->ExceptionRecord->ExceptionCode;

    if (dwExceptionCode == EXCEPTION_BREAKPOINT)
    {
        // ����Ƿ����������õ�int 3�ϵ㣬������ǣ��������ݸ������쳣�������
        if (ExceptionInfo->ExceptionRecord->ExceptionAddress != g_pfnLoadLibraryExWAddress)
            return EXCEPTION_CONTINUE_SEARCH;

        // ��LoadLibraryExW����ִ���û�������Զ������
        LoadLibraryExWCustomActions(ExceptionInfo->ExceptionRecord->ExceptionAddress,
            (LPVOID)(ExceptionInfo->ContextRecord->Esp));

        // ��ʱ�Ƴ�int 3�ϵ�
        RemoveBreakPoint(g_pfnLoadLibraryExWAddress, g_bOriginalCodeByte);
        // ���õ����ж�
        ExceptionInfo->ContextRecord->EFlags |= 0x100;

        // ����ִ�з���int 3�쳣��ָ���Ϊ�����˵����жϣ��������ᵥ��ִ�����һ��ָ��
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    else if (dwExceptionCode == EXCEPTION_SINGLE_STEP)
    {
        if (ExceptionInfo->ExceptionRecord->ExceptionAddress !=
            (LPBYTE)g_pfnLoadLibraryExWAddress + 2)
            return EXCEPTION_CONTINUE_SEARCH;

        // �Ѿ�ִ�����û����Զ��������Ҳ�Ѿ�����ִ����LoadLibraryExW�����ĵ�һ����䣬
        // ��������int 3�ϵ㣬�Եȴ���һ��LoadLibraryExW��������
        SetBreakPoint(g_pfnLoadLibraryExWAddress);

        // ��������
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    // ��int 3�ϵ�͵����ж϶�������
    return EXCEPTION_CONTINUE_SEARCH;
}

BYTE SetBreakPoint(LPVOID lpCodeAddr)
{
    BYTE bOriginalCodeByte;
    BYTE bInt3 = 0xCC;
    DWORD dwOldProtect;

    // ��ȡLoadLibraryExW�����ĵ�һ��ָ����
    ReadProcessMemory(GetCurrentProcess(), lpCodeAddr, &bOriginalCodeByte,
        sizeof(bOriginalCodeByte), NULL);

    // ����int 3�ϵ�
    VirtualProtect(lpCodeAddr, 1, PAGE_EXECUTE_READWRITE, &dwOldProtect);
    WriteProcessMemory(GetCurrentProcess(), lpCodeAddr, &bInt3, sizeof(bInt3), NULL);
    VirtualProtect(lpCodeAddr, 1, dwOldProtect, &dwOldProtect);

    return bOriginalCodeByte;
}

VOID RemoveBreakPoint(LPVOID lpCodeAddr, BYTE bOriginalCodeByte)
{
    DWORD dwOldProtect;

    VirtualProtect(lpCodeAddr, 1, PAGE_EXECUTE_READWRITE, &dwOldProtect);
    WriteProcessMemory(GetCurrentProcess(), lpCodeAddr, &bOriginalCodeByte,
        sizeof(bOriginalCodeByte), NULL);
    VirtualProtect(lpCodeAddr, 1, dwOldProtect, &dwOldProtect);
}

VOID LoadLibraryExWCustomActions(LPVOID lpCodeAddr, LPVOID lpStackAddr)
{
    TCHAR szDllName[MAX_PATH] = { 0 };

    ReadProcessMemory(GetCurrentProcess(), (LPVOID)(*(LPDWORD)((LPBYTE)lpStackAddr + 4)),
        szDllName, sizeof(szDllName), NULL);

    // dll������ʾ��CreateProcessInjectDll����ı༭�ؼ���
    SendDlgItemMessage(g_hwndDlg, 1002, EM_SETSEL, -1, -1);
    SendDlgItemMessage(g_hwndDlg, 1002, EM_REPLACESEL, TRUE, (LPARAM)szDllName);
    SendDlgItemMessage(g_hwndDlg, 1002, EM_REPLACESEL, TRUE, (LPARAM)TEXT("\r\n"));
}