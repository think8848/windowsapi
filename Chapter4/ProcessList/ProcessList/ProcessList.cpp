#include <windows.h>
#include <Commctrl.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <tchar.h>
#include "resource.h"

#pragma comment(lib, "Comctl32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 全局变量
HINSTANCE g_hInstance;
HWND g_hwndDlg;                 // 对话框窗口句柄
HIMAGELIST g_hImagListSmall;    // 列表视图控件所用的图像列表

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 显示进程列表
BOOL GetProcessList();
// 提升本进程的权限
BOOL AdjustPrivileges(HANDLE hProcess, LPCTSTR lpPrivilegeName = SE_DEBUG_NAME);
// 暂停、恢复进程
VOID SuspendProcess(DWORD dwProcessId, BOOL bSuspend);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    g_hInstance = hInstance;

    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LVCOLUMN lvc = { 0 };
    POINT pt = { 0 };
    int nSelected, nRet;
    LVITEM lvi = { 0 };
    TCHAR szProcessName[MAX_PATH] = { 0 }, szProcessID[16] = { 0 }, szBuf[MAX_PATH] = { 0 };
    HANDLE hProcess;
    HMENU hMenu;
    BOOL bRet = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndDlg = hwndDlg;

        // 设置列表视图控件的扩展样式
        SendMessage(GetDlgItem(hwndDlg, IDC_LIST_PROCESS), LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        // 设置列标题：进程名称、进程ID、父进程ID、可执行文件路径
        lvc.mask = LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT;
        lvc.iSubItem = 0; lvc.cx = 150; lvc.pszText = TEXT("进程名称");
        SendMessage(GetDlgItem(hwndDlg, IDC_LIST_PROCESS), LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);
        lvc.iSubItem = 1; lvc.cx = 60; lvc.pszText = TEXT("进程ID");
        SendMessage(GetDlgItem(hwndDlg, IDC_LIST_PROCESS), LVM_INSERTCOLUMN, 1, (LPARAM)&lvc);
        lvc.iSubItem = 2; lvc.cx = 60; lvc.pszText = TEXT("父进程ID");
        SendMessage(GetDlgItem(hwndDlg, IDC_LIST_PROCESS), LVM_INSERTCOLUMN, 2, (LPARAM)&lvc);
        lvc.iSubItem = 3; lvc.cx = 260; lvc.pszText = TEXT("可执行文件路径");
        SendMessage(GetDlgItem(hwndDlg, IDC_LIST_PROCESS), LVM_INSERTCOLUMN, 3, (LPARAM)&lvc);

        // 为列表视图控件设置图像列表
        g_hImagListSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
            GetSystemMetrics(SM_CYSMICON), ILC_MASK | ILC_COLOR32, 500, 0);
        SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_SETIMAGELIST,
            LVSIL_SMALL, (LPARAM)g_hImagListSmall);

        // 提升本进程的权限
        AdjustPrivileges(GetCurrentProcess());
        // 显示进程列表
        GetProcessList();
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_REFRESH:
            // 显示进程列表
            GetProcessList();
            break;

        case ID_TERMINATE:
            // 结束选定进程
            nSelected = SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_GETSELECTIONMARK, 0, 0);

            // 确定要结束进程吗？
            lvi.iItem = nSelected; lvi.iSubItem = 0;
            lvi.mask = LVIF_TEXT;
            lvi.pszText = szProcessName;
            lvi.cchTextMax = _countof(szProcessName);
            SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_GETITEM, 0, (LPARAM)&lvi);
            wsprintf(szBuf, TEXT("确定要结束 %s 进程吗？"), lvi.pszText);
            nRet = MessageBox(hwndDlg, szBuf, TEXT("结束进程"), MB_OKCANCEL | MB_ICONINFORMATION | MB_DEFBUTTON2);
            if (nRet == IDCANCEL)
                return FALSE;

            // 获取进程句柄
            lvi.iSubItem = 1;
            lvi.pszText = szProcessID;
            lvi.cchTextMax = _countof(szProcessID);
            SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_GETITEM, 0, (LPARAM)&lvi);
            hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, _ttoi(lvi.pszText));
            if (hProcess)
            {
                // 结束进程
                bRet = TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
            }

            if (!bRet)
            {
                wsprintf(szBuf, TEXT("结束 %s 进程失败"), szProcessName);
                MessageBox(hwndDlg, szBuf, TEXT("错误提示"), MB_OK);
            }
            else
            {
                // 删除列表项
                SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_DELETEITEM, nSelected, 0);
            }
            break;

        case ID_OPEN:
            // 打开文件所在位置
            nSelected = SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_GETSELECTIONMARK, 0, 0);
            lvi.iItem = nSelected; lvi.iSubItem = 3;
            lvi.mask = LVIF_TEXT;
            lvi.pszText = szProcessName;
            lvi.cchTextMax = _countof(szProcessName);
            SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_GETITEM, 0, (LPARAM)&lvi);

            // 打开父目录并选定指定文件的命令：Explorer.exe /select,文件名称
            wsprintf(szBuf, TEXT("/select,%s"), lvi.pszText);
            ShellExecute(hwndDlg, TEXT("open"), TEXT("Explorer.exe"), szBuf, NULL, SW_SHOW);
            break;

        case ID_SUSPEND:
            // 暂停进程
            nSelected = SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_GETSELECTIONMARK, 0, 0);
            lvi.iItem = nSelected; lvi.iSubItem = 1;
            lvi.mask = LVIF_TEXT;
            lvi.pszText = szProcessID;
            lvi.cchTextMax = _countof(szProcessID);
            SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_GETITEM, 0, (LPARAM)&lvi);
            SuspendProcess(_ttoi(lvi.pszText), TRUE);
            break;

        case ID_RESUME:
            // 恢复进程
            nSelected = SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_GETSELECTIONMARK, 0, 0);
            lvi.iItem = nSelected; lvi.iSubItem = 1;
            lvi.mask = LVIF_TEXT;
            lvi.pszText = szProcessID;
            lvi.cchTextMax = _countof(szProcessID);
            SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_GETITEM, 0, (LPARAM)&lvi);
            SuspendProcess(_ttoi(lvi.pszText), FALSE);
            break;

        case IDCANCEL:
            ImageList_Destroy(g_hImagListSmall);
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;

    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->idFrom == IDC_LIST_PROCESS && ((LPNMHDR)lParam)->code == NM_RCLICK)
        {
            if (((LPNMITEMACTIVATE)lParam)->iItem < 0)
                return FALSE;

            // 如果可执行文件路径一列为空，则禁用结束该进程、打开文件所在位置、暂停进程、结束进程菜单
            nSelected = SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_GETSELECTIONMARK, 0, 0);
            hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MENU));
            lvi.iItem = nSelected; lvi.iSubItem = 3;
            lvi.mask = LVIF_TEXT;
            lvi.pszText = szProcessName;
            lvi.cchTextMax = _countof(szProcessName);
            SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_GETITEM, 0, (LPARAM)&lvi);
            if (_tcsicmp(lvi.pszText, TEXT("")) == 0)
            {
                EnableMenuItem(hMenu, ID_TERMINATE, MF_BYCOMMAND | MF_DISABLED);
                EnableMenuItem(hMenu, ID_OPEN, MF_BYCOMMAND | MF_DISABLED);
                EnableMenuItem(hMenu, ID_SUSPEND, MF_BYCOMMAND | MF_DISABLED);
                EnableMenuItem(hMenu, ID_RESUME, MF_BYCOMMAND | MF_DISABLED);
            }

            // 弹出快捷菜单
            GetCursorPos(&pt);
            TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, hwndDlg, NULL);
        }
        return TRUE;
    }

    return FALSE;
}

