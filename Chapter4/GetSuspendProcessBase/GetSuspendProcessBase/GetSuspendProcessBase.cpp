#include <Windows.h>
#include <Psapi.h>
#include <tchar.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    TCHAR szCommandLine[MAX_PATH] = TEXT("ThreeThousandYears.exe"); // Ŀ�����
    MEMORY_BASIC_INFORMATION mbi = { 0 };                           // VirtualQueryEx����
    SIZE_T nBufSize;                                                // VirtualQueryEx����ֵ
    TCHAR szImageFile[MAX_PATH] = { 0 };                            // Ŀ���������·��
    TCHAR szBuf[MAX_PATH * 2] = { 0 };                              // ������

    // ����1
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };
    CONTEXT context = { 0 };

    // ����һ������Ľ���
    GetStartupInfo(&si);
    CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);

    // ��ȡĿ��������̻߳���
    context.ContextFlags = CONTEXT_ALL;
    GetThreadContext(pi.hThread, &context);

    // context.Eax�ǳ�����ڵ��ַ
    nBufSize = VirtualQueryEx(pi.hProcess, (LPVOID)context.Eax, &mbi, sizeof(mbi));
    if (nBufSize > 0)
    {
        GetMappedFileName(pi.hProcess, (LPVOID)context.Eax, szImageFile, _countof(szImageFile));
        wsprintf(szBuf, TEXT("%s ����ַ��0x%p"), szImageFile, mbi.AllocationBase);
        MessageBox(NULL, szBuf, TEXT("��ʾ"), MB_OK);
    }

    // ����2
    SYSTEM_INFO systemInfo = { 0 };
    LPVOID lpMinAppAddress = NULL;

    // ��ȡ���̵�ַ�ռ����С������ڴ��ַ����ҳ���С
    GetSystemInfo(&systemInfo);
    lpMinAppAddress = systemInfo.lpMinimumApplicationAddress;

    // ����С�ڴ��ַ��ʼ���ҵ�һ������MEM_IMAGE�洢���͵�ҳ��(ҳ��״̬Ϊ���ύ)
    while (lpMinAppAddress < systemInfo.lpMaximumApplicationAddress)
    {
        ZeroMemory(&mbi, sizeof(MEMORY_BASIC_INFORMATION));
        nBufSize = VirtualQueryEx(pi.hProcess, lpMinAppAddress, &mbi, sizeof(mbi));
        if (nBufSize == 0)
        {
            lpMinAppAddress = (LPBYTE)lpMinAppAddress + systemInfo.dwPageSize;
            continue;
        }

        switch (mbi.State)
        {
        case MEM_RESERVE:
        case MEM_FREE:
            lpMinAppAddress = (LPBYTE)(mbi.BaseAddress) + mbi.RegionSize;
            break;

        case MEM_COMMIT:
            if (mbi.Type == MEM_IMAGE)
            {
                GetMappedFileName(pi.hProcess, lpMinAppAddress, szImageFile, _countof(szImageFile));
                wsprintf(szBuf, TEXT("%s ����ַ��0x%p"), szImageFile, mbi.AllocationBase);
                MessageBox(NULL, szBuf, TEXT("��ʾ"), MB_OK);
                break;
            }

            lpMinAppAddress = (LPBYTE)(mbi.BaseAddress) + mbi.RegionSize;
            break;
        }

        // �ҵ��˾��˳�ѭ��
        if (mbi.Type == MEM_IMAGE)
            break;
    }

    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    return 0;
}