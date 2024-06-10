#include <windows.h>
#include <Taskschd.h>
#include <comdef.h>
#include "resource.h"

#pragma comment(lib, "taskschd.lib")

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 创建任务计划
BOOL MyCreateTaskScheduler(LPCTSTR lpszTaskName, LPCTSTR lpszProgramPath,
    LPCTSTR lpszParameter, LPCTSTR lpszAuthor);
// 删除任务计划
BOOL MyDeleteTaskScheduler(LPCTSTR lpszTaskName);

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
        case IDC_BTN_OK:
            if (MyCreateTaskScheduler(TEXT("开机运行HelloWindows"),
                TEXT("F:\\Source\\Windows\\Chapter2\\HelloWindows\\Debug\\HelloWindows.exe"),
                NULL, TEXT("老王出品")))
                MessageBox(hwndDlg, TEXT("创建任务计划成功"), TEXT("操作成功"), MB_OK);
            break;

        case IDC_BTN_DELETE:
            if (MyDeleteTaskScheduler(TEXT("开机运行HelloWindows")))
                MessageBox(hwndDlg, TEXT("删除任务计划成功"), TEXT("操作成功"), MB_OK);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

/*********************************************************************************
  * 函数功能：		通过调用COM库接口函数创建任务计划实现程序开机自动启动
  * 输入参数的说明：
    1. lpszTaskName参数表示任务名称，必须指定
    2. lpszProgramPath参数表示要开机自动启动的程序路径，必须指定
    3. lpszParameter参数表示程序参数，不需要可以设置为NULL
    4. lpszAuthor参数表示创建者，不需要可以设置为NULL
  * 注意：该函数需要使用Taskschd.h和comdef.h头文件，并需要taskschd.lib导入库
**********************************************************************************/
BOOL MyCreateTaskScheduler(LPCTSTR lpszTaskName, LPCTSTR lpszProgramPath,
    LPCTSTR lpszParameter, LPCTSTR lpszAuthor)
{
    HRESULT hr;

    if (!lpszTaskName || !lpszProgramPath)
        return FALSE;

    // 初始化COM并设置COM安全级别
    hr = CoInitializeEx(NULL, 0);

    hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);

    // 创建一个ITaskService对象
    ITaskService* pTaskService;
    hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_SERVER, IID_ITaskService,
        (LPVOID*)&pTaskService);
    // 先连接到本地电脑任务服务，然后才可以使用ITaskService对象中的其他方法
    hr = pTaskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());

    // 获取根任务文件夹的路径
    ITaskFolder* pTaskFolder;
    hr = pTaskService->GetFolder((BSTR)TEXT("\\"), &pTaskFolder);

    // 如果已经存在相同任务名称的任务计划，则删除
    pTaskFolder->DeleteTask((BSTR)lpszTaskName, 0);

    // 创建ITaskDefinition对象(任务定义对象)，使用ITaskDefinition对象定义任务的信息
    ITaskDefinition* pTaskDefinition;
    hr = pTaskService->NewTask(0, &pTaskDefinition);
    pTaskService->Release();

    // 设置注册信息
    IRegistrationInfo* pRegistrationInfo;
    hr = pTaskDefinition->get_RegistrationInfo(&pRegistrationInfo);
    hr = pRegistrationInfo->put_Author((BSTR)lpszAuthor);
    pRegistrationInfo->Release();

    // 设置主体信息
    IPrincipal* pPrincipal;
    hr = pTaskDefinition->get_Principal(&pPrincipal);
    hr = pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
    hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
    pPrincipal->Release();

    // 设置配置信息
    ITaskSettings* pTaskSettings;
    hr = pTaskDefinition->get_Settings(&pTaskSettings);
    hr = pTaskSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE);
    hr = pTaskSettings->put_DisallowStartIfOnBatteries(VARIANT_FALSE);
    hr = pTaskSettings->put_AllowDemandStart(VARIANT_TRUE);
    hr = pTaskSettings->put_StartWhenAvailable(VARIANT_FALSE);
    hr = pTaskSettings->put_MultipleInstances(TASK_INSTANCES_PARALLEL);
    hr = pTaskSettings->put_WakeToRun(VARIANT_TRUE);
    pTaskSettings->Release();

    // 获取IActionCollection对象
    IActionCollection* pActionCollection;
    hr = pTaskDefinition->get_Actions(&pActionCollection);

    // 创建执行操作
    IAction* pAction;
    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
    pActionCollection->Release();

    // 获取IExecAction对象
    IExecAction* pExecAction;
    hr = pAction->QueryInterface(IID_IExecAction, (LPVOID*)&pExecAction);
    pAction->Release();

    // 设置可执行文件路径和参数
    hr = pExecAction->put_Path((BSTR)lpszProgramPath);
    hr = pExecAction->put_Arguments((BSTR)lpszParameter);
    pExecAction->Release();

    // 获取ITriggerCollection对象
    ITriggerCollection* pTriggerCollection;
    hr = pTaskDefinition->get_Triggers(&pTriggerCollection);

    // 创建触发器
    ITrigger* pTrigger;
    hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
    pTriggerCollection->Release();

    // 保存任务到根任务文件夹
    IRegisteredTask* pRegisteredTask;
    hr = pTaskFolder->RegisterTaskDefinition((BSTR)lpszTaskName, pTaskDefinition,
        TASK_CREATE_OR_UPDATE, _variant_t(), _variant_t(),
        TASK_LOGON_INTERACTIVE_TOKEN, _variant_t(TEXT("")), &pRegisteredTask);

    // 释放相关对象
    pRegisteredTask->Release();
    pTaskDefinition->Release();
    pTaskFolder->Release();
    // 关闭COM库
    CoUninitialize();

    return TRUE;
}

BOOL MyDeleteTaskScheduler(LPCTSTR lpszTaskName)
{
    HRESULT hr;

    if (!lpszTaskName)
        return FALSE;

    // 初始化COM并设置COM安全级别
    hr = CoInitializeEx(NULL, 0);

    hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);

    // 创建一个ITaskService对象
    ITaskService* pTaskService;
    hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_SERVER, IID_ITaskService,
        (LPVOID*)&pTaskService);
    // 先连接到本地电脑任务服务，然后才可以使用ITaskService对象中的其他方法
    hr = pTaskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());

    // 获取根任务文件夹的路径
    ITaskFolder* pTaskFolder;
    hr = pTaskService->GetFolder((BSTR)TEXT("\\"), &pTaskFolder);

    // 删除任务计划
    pTaskFolder->DeleteTask((BSTR)lpszTaskName, 0);

    // 释放相关对象
    pTaskFolder->Release();
    pTaskService->Release();
    // 关闭COM库
    CoUninitialize();

    return TRUE;
}