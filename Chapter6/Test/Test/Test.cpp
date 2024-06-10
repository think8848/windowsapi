#include <Windows.h>
#include <strsafe.h>
#include <tchar.h>
#include "DllSample.h"                  // DllSample.hͷ�ļ�

#pragma comment(lib, "DllSample.lib")   // DllSample.lib������ļ�

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    TCHAR szBuf[256] = { 0 };
    TCHAR szBuf2[512] = { 0 };

    // ���Ե�������
    wsprintf(szBuf, TEXT("nValue = %d\nps.x = %d, ps.y = %d\n"), nValue, ps.x, ps.y);
    StringCchCopy(szBuf2, _countof(szBuf2), szBuf);

    // ���Ե�����
    CStudent student((LPTSTR)TEXT("����"), 40);
    wsprintf(szBuf, TEXT("������%s�� ���䣺%d\n"), student.GetName(), student.GetAge());
    StringCchCat(szBuf2, _countof(szBuf2), szBuf);

    // ���Ե�������
    wsprintf(szBuf, TEXT("funAdd(5, 6) = %d\nfunMul(5, 6) = %d"), funAdd(5, 6), funMul(5, 6));
    StringCchCat(szBuf2, _countof(szBuf2), szBuf);

    MessageBox(NULL, szBuf2, TEXT("��ʾ"), MB_OK);
}