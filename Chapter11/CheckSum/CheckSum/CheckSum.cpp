#include <windows.h>
#include <imagehlp.h>
#include "resource.h"

#pragma comment(lib, "Imagehlp.lib")

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

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

   HANDLE hFile;
   DWORD dwFileSize;
   LPVOID lpMemory;

   PIMAGE_DOS_HEADER pImageDosHeader;
   PIMAGE_NT_HEADERS pImageNtHeader;

   DWORD dwCheckSum = 0;
   DWORD dwCheckSumImageHlp = 0;
   DWORD dwCheckSumImageHlpOrigin = 0;
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
         // ��һ��PE�ļ�������ȡ�ļ���С
         GetDlgItemText(hwndDlg, IDC_EDIT_FILENAME, szFile, _countof(szFile));
         hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
         dwFileSize = GetFileSize(hFile, NULL);

         // �����ڴ棬����PE�ļ�
         lpMemory = VirtualAlloc(NULL, dwFileSize, MEM_COMMIT, PAGE_READWRITE);
         ReadFile(hFile, lpMemory, dwFileSize, NULL, NULL);
         CloseHandle(hFile);

         // �򿪵��ļ��ǲ���PE�ļ�
         pImageDosHeader = (PIMAGE_DOS_HEADER)lpMemory;
         if (pImageDosHeader->e_magic == IMAGE_DOS_SIGNATURE)
         {
            pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);
            if (pImageNtHeader->Signature == IMAGE_NT_SIGNATURE)
               // IMAGE_NT_HEADERS.OptionalHeader.CheckSum�ֶ���0
               pImageNtHeader->OptionalHeader.CheckSum = 0;
         }

         // ����У���
         DWORD i;
         for (i = 0; i < dwFileSize - 1; i += 2)
         {
            dwCheckSum += *((LPWORD)((LPBYTE)lpMemory + i));
            dwCheckSum = (dwCheckSum >> 16) + (dwCheckSum & 0xFFFF);
         }
         if (i == dwFileSize - 1)
            dwCheckSum += *((LPBYTE)lpMemory + i);
         dwCheckSum += dwFileSize;

         VirtualFree(lpMemory, 0, MEM_RELEASE);
         // ��ʾ�Զ����㷨����Ľ��
         wsprintf(szBuf, TEXT("0x%08X"), (DWORD)dwCheckSum);
         SetDlgItemText(hwndDlg, IDC_EDIT_CUSTOMFUNC, szBuf);

         MapFileAndCheckSum(szFile, &dwCheckSumImageHlpOrigin, &dwCheckSumImageHlp);
         // ��ʾMapFileAndCheckSum��������Ľ��
         wsprintf(szBuf, TEXT("0x%08X"), dwCheckSumImageHlp);
         SetDlgItemText(hwndDlg, IDC_EDIT_MAPFILEANDCHECKSUM, szBuf);
         break;

      case IDCANCEL:
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;
   }

   return FALSE;
}