BOOL GetProcessList()
{
    HANDLE hSnapshot;
    PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
    BOOL bRet;
    HANDLE hProcess;
    TCHAR szPath[MAX_PATH] = { 0 };
    TCHAR szBuf[16] = { 0 };
    DWORD dwLen;
    SHFILEINFO fi = { 0 };
    int nImage;
    LVITEM lvi = { 0 };

    // 删除图像列表中的所有图像
    ImageList_Remove(g_hImagListSmall, -1);
    // 删除所有列表项
    SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_DELETEALLITEMS, 0, 0);

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        MessageBox(g_hwndDlg, TEXT("CreateToolhelp32Snapshot函数调用失败"), TEXT("提示"), MB_OK);
        return FALSE;
    }

    bRet = Process32First(hSnapshot, &pe);
    while (bRet)
    {
        nImage = -1;
        ZeroMemory(szPath, sizeof(szPath));
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
        if (hProcess)
        {
            // 获取可执行文件路径
            dwLen = _countof(szPath);
            QueryFullProcessImageName(hProcess, 0, szPath, &dwLen);
            // 获取程序图标
            SHGetFileInfo(szPath, 0, &fi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON);
            if (fi.hIcon)
                nImage = ImageList_AddIcon(g_hImagListSmall, fi.hIcon);

            CloseHandle(hProcess);
        }

        lvi.mask = LVIF_TEXT | LVIF_IMAGE;
        lvi.iItem = SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_GETITEMCOUNT, 0, 0);
        // 第1列，进程名称
        lvi.iSubItem = 0; lvi.pszText = pe.szExeFile; lvi.iImage = nImage;
        SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_INSERTITEM, 0, (LPARAM)&lvi);
        if (fi.hIcon)
            DestroyIcon(fi.hIcon);

        // 第2列，进程ID
        lvi.mask = LVIF_TEXT;
        lvi.iSubItem = 1; _itot_s(pe.th32ProcessID, szBuf, _countof(szBuf), 10); lvi.pszText = szBuf;
        SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_SETITEM, 0, (LPARAM)&lvi);

        // 第3列，父进程ID
        lvi.iSubItem = 2; _itot_s(pe.th32ParentProcessID, szBuf, _countof(szBuf), 10); lvi.pszText = szBuf;
        SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_SETITEM, 0, (LPARAM)&lvi);

        // 第4列，可执行文件路径
        lvi.iSubItem = 3; lvi.pszText = szPath;
        SendMessage(GetDlgItem(g_hwndDlg, IDC_LIST_PROCESS), LVM_SETITEM, 0, (LPARAM)&lvi);

        bRet = Process32Next(hSnapshot, &pe);
    }

    CloseHandle(hSnapshot);
    return TRUE;
}

VOID SuspendProcess(DWORD dwProcessId, BOOL bSuspend)
{
    HANDLE hSnapshot = INVALID_HANDLE_VALUE;
    THREADENTRY32 te = { sizeof(THREADENTRY32) };
    BOOL bRet = FALSE;
    HANDLE hThread = NULL;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return;

    bRet = Thread32First(hSnapshot, &te);
    while (bRet)
    {
        if (te.th32OwnerProcessID == dwProcessId)
        {
            hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
            if (hThread)
            {
                if (bSuspend)
                    SuspendThread(hThread);
                else
                    ResumeThread(hThread);

                // 关闭线程句柄
                CloseHandle(hThread);
            }
        }

        bRet = Thread32Next(hSnapshot, &te);
    }

    CloseHandle(hSnapshot);
    return;
}

BOOL AdjustPrivileges(HANDLE hProcess, LPCTSTR lpPrivilegeName)
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tokenPrivileges;

    if (OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken))
    {
        LUID luid;
        if (LookupPrivilegeValue(NULL, lpPrivilegeName, &luid))
        {
            tokenPrivileges.PrivilegeCount = 1;
            tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            tokenPrivileges.Privileges[0].Luid = luid;
            if (AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, 0, NULL, NULL))
                return TRUE;
        }

        CloseHandle(hToken);
    }

    return FALSE;
}