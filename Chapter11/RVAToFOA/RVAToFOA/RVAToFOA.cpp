#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "resource.h"

// 函数声明
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
      TEXT("exe文件(*.exe)\0*.exe\0dll文件(*.dll)\0*.dll\0All(*.*)\0*.*\0");
   ofn.nFilterIndex = 3;
   ofn.lpstrFile = szFile;
   ofn.lpstrFile[0] = NULL;
   ofn.nMaxFile = _countof(szFile);
   ofn.lpstrTitle = TEXT("请选择要打开的PE文件");
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
         // 打开一个PE文件
         GetDlgItemText(hwndDlg, IDC_EDIT_PATH, szFile, _countof(szFile));
         hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, NULL);
         if (hFile == INVALID_HANDLE_VALUE)
         {
            MessageBox(hwndDlg, TEXT("CreateFile函数调用失败"), TEXT("提示"), MB_OK);
            return TRUE;
         }
         else
         {
            GetFileSizeEx(hFile, &liFileSize);
            if (liFileSize.QuadPart == 0)
            {
               MessageBox(hwndDlg, TEXT("文件大小为0"), TEXT("提示"), MB_OK);
               return TRUE;
            }
         }

         // 为hFile文件对象创建一个文件映射内核对象
         hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
         if (!hFileMap)
         {
            MessageBox(hwndDlg, TEXT("CreateFileMapping调用失败"), TEXT("提示"), MB_OK);
            return TRUE;
         }

         // 把文件映射对象hFileMap的全部映射到进程的虚拟地址空间中
         lpMemory = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
         if (!lpMemory)
         {
            MessageBox(hwndDlg, TEXT("MapViewOfFile调用失败"), TEXT("提示"), MB_OK);
            return TRUE;
         }

         // 打开的文件是不是PE文件
         pImageDosHeader = (PIMAGE_DOS_HEADER)lpMemory;
         if (pImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
         {
            MessageBox(hwndDlg, TEXT("打开的不是PE文件"), TEXT("提示"), MB_OK);
            return TRUE;
         }

         pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);
         if (pImageNtHeader->Signature != IMAGE_NT_SIGNATURE)
         {
            MessageBox(hwndDlg, TEXT("打开的不是PE文件"), TEXT("提示"), MB_OK);
            return TRUE;
         }

         // 获取RVA编辑控件中的字符串
         GetDlgItemText(hwndDlg, IDC_EDIT_RVA, szBuf, _countof(szBuf));
         // 数值形式的RVA字符串转换为十六进制数值的RVA
         swscanf_s(szBuf, TEXT("%X"), &dwTargetRVA);
         // RVAToFOA
         if ((iTargetFOA = RVAToFOA(pImageNtHeader, dwTargetRVA)) >= 0)
         {
            // FOA数值转换为字符串
            _itot_s(iTargetFOA, szBuf, _countof(szBuf), 16);
            // 转换为大写
            _tcsupr_s(szBuf, _tcslen(szBuf) + 1);
            // 前面添加0x
            //_tcscat_s(szFOA, _countof(szFOA), szBuf);
            // 把FOA数值形式的字符串显示到FOA编辑控件中
            SetDlgItemText(hwndDlg, IDC_EDIT_FOA, szBuf);
         }

         // 清理工作
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

   // PE和PE32+的节表定位不同
   if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
      pImageSectionHeader =
      (PIMAGE_SECTION_HEADER)((LPBYTE)pImageNtHeader + sizeof(IMAGE_NT_HEADERS32));
   else
      pImageSectionHeader =
      (PIMAGE_SECTION_HEADER)((LPBYTE)pImageNtHeader + sizeof(IMAGE_NT_HEADERS64));

   // 遍历节表
   for (int i = 0; i < pImageNtHeader->FileHeader.NumberOfSections; i++)
   {
      if ((dwTargetRVA >= pImageSectionHeader->VirtualAddress) &&
         (dwTargetRVA <= (pImageSectionHeader->VirtualAddress + pImageSectionHeader->SizeOfRawData)))
      {
         iTargetFOA = dwTargetRVA - pImageSectionHeader->VirtualAddress;
         iTargetFOA += pImageSectionHeader->PointerToRawData;
      }

      // 指向下一个节区信息结构
      pImageSectionHeader++;
   }

   return iTargetFOA;
}