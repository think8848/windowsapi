#include <Windows.h>
#include <tchar.h>

// ȫ�ֱ���
LPBYTE pExtTextOutW;

TCHAR szText1[] = TEXT("��Ļ");
TCHAR szText2[] = TEXT("�û���");
TCHAR szText3[] = TEXT("������");
TCHAR szTextReplace[] = TEXT("                                                              ");
LPTSTR lpStr;

VOID InterceptExtTextOutW(LPTSTR lpText);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    BYTE bExtTextOutWCall[] = { 0xFF,  0x74,  0x24,  0x18,  0xE8,  0x00,  0x00,  0x00,  0x00,  0x90,  0x90,  0x90,  0x90 };
    DWORD dwOldProtect;
    MEMORY_BASIC_INFORMATION mbi = { 0 };

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // ��ȡExtTextOutW�����ĵ�ַ
        pExtTextOutW = (LPBYTE)GetProcAddress(GetModuleHandle(TEXT("Gdi32.dll")), "ExtTextOutW");
        *(LPINT)(bExtTextOutWCall + 5) = (INT)InterceptExtTextOutW - (INT)pExtTextOutW - 0x9;
        // ��ExtTextOutW������ʼ����ΪCall
        VirtualQuery(pExtTextOutW, &mbi, sizeof(mbi));
        VirtualProtect(pExtTextOutW, 512, PAGE_EXECUTE_READWRITE, &dwOldProtect);
        WriteProcessMemory(GetCurrentProcess(), pExtTextOutW, bExtTextOutWCall, sizeof(bExtTextOutWCall), NULL);
        VirtualProtect(pExtTextOutW, 512, mbi.Protect, &dwOldProtect);
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

_declspec(naked)VOID InterceptExtTextOutW(LPTSTR lpText)
{
    _asm
    {
        // �����������ͷ�����������
        push ebp
        mov  ebp, esp
        sub  esp, 0x10
        push ebx
        push esi
        push edi

        // ���Ᵽ��ecx��edx��eax��esp��ʱ����Ҫ����
        push ecx
        push edx
    }

    // �����C++�����п��ܻ���һЩpushָ�����û��ϵ�����������Զ��ָ�esp��ֵ�������������
    if ((lpStr = _tcsstr(lpText, szText1)) || (lpStr = _tcsstr(lpText, szText2)) || 
        (lpStr = _tcsstr(lpText, szText3)))
    {
        memcpy(lpStr, szTextReplace, _tcslen(lpStr) * sizeof(TCHAR));
    }

    _asm
    {
        // �ָ�edx��ecx
        pop edx
        pop ecx

        // �����������β�����������
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp

        // �޸�ExtTextOutW������ʱ����һ��push��call������esp����8���ֽ�
        add esp, 8

        // �Ѿ��ָ���ͨ�üĴ����Ͷ�ջ�ռ䲼�֣���ʼִ��ԭExtTextOutW������ͷ��һЩָ����
        mov  edi, edi
        push ebp
        mov  ebp, esp
        push ecx
        mov  dword ptr[ebp - 0x4], 0x49414E

        // ��ת�������޸Ĺ���ָ�����һ��ָ���м���ִ�У�����ExtTextOutW + 0xD��ַ��
        mov eax, pExtTextOutW
        add eax, 0xD
        jmp eax
    }
}