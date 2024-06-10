#include <windows.h>
#include "resource.h"

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ��������
//#define Poly 0xEDB88320L                // CRC-32��׼
// ����CRC-32��ѯ��
VOID GenerateCRC32Table(PUINT pCRC32Table);
// ����CRC-32
UINT CRC32(LPBYTE lpData, UINT nSize);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
    return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // ���ļ����ñ���
    TCHAR szFile[MAX_PATH] = { 0 };
    OPENFILENAME ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndDlg;
    ofn.lpstrFilter =
        TEXT("exe�ļ�(*.exe)\0*.exe\0dll�ļ�(*.dll)\0*.dll\0All(*.*)\0*.*\0");
    ofn.nFilterIndex = 3;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = NULL;
    ofn.nMaxFile = _countof(szFile);
    ofn.lpstrTitle = TEXT("��ѡ��Ҫ�򿪵�PE�ļ�");
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // �ڴ�ӳ���ļ����ñ���
    HANDLE hFile, hFileMap;
    LPVOID lpMemory;
    LARGE_INTEGER liFileSize;

    UINT nCRC;
    TCHAR szBuf[16] = { 0 };

    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_BROWSE:
            if (GetOpenFileName(&ofn))
                SetDlgItemText(hwndDlg, IDC_EDIT_FILENAME, szFile);
            break;

        case IDC_BTN_CALC:
            // ���ļ�
            GetDlgItemText(hwndDlg, IDC_EDIT_FILENAME, szFile, _countof(szFile));
            hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE)
            {
                MessageBox(hwndDlg, TEXT("CreateFile��������ʧ��"), TEXT("��ʾ"), MB_OK);
                return TRUE;
            }
            else
            {
                GetFileSizeEx(hFile, &liFileSize);
                if (liFileSize.QuadPart == 0)
                {
                    MessageBox(hwndDlg, TEXT("�ļ���СΪ0"), TEXT("��ʾ"), MB_OK);
                    return TRUE;
                }
            }

            // ΪhFile�ļ����󴴽�һ���ļ�ӳ���ں˶���
            hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
            if (!hFileMap)
            {
                MessageBox(hwndDlg, TEXT("CreateFileMapping����ʧ��"), TEXT("��ʾ"), MB_OK);
                return TRUE;
            }

            // ���ļ�ӳ�����hFileMap��ȫ��ӳ�䵽���̵������ַ�ռ���
            lpMemory = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
            if (!lpMemory)
            {
                MessageBox(hwndDlg, TEXT("MapViewOfFile����ʧ��"), TEXT("��ʾ"), MB_OK);
                return TRUE;
            }

            // �Զ��庯������CRC-32
            nCRC = CRC32((LPBYTE)lpMemory, liFileSize.LowPart);
            wsprintf(szBuf, TEXT("0x%08X"), nCRC);
            SetDlgItemText(hwndDlg, IDC_EDIT_RESULT, szBuf);

            // RtlComputeCrc32����CRC-32
            typedef UINT(WINAPI* pfnRtlComputeCrc32)(INT dwInitial, LPVOID lpData, INT nLen);
            pfnRtlComputeCrc32 fnRtlComputeCrc32;
            fnRtlComputeCrc32 = (pfnRtlComputeCrc32)
                GetProcAddress(GetModuleHandle(TEXT("NtDll.dll")), "RtlComputeCrc32");
            nCRC = fnRtlComputeCrc32(0, lpMemory, liFileSize.LowPart);  // ��һ������ָ��Ϊ0
            wsprintf(szBuf, TEXT("0x%08X"), nCRC);
            SetDlgItemText(hwndDlg, IDC_EDIT_RESULT2, szBuf);

            // ������
            UnmapViewOfFile(lpMemory);
            CloseHandle(hFileMap);
            CloseHandle(hFile);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }
        return TRUE;
    }

    return FALSE;
}

#define Poly 0xEDB88320                 // CRC-32��׼

VOID GenerateCRC32Table(PUINT pCRC32Table)
{
    UINT nCrc;

    for (UINT i = 0; i < 256; i++)
    {
        nCrc = i;
        for (int j = 0; j < 8; j++)
        {
            if (nCrc & 0x00000001)
                nCrc = (nCrc >> 1) ^ Poly;
            else
                nCrc = nCrc >> 1;
        }

        pCRC32Table[i] = nCrc;
    }
}

UINT CRC32(LPBYTE lpData, UINT nSize)
{
    UINT CRC32Table[256] = { 0 };       // CRC-32��ѯ��
    UINT nCrc = 0xFFFFFFFF;

    // ����CRC-32��ѯ��
    GenerateCRC32Table(CRC32Table);

    // ����CRC-32
    for (UINT i = 0; i < nSize; i++)
        nCrc = CRC32Table[(nCrc ^ lpData[i]) & 0xFF] ^ (nCrc >> 8);

    return nCrc ^ 0xFFFFFFFF;           // ��λȡ��
}