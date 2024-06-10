#pragma once

#ifdef DLL_EXPORT
    #define DLL_API       extern "C" __declspec(dllexport)
#else
    #define DLL_API       extern "C" __declspec(dllimport)
#endif

typedef interface ICMLuaUtil ICMLuaUtil;

typedef struct ICMLuaUtilVtbl {

    BEGIN_INTERFACE

        HRESULT(STDMETHODCALLTYPE* QueryInterface)(
            __RPC__in       ICMLuaUtil* This,
            __RPC__in       REFIID      riid,
            _COM_Outptr_    void**      ppvObject);

        ULONG(STDMETHODCALLTYPE* AddRef)(__RPC__in ICMLuaUtil* This);

        ULONG(STDMETHODCALLTYPE* Release)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method1)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method2)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method3)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method4)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method5)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method6)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* ShellExec)(
            __RPC__in ICMLuaUtil* This,
            _In_      LPCWSTR   lpFile,
            _In_opt_  LPCTSTR   lpParameters,
            _In_opt_  LPCTSTR   lpDirectory,
            _In_      ULONG     fMask,
            _In_      ULONG     nShow);

        HRESULT(STDMETHODCALLTYPE* SetRegistryStringValue)(
            __RPC__in ICMLuaUtil* This,
            _In_      HKEY hKey,
            _In_opt_  LPCTSTR lpSubKey,
            _In_opt_  LPCTSTR lpValueName,
            _In_      LPCTSTR lpValueString);

        HRESULT(STDMETHODCALLTYPE* Method9)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method10)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method11)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method12)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method13)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method14)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method15)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method16)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method17)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method18)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method19)(__RPC__in ICMLuaUtil* This);

        HRESULT(STDMETHODCALLTYPE* Method20)(__RPC__in ICMLuaUtil* This);

    END_INTERFACE

} *PICMLuaUtilVtbl;

interface ICMLuaUtil
{
    CONST_VTBL struct ICMLuaUtilVtbl* lpVtbl;
};

// 导出函数
DLL_API VOID CALLBACK PassUAC(HWND hwnd, HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow);

// 内部函数
HRESULT CoCreateInstanceAsAdmin(HWND hwnd, REFCLSID rclsid, REFIID riid, LPVOID* ppVoid);