#include <Windows.h>
#include <tchar.h>                  // _tcslen

#pragma comment(lib, "Winmm.lib")   // ����������PlaySound������ҪWinmm�����

// ȫ�ֱ���
HANDLE g_hMutex;

// �������������ڹ���
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // �������һ���ں˶���
    HANDLE g_hMutex = CreateMutex(NULL, FALSE, TEXT("{FA531CC1-0497-11d3-A180-00105A276C3E}"));
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // �Ѿ���һ������ʵ����������
        MessageBox(NULL, TEXT("�Ѿ���һ������ʵ����������"), TEXT("��ʾ"), MB_OK);
        CloseHandle(g_hMutex);
        return 0;
    }

    // ����ĵ�һ��ʵ��
    // ��������ִ��

    WNDCLASSEX wndclass;                        // RegisterClassEx�����õ�WNDCLASSEX�ṹ
    TCHAR szClassName[] = TEXT("MyWindow");     // RegisterClassEx����ע��Ĵ����������
    TCHAR szAppName[] = TEXT("HelloWindows");   // ���ڱ���
    HWND hwnd;                                  // CreateWindowEx���������Ĵ��ڵľ��
    MSG msg;                                    // ��Ϣѭ�����õ���Ϣ�ṹ��

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WindowProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szClassName;
    wndclass.hIconSm = NULL;
    RegisterClassEx(&wndclass);

    hwnd = CreateWindowEx(0, szClassName, szAppName, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 180, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    TCHAR szStr[] = TEXT("��ã�Windows�������");

    switch (uMsg)
    {
    case WM_CREATE:
        PlaySound(TEXT("�ɶ�(�����).wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        TextOut(hdc, 10, 10, szStr, _tcslen(szStr));
        EndPaint(hwnd, &ps);
        return 0;

    case WM_DESTROY:
        // �����˹ر��ں˶�����
        CloseHandle(g_hMutex);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}