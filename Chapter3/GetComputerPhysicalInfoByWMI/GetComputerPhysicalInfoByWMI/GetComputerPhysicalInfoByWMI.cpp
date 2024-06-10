#include <windows.h>
#include <wbemidl.h>
#include <Propvarutil.h>
#include <comdef.h>
#include "resource.h"

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Propsys.lib")

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndEditInfo;
    HRESULT hr;
    IWbemLocator* pIWbemLocator = NULL;
    IWbemServices* pIWbemServices = NULL;
    IEnumWbemClassObject* pIEnumWbemClassObject = NULL;
    IWbemClassObject* pIWbemClassObject = NULL;
    ULONG uReturned = 0;
    VARIANT variant;
    TCHAR szBuf[256] = { 0 };

    switch (uMsg)
    {
    case WM_INITDIALOG:
        hwndEditInfo = GetDlgItem(hwndDlg, IDC_EDIT_INFO);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_GETINFO:
            // 初始化COM
            hr = CoInitializeEx(0, COINIT_MULTITHREADED);
            if (FAILED(hr))
                return FALSE;

            hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
                RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
            if (FAILED(hr))
            {
                CoUninitialize();
                return FALSE;
            }

            // 创建到WMI指定命名空间ROOT\CIMV2的连接
            hr = CoCreateInstance(CLSID_WbemLocator, NULL,
                CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pIWbemLocator);
            if (FAILED(hr))
            {
                CoUninitialize();
                return FALSE;
            }

            hr = pIWbemLocator->ConnectServer((BSTR)TEXT("ROOT\\CIMV2"),
                NULL, NULL, 0, NULL, 0, 0, &pIWbemServices);
            if (FAILED(hr))
            {
                pIWbemLocator->Release();
                CoUninitialize();
                return FALSE;
            }

            // 设置WMI连接的安全级别
            hr = CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
            if (FAILED(hr))
            {
                pIWbemServices->Release();
                pIWbemLocator->Release();
                CoUninitialize();
                return FALSE;
            }

            // 访问WMI
            // 获取硬盘序列号
            hr = pIWbemServices->ExecQuery((BSTR)TEXT("WQL"),
                (BSTR)TEXT("select * from Win32_DiskDrive where SerialNumber is not null"),
                WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pIEnumWbemClassObject);
            if (FAILED(hr))
            {
                pIWbemServices->Release();
                pIWbemLocator->Release();
                CoUninitialize();
                return FALSE;
            }

            while (pIEnumWbemClassObject)
            {
                hr = pIEnumWbemClassObject->Next(WBEM_INFINITE, 1, &pIWbemClassObject, &uReturned);
                if (uReturned == 0)
                    break;

                hr = pIWbemClassObject->Get(TEXT("SerialNumber"), 0, &variant, NULL, NULL);
                if (SUCCEEDED(hr))
                {
                    wsprintf(szBuf, TEXT("硬盘序列号：\t%s\n"), variant.bstrVal);
                    SendMessage(hwndEditInfo, EM_SETSEL, -1, -1);
                    SendMessage(hwndEditInfo, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

                    VariantClear(&variant);
                    pIWbemClassObject->Release();
                    pIWbemClassObject = NULL;
                }
            }

            // 获取主板序列号
            hr = pIWbemServices->ExecQuery((BSTR)TEXT("WQL"),
                (BSTR)TEXT("select * from Win32_BaseBoard where SerialNumber is not null"),
                WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pIEnumWbemClassObject);
            if (FAILED(hr))
            {
                pIWbemServices->Release();
                pIWbemLocator->Release();
                CoUninitialize();
                return FALSE;
            }

            hr = pIEnumWbemClassObject->Next(WBEM_INFINITE, 1, &pIWbemClassObject, &uReturned);
            if (uReturned == 0)
                break;

            hr = pIWbemClassObject->Get(TEXT("SerialNumber"), 0, &variant, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                wsprintf(szBuf, TEXT("主板序列号：\t%s\n"), variant.bstrVal);
                SendMessage(hwndEditInfo, EM_SETSEL, -1, -1);
                SendMessage(hwndEditInfo, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

                VariantClear(&variant);
                pIWbemClassObject->Release();
                pIWbemClassObject = NULL;
            }

            // 获取处理器CPUID
            hr = pIWbemServices->ExecQuery((BSTR)TEXT("WQL"),
                (BSTR)TEXT("select * from Win32_Processor where ProcessorId is not null"),
                WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pIEnumWbemClassObject);
            if (FAILED(hr))
            {
                pIWbemServices->Release();
                pIWbemLocator->Release();
                CoUninitialize();
                return FALSE;
            }

            hr = pIEnumWbemClassObject->Next(WBEM_INFINITE, 1, &pIWbemClassObject, &uReturned);
            if (uReturned == 0)
                break;

            hr = pIWbemClassObject->Get(TEXT("ProcessorId"), 0, &variant, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                wsprintf(szBuf, TEXT("CPUID：\t\t%s\n"), variant.bstrVal);
                SendMessage(hwndEditInfo, EM_SETSEL, -1, -1);
                SendMessage(hwndEditInfo, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

                VariantClear(&variant);
                pIWbemClassObject->Release();
                pIWbemClassObject = NULL;
            }

            // 获取BIOS序列号
            hr = pIWbemServices->ExecQuery((BSTR)TEXT("WQL"),
                (BSTR)TEXT("select * from Win32_BIOS where SerialNumber is not null"),
                WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pIEnumWbemClassObject);
            if (FAILED(hr))
            {
                pIWbemServices->Release();
                pIWbemLocator->Release();
                CoUninitialize();
                return FALSE;
            }

            hr = pIEnumWbemClassObject->Next(WBEM_INFINITE, 1, &pIWbemClassObject, &uReturned);
            if (uReturned == 0)
                break;

            hr = pIWbemClassObject->Get(TEXT("SerialNumber"), 0, &variant, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                wsprintf(szBuf, TEXT("BIOS序列号：\t%s\n"), variant.bstrVal);
                SendMessage(hwndEditInfo, EM_SETSEL, -1, -1);
                SendMessage(hwndEditInfo, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

                VariantClear(&variant);
                pIWbemClassObject->Release();
                pIWbemClassObject = NULL;
            }

            // 获取网卡Mac地址
            hr = pIWbemServices->ExecQuery((BSTR)TEXT("WQL"),
                (BSTR)TEXT("select * from Win32_NetworkAdapter where MACAddress is not null and not (PNPDeviceID like 'ROOT%')"),
                WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pIEnumWbemClassObject);
            if (FAILED(hr))
            {
                pIWbemServices->Release();
                pIWbemLocator->Release();
                CoUninitialize();
                return FALSE;
            }

            while (pIEnumWbemClassObject)
            {
                hr = pIEnumWbemClassObject->Next(WBEM_INFINITE, 1, &pIWbemClassObject, &uReturned);
                if (uReturned == 0)
                    break;

                hr = pIWbemClassObject->Get(TEXT("MACAddress"), 0, &variant, NULL, NULL);
                if (SUCCEEDED(hr))
                {
                    wsprintf(szBuf, TEXT("MAC地址：\t%s\n"), variant.bstrVal);
                    SendMessage(hwndEditInfo, EM_SETSEL, -1, -1);
                    SendMessage(hwndEditInfo, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

                    VariantClear(&variant);
                    pIWbemClassObject->Release();
                    pIWbemClassObject = NULL;
                }
            }

            // 清理工作
            pIEnumWbemClassObject->Release();
            pIWbemServices->Release();
            pIWbemLocator->Release();
            CoUninitialize();
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}