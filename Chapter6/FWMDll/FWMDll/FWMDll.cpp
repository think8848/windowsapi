#include <Windows.h>
#include <tchar.h>

// 全局变量
LPBYTE pExtTextOutW;

TCHAR szText1[] = TEXT("屏幕");
TCHAR szText2[] = TEXT("用户名");
TCHAR szText3[] = TEXT("购买者");
TCHAR szTextReplace[] = TEXT("                                                              ");
LPTSTR lpStr;

VOID InterceptExtTextOutW(LPTSTR lpText);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
   BYTE bExtTextOutWCall[] = { 0xFF,  0x74,  0x24,  0x18,  0xE8,  0x00,  0x00,  0x00,  0x00,  0x90,  0x90,  0x90,  0x90 };
   DWORD dwOldProtect;

   switch (ul_reason_for_call)
   {
   case DLL_PROCESS_ATTACH:
      // 获取ExtTextOutW函数的地址
      pExtTextOutW = (LPBYTE)GetProcAddress(GetModuleHandle(TEXT("Gdi32.dll")), "ExtTextOutW");
      *(LPINT)(bExtTextOutWCall + 5) = (INT)InterceptExtTextOutW - (INT)pExtTextOutW - 0x9;
      // 把ExtTextOutW函数起始处改为Call
      VirtualProtect(pExtTextOutW, 512, PAGE_EXECUTE_READWRITE, &dwOldProtect);
      WriteProcessMemory(GetCurrentProcess(), pExtTextOutW, bExtTextOutWCall, sizeof(bExtTextOutWCall), NULL);
      VirtualProtect(pExtTextOutW, 512, dwOldProtect, &dwOldProtect);
      break;

   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
   case DLL_PROCESS_DETACH:
      break;
   }

   return TRUE;
}

VOID InterceptExtTextOutW(LPTSTR lpText)
{
   _asm
   {
      pushad
   }

   if ((lpStr = _tcsstr(lpText, szText1)) || (lpStr = _tcsstr(lpText, szText2)) || (lpStr = _tcsstr(lpText, szText3)))
   {
      memcpy(lpStr, szTextReplace, _tcslen(lpStr) * sizeof(TCHAR));
   }

   _asm
   {
      popad
      // 修改ExtTextOutW函数的时候，有一个push和call，导致esp减了8个字节
      add esp, 8

      // 已经恢复各通用寄存器和堆栈空间布局，开始执行原ExtTextOutW函数开头的一些指令行
      mov edi, edi
      push ebp
      mov ebp, esp
      push ecx
      mov dword ptr[ebp - 0x4], 0x49414E

      // 跳转到我们修改过的指令的下一条指令行继续执行，就是ExtTextOutW + 0xD地址处
      mov eax, pExtTextOutW
      add eax, 0xD
      jmp eax
   }
}