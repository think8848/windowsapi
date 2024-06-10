#include <windows.h>
#include <NetCon.h>
#include "resource.h"

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL ConnectNetwork(BOOL bConnect);

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
        case IDC_BTN_CONNECT:
            ConnectNetwork(TRUE);
            break;

        case IDC_BTN_DISCONNECT:
            ConnectNetwork(FALSE);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

BOOL ConnectNetwork(BOOL bConnect)
{
    HRESULT hr;
    INetConnectionManager*  pNetConnManager;
    INetConnection*         pNetConn;
    IEnumNetConnection*     pEnumNetConn;
    ULONG                   uCeltFetched;

    CoInitializeEx(NULL, 0);
    hr = CoCreateInstance(CLSID_ConnectionManager, NULL, CLSCTX_SERVER,
        IID_INetConnectionManager, (LPVOID*)&pNetConnManager);
    if (FAILED(hr))
        return FALSE;

    pNetConnManager->EnumConnections(NCME_DEFAULT, &pEnumNetConn);
    pNetConnManager->Release();
    if (pEnumNetConn == NULL)
        return FALSE;

    while (pEnumNetConn->Next(1, &pNetConn, &uCeltFetched) == S_OK)
    {
        if (bConnect)
            pNetConn->Connect();              //启用连接
        else
            pNetConn->Disconnect();           //禁用连接
    }

    CoUninitialize();
    return TRUE;
}