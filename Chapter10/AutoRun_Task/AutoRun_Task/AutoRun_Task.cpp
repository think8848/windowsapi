#include <windows.h>
#include <Taskschd.h>
#include <comdef.h>
#include "resource.h"

#pragma comment(lib, "taskschd.lib")

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// ��������ƻ�
BOOL MyCreateTaskScheduler(LPCTSTR lpszTaskName, LPCTSTR lpszProgramPath,
    LPCTSTR lpszParameter, LPCTSTR lpszAuthor);
// ɾ������ƻ�
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
            if (MyCreateTaskScheduler(TEXT("��������HelloWindows"),
                TEXT("F:\\Source\\Windows\\Chapter2\\HelloWindows\\Debug\\HelloWindows.exe"),
                NULL, TEXT("������Ʒ")))
                MessageBox(hwndDlg, TEXT("��������ƻ��ɹ�"), TEXT("�����ɹ�"), MB_OK);
            break;

        case IDC_BTN_DELETE:
            if (MyDeleteTaskScheduler(TEXT("��������HelloWindows")))
                MessageBox(hwndDlg, TEXT("ɾ������ƻ��ɹ�"), TEXT("�����ɹ�"), MB_OK);
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
  * �������ܣ�		ͨ������COM��ӿں�����������ƻ�ʵ�ֳ��򿪻��Զ�����
  * ���������˵����
    1. lpszTaskName������ʾ�������ƣ�����ָ��
    2. lpszProgramPath������ʾҪ�����Զ������ĳ���·��������ָ��
    3. lpszParameter������ʾ�������������Ҫ��������ΪNULL
    4. lpszAuthor������ʾ�����ߣ�����Ҫ��������ΪNULL
  * ע�⣺�ú�����Ҫʹ��Taskschd.h��comdef.hͷ�ļ�������Ҫtaskschd.lib�����
**********************************************************************************/
BOOL MyCreateTaskScheduler(LPCTSTR lpszTaskName, LPCTSTR lpszProgramPath,
    LPCTSTR lpszParameter, LPCTSTR lpszAuthor)
{
    HRESULT hr;

    if (!lpszTaskName || !lpszProgramPath)
        return FALSE;

    // ��ʼ��COM������COM��ȫ����
    hr = CoInitializeEx(NULL, 0);

    hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);

    // ����һ��ITaskService����
    ITaskService* pTaskService;
    hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_SERVER, IID_ITaskService,
        (LPVOID*)&pTaskService);
    // �����ӵ����ص����������Ȼ��ſ���ʹ��ITaskService�����е���������
    hr = pTaskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());

    // ��ȡ�������ļ��е�·��
    ITaskFolder* pTaskFolder;
    hr = pTaskService->GetFolder((BSTR)TEXT("\\"), &pTaskFolder);

    // ����Ѿ�������ͬ�������Ƶ�����ƻ�����ɾ��
    pTaskFolder->DeleteTask((BSTR)lpszTaskName, 0);

    // ����ITaskDefinition����(���������)��ʹ��ITaskDefinition�������������Ϣ
    ITaskDefinition* pTaskDefinition;
    hr = pTaskService->NewTask(0, &pTaskDefinition);
    pTaskService->Release();

    // ����ע����Ϣ
    IRegistrationInfo* pRegistrationInfo;
    hr = pTaskDefinition->get_RegistrationInfo(&pRegistrationInfo);
    hr = pRegistrationInfo->put_Author((BSTR)lpszAuthor);
    pRegistrationInfo->Release();

    // ����������Ϣ
    IPrincipal* pPrincipal;
    hr = pTaskDefinition->get_Principal(&pPrincipal);
    hr = pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
    hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
    pPrincipal->Release();

    // ����������Ϣ
    ITaskSettings* pTaskSettings;
    hr = pTaskDefinition->get_Settings(&pTaskSettings);
    hr = pTaskSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE);
    hr = pTaskSettings->put_DisallowStartIfOnBatteries(VARIANT_FALSE);
    hr = pTaskSettings->put_AllowDemandStart(VARIANT_TRUE);
    hr = pTaskSettings->put_StartWhenAvailable(VARIANT_FALSE);
    hr = pTaskSettings->put_MultipleInstances(TASK_INSTANCES_PARALLEL);
    hr = pTaskSettings->put_WakeToRun(VARIANT_TRUE);
    pTaskSettings->Release();

    // ��ȡIActionCollection����
    IActionCollection* pActionCollection;
    hr = pTaskDefinition->get_Actions(&pActionCollection);

    // ����ִ�в���
    IAction* pAction;
    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
    pActionCollection->Release();

    // ��ȡIExecAction����
    IExecAction* pExecAction;
    hr = pAction->QueryInterface(IID_IExecAction, (LPVOID*)&pExecAction);
    pAction->Release();

    // ���ÿ�ִ���ļ�·���Ͳ���
    hr = pExecAction->put_Path((BSTR)lpszProgramPath);
    hr = pExecAction->put_Arguments((BSTR)lpszParameter);
    pExecAction->Release();

    // ��ȡITriggerCollection����
    ITriggerCollection* pTriggerCollection;
    hr = pTaskDefinition->get_Triggers(&pTriggerCollection);

    // ����������
    ITrigger* pTrigger;
    hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
    pTriggerCollection->Release();

    // �������񵽸������ļ���
    IRegisteredTask* pRegisteredTask;
    hr = pTaskFolder->RegisterTaskDefinition((BSTR)lpszTaskName, pTaskDefinition,
        TASK_CREATE_OR_UPDATE, _variant_t(), _variant_t(),
        TASK_LOGON_INTERACTIVE_TOKEN, _variant_t(TEXT("")), &pRegisteredTask);

    // �ͷ���ض���
    pRegisteredTask->Release();
    pTaskDefinition->Release();
    pTaskFolder->Release();
    // �ر�COM��
    CoUninitialize();

    return TRUE;
}

BOOL MyDeleteTaskScheduler(LPCTSTR lpszTaskName)
{
    HRESULT hr;

    if (!lpszTaskName)
        return FALSE;

    // ��ʼ��COM������COM��ȫ����
    hr = CoInitializeEx(NULL, 0);

    hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0, NULL);

    // ����һ��ITaskService����
    ITaskService* pTaskService;
    hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_SERVER, IID_ITaskService,
        (LPVOID*)&pTaskService);
    // �����ӵ����ص����������Ȼ��ſ���ʹ��ITaskService�����е���������
    hr = pTaskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());

    // ��ȡ�������ļ��е�·��
    ITaskFolder* pTaskFolder;
    hr = pTaskService->GetFolder((BSTR)TEXT("\\"), &pTaskFolder);

    // ɾ������ƻ�
    pTaskFolder->DeleteTask((BSTR)lpszTaskName, 0);

    // �ͷ���ض���
    pTaskFolder->Release();
    pTaskService->Release();
    // �ر�COM��
    CoUninitialize();

    return TRUE;
}