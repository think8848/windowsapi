#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "resource.h"

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT RVAToFOA(PIMAGE_NT_HEADERS pImageNtHeader, DWORD dwTargetRVA);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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

   HANDLE hFile, hFileMap;
   LPVOID lpMemory;
   LARGE_INTEGER liFileSize;

   PIMAGE_DOS_HEADER pImageDosHeader;
   PIMAGE_NT_HEADERS pImageNtHeader;

   DWORD dwTargetRVA;
   INT iTargetFOA;
   TCHAR szBuf[32] = { 0 };
   //TCHAR szFOA[64] = TEXT("0x");

   switch (uMsg)
   {
   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_BROWSE:
         if (GetOpenFileName(&ofn))
            SetDlgItemText(hwndDlg, IDC_EDIT_PATH, szFile);
         break;

      case IDC_BTN_CONVERT:
         // ��һ��PE�ļ�
         GetDlgItemText(hwndDlg, IDC_EDIT_PATH, szFile, _countof(szFile));
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

         // �򿪵��ļ��ǲ���PE�ļ�
         pImageDosHeader = (PIMAGE_DOS_HEADER)lpMemory;
         if (pImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
         {
            MessageBox(hwndDlg, TEXT("�򿪵Ĳ���PE�ļ�"), TEXT("��ʾ"), MB_OK);
            return TRUE;
         }

         pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);
         if (pImageNtHeader->Signature != IMAGE_NT_SIGNATURE)
         {
            MessageBox(hwndDlg, TEXT("�򿪵Ĳ���PE�ļ�"), TEXT("��ʾ"), MB_OK);
            return TRUE;
         }

         // ��ȡRVA�༭�ؼ��е��ַ���
         GetDlgItemText(hwndDlg, IDC_EDIT_RVA, szBuf, _countof(szBuf));
         // ��ֵ��ʽ��RVA�ַ���ת��Ϊʮ��������ֵ��RVA
         swscanf_s(szBuf, TEXT("%X"), &dwTargetRVA);
         // RVAToFOA
         if ((iTargetFOA = RVAToFOA(pImageNtHeader, dwTargetRVA)) >= 0)
         {
            // FOA��ֵת��Ϊ�ַ���
            _itot_s(iTargetFOA, szBuf, _countof(szBuf), 16);
            // ת��Ϊ��д
            _tcsupr_s(szBuf, _tcslen(szBuf) + 1);
            // ǰ�����0x
            //_tcscat_s(szFOA, _countof(szFOA), szBuf);
            // ��FOA��ֵ��ʽ���ַ�����ʾ��FOA�༭�ؼ���
            SetDlgItemText(hwndDlg, IDC_EDIT_FOA, szBuf);
         }

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

INT RVAToFOA(PIMAGE_NT_HEADERS pImageNtHeader, DWORD dwTargetRVA)
{
   PIMAGE_SECTION_HEADER pImageSectionHeader;
   INT iTargetFOA = -1;

   // PE��PE32+�Ľڱ�λ��ͬ
   if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
      pImageSectionHeader =
      (PIMAGE_SECTION_HEADER)((LPBYTE)pImageNtHeader + sizeof(IMAGE_NT_HEADERS32));
   else
      pImageSectionHeader =
      (PIMAGE_SECTION_HEADER)((LPBYTE)pImageNtHeader + sizeof(IMAGE_NT_HEADERS64));

   // �����ڱ�
   for (int i = 0; i < pImageNtHeader->FileHeader.NumberOfSections; i++)
   {
      if ((dwTargetRVA >= pImageSectionHeader->VirtualAddress) &&
         (dwTargetRVA <= (pImageSectionHeader->VirtualAddress + pImageSectionHeader->SizeOfRawData)))
      {
         iTargetFOA = dwTargetRVA - pImageSectionHeader->VirtualAddress;
         iTargetFOA += pImageSectionHeader->PointerToRawData;
      }

      // ָ����һ��������Ϣ�ṹ
      pImageSectionHeader++;
   }

   return iTargetFOA;
}