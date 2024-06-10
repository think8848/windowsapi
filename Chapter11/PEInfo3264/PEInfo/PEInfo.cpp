#include <windows.h>
#include <tchar.h>
#include "resource.h"

// ȫ�ֱ���
HWND g_hwndDlg;
LPCTSTR arrDataDirectory[] = { TEXT("������\t\t"), TEXT("�����\t\t"),
                               TEXT("��Դ��\t\t"), TEXT("�쳣��\t\t"),
                               TEXT("����֤���\t"), TEXT("�ض�λ��\t"),
                               TEXT("������Ϣ\t"), TEXT("��ƽ̨��ص�����"),
                               TEXT("ָ��ȫ��ָ��Ĵ�����ֵ"), TEXT("�ֲ߳̾��洢\t"),
                               TEXT("����������Ϣ��\t"), TEXT("�󶨵����\t"),
                               TEXT("���뺯����ַ��IAT"), TEXT("�ӳټ��ص����\t"),
                               TEXT("CLR����ʱͷ������"), TEXT("����\t\t") };

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ͨ��ָ����������(���絼����������ض�λ���)��RVA�õ�FOA
INT RVAToFOA(PIMAGE_DOS_HEADER pImageDosHeader, DWORD dwTargetRVA);

// ͨ��һ��RVAֵ��ȡ���ڽ���������
LPSTR GetSectionNameByRVA(PIMAGE_DOS_HEADER pImageDosHeader, DWORD dwRVA);

// ͨ��IMAGE_FILE_HEADER.TimeDateStamp�ֶλ�ȡ����ʱ���ʽ�ַ���
LPTSTR GetDateTime(DWORD dwTimeDateStamp);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   static HWND hwndEdit;

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
   PIMAGE_SECTION_HEADER pImageSectionHeader;
   TCHAR szSectionName[IMAGE_SIZEOF_SHORT_NAME + 1] = { 0 };  // Unicode��������
   TCHAR szBuf[256] = { 0 };

   switch (uMsg)
   {
   case WM_INITDIALOG:
      g_hwndDlg = hwndDlg;

      hwndEdit = GetDlgItem(hwndDlg, IDC_EDIT_INFO);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_BROWSE:
         if (GetOpenFileName(&ofn))
            SetDlgItemText(hwndDlg, IDC_EDIT_PATH, szFile);
         break;

      case IDC_BTN_GET:
         // ��һ��PE�ļ�
         GetDlgItemText(g_hwndDlg, IDC_EDIT_PATH, szFile, _countof(szFile));
         hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, NULL);
         if (hFile == INVALID_HANDLE_VALUE)
         {
            MessageBox(g_hwndDlg, TEXT("CreateFile��������ʧ��"), TEXT("��ʾ"), MB_OK);
            return TRUE;
         }
         else
         {
            GetFileSizeEx(hFile, &liFileSize);
            if (liFileSize.QuadPart == 0)
            {
               MessageBox(g_hwndDlg, TEXT("�ļ���СΪ0"), TEXT("��ʾ"), MB_OK);
               return TRUE;
            }
         }

         // ΪhFile�ļ����󴴽�һ���ļ�ӳ���ں˶���
         hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
         if (!hFileMap)
         {
            MessageBox(g_hwndDlg, TEXT("CreateFileMapping����ʧ��"), TEXT("��ʾ"), MB_OK);
            return TRUE;
         }

         // ���ļ�ӳ�����hFileMap��ȫ��ӳ�䵽���̵������ַ�ռ���
         lpMemory = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
         if (!lpMemory)
         {
            MessageBox(g_hwndDlg, TEXT("MapViewOfFile����ʧ��"), TEXT("��ʾ"), MB_OK);
            return TRUE;
         }

         // �򿪵��ļ��ǲ���PE�ļ�
         pImageDosHeader = (PIMAGE_DOS_HEADER)lpMemory;
         if (pImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
         {
            MessageBox(g_hwndDlg, TEXT("�򿪵Ĳ���PE�ļ�"), TEXT("��ʾ"), MB_OK);
            return TRUE;
         }

         pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);
         if (pImageNtHeader->Signature != IMAGE_NT_SIGNATURE)
         {
            MessageBox(g_hwndDlg, TEXT("�򿪵Ĳ���PE�ļ�"), TEXT("��ʾ"), MB_OK);
            return TRUE;
         }

         // ��ձ༭�ؼ�
         SetDlgItemText(hwndDlg, IDC_EDIT_INFO, TEXT(""));

         // ������Ϣ
         // �����PE32+���pImageNtHeaderǿ��ת��ΪPIMAGE_NT_HEADERS64
         if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
         {
            wsprintf(szBuf, TEXT("����ƽ̨��\t0x%04X\r\n����������\t0x%04X\r\n����ʱ�䣺\t%s\r\n�ļ����ԣ�\t0x%04X\r\n�ļ���ʽ��\t0x%04X\r\n����װ�ص�ַ��\t0x%016I64X\r\n��ڵ�ַ��\t0x%08X\r\n�ڴ���룺\t0x%08X\r\n�ļ����룺\t0x%08X\r\n�ڴ�ӳ���С��\t0x%08X\r\nУ��ͣ�\t0x%08X\r\n����Ŀ¼������\t0x%08X\r\n\r\n"),
               ((PIMAGE_NT_HEADERS64)pImageNtHeader)->FileHeader.Machine,
               ((PIMAGE_NT_HEADERS64)pImageNtHeader)->FileHeader.NumberOfSections,
               GetDateTime(((PIMAGE_NT_HEADERS64)pImageNtHeader)->FileHeader.TimeDateStamp),
               ((PIMAGE_NT_HEADERS64)pImageNtHeader)->FileHeader.Characteristics,
               ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.Magic,
               ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.ImageBase,
               ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.AddressOfEntryPoint,
               ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.SectionAlignment,
               ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.FileAlignment,
               ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.SizeOfImage,
               ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.CheckSum,
               ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.NumberOfRvaAndSizes);
         }
         // �����PE���pImageNtHeaderǿ��ת��ΪPIMAGE_NT_HEADERS32
         else
         {
            wsprintf(szBuf, TEXT("����ƽ̨��\t0x%04X\r\n����������\t0x%04X\r\n����ʱ�䣺\t%s\r\n�ļ����ԣ�\t0x%04X\r\n�ļ���ʽ��\t0x%04X\r\n����װ�ص�ַ��\t0x%08X\r\n��ڵ�ַ��\t0x%08X\r\n�ڴ���룺\t0x%08X\r\n�ļ����룺\t0x%08X\r\n�ڴ�ӳ���С��\t0x%08X\r\nУ��ͣ�\t0x%08X\r\n����Ŀ¼������\t0x%08X\r\n\r\n"),
               ((PIMAGE_NT_HEADERS32)pImageNtHeader)->FileHeader.Machine,
               ((PIMAGE_NT_HEADERS32)pImageNtHeader)->FileHeader.NumberOfSections,
               GetDateTime(((PIMAGE_NT_HEADERS32)pImageNtHeader)->FileHeader.TimeDateStamp),
               ((PIMAGE_NT_HEADERS32)pImageNtHeader)->FileHeader.Characteristics,
               ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.Magic,
               ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.ImageBase,
               ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.AddressOfEntryPoint,
               ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.SectionAlignment,
               ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.FileAlignment,
               ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.SizeOfImage,
               ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.CheckSum,
               ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.NumberOfRvaAndSizes);
         }
         SendMessage(hwndEdit, EM_SETSEL, -1, -1);
         SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

         // ������Ϣ
         wsprintf(szBuf, TEXT("��������\t����  RVA\t���� FOA\tʵ�ʴ�С\t�����С\t��������\r\n"));
         SendMessage(hwndEdit, EM_SETSEL, -1, -1);
         SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

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
            // ��������
            MultiByteToWideChar(CP_UTF8, 0, (LPSTR)pImageSectionHeader, IMAGE_SIZEOF_SHORT_NAME,
               szSectionName, IMAGE_SIZEOF_SHORT_NAME);

            // ������Ϣ
            wsprintf(szBuf, TEXT("%s\t\t0x%08X\t0x%08X\t0x%08X\t0x%08X\t0x%08X\r\n"),
               szSectionName,
               pImageSectionHeader->VirtualAddress,
               pImageSectionHeader->PointerToRawData,
               pImageSectionHeader->Misc.VirtualSize,
               pImageSectionHeader->SizeOfRawData,
               pImageSectionHeader->Characteristics);
            SendMessage(hwndEdit, EM_SETSEL, -1, -1);
            SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

            // ָ����һ��������Ϣ�ṹ
            pImageSectionHeader++;
         }
         SendMessage(hwndEdit, EM_SETSEL, -1, -1);
         SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)TEXT("\r\n"));

         // �����������ݿ����Ϣ
         wsprintf(szBuf, TEXT("����\t\t����Ŀ¼\t\t���ݵ�RVA\t���ݵĴ�С\t���ݵ�FOA\t�����Ľ���\r\n"));
         SendMessage(hwndEdit, EM_SETSEL, -1, -1);
         SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

         // �����PE32+���pImageNtHeaderǿ��ת��ΪPIMAGE_NT_HEADERS64
         if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
         {
            for (DWORD i = 0; i < ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.NumberOfRvaAndSizes; i++)
            {
               if (((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[i].Size != 0)
               {
                  // �����Ľ�������
                  MultiByteToWideChar(CP_UTF8, 0,
                     GetSectionNameByRVA(pImageDosHeader, ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[i].VirtualAddress), IMAGE_SIZEOF_SHORT_NAME, szSectionName, IMAGE_SIZEOF_SHORT_NAME);

                  // ������Ϣ
                  wsprintf(szBuf, TEXT("%d\t\t%s\t0x%08X\t0x%08X\t0x%08X\t%s\r\n"),
                     i,
                     arrDataDirectory[i],
                     ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[i].VirtualAddress,
                     ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[i].Size,
                     RVAToFOA(pImageDosHeader, ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[i].VirtualAddress),
                     szSectionName);
                  SendMessage(hwndEdit, EM_SETSEL, -1, -1);
                  SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);
               }
            }
         }
         // �����PE���pImageNtHeaderǿ��ת��ΪPIMAGE_NT_HEADERS32
         else
         {
            for (DWORD i = 0; i < ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.NumberOfRvaAndSizes; i++)
            {
               if (((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[i].Size != 0)
               {
                  // �����Ľ�������
                  MultiByteToWideChar(CP_UTF8, 0,
                     GetSectionNameByRVA(pImageDosHeader, ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[i].VirtualAddress), IMAGE_SIZEOF_SHORT_NAME, szSectionName, IMAGE_SIZEOF_SHORT_NAME);

                  // ������Ϣ
                  wsprintf(szBuf, TEXT("%d\t\t%s\t0x%08X\t0x%08X\t0x%08X\t%s\r\n"),
                     i,
                     arrDataDirectory[i],
                     ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[i].VirtualAddress,
                     ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[i].Size,
                     RVAToFOA(pImageDosHeader, ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[i].VirtualAddress),
                     szSectionName);
                  SendMessage(hwndEdit, EM_SETSEL, -1, -1);
                  SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);
               }
            }
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

/*********************************************************************************
  * �������ܣ�ͨ��ָ����������(���絼����������ض�λ���)��RVA�õ�FOA
  * ���������˵����
    1. pImageDosHeader������ʾPE�ڴ�ӳ���ļ��������ڴ��е���ʼ��ַ������ָ��
    2. dwTargetRVA������ʾĿ���������ݵ�RVA������ָ��
  * ����ֵ��  ����-1��ʾ����ִ��ʧ��
**********************************************************************************/
INT RVAToFOA(PIMAGE_DOS_HEADER pImageDosHeader, DWORD dwTargetRVA)
{
   PIMAGE_NT_HEADERS pImageNtHeader;
   PIMAGE_SECTION_HEADER pImageSectionHeader;
   INT iTargetFOA = -1;

   // PE�ļ�ͷ�ĵ�ַ
   pImageNtHeader =
      (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);

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

/*********************************************************************************
  * �������ܣ�ͨ��һ��RVAֵ��ȡ���ڽ���������
  * ���������˵����
    1. pImageDosHeader������ʾPE�ڴ�ӳ���ļ��������ڴ��е���ʼ��ַ������ָ��
    2. dwRVA������ʾһ��RVAֵ������ָ��
  * ����ֵ��  ����NULL��ʾ����ִ��ʧ�ܣ���ע�ⷵ�صĽ��������ַ�������һ�������β
**********************************************************************************/
LPSTR GetSectionNameByRVA(PIMAGE_DOS_HEADER pImageDosHeader, DWORD dwRVA)
{
   LPSTR lpSectionName = NULL;
   PIMAGE_SECTION_HEADER pImageSectionHeader;

   // PE�ļ�ͷ�ĵ�ַ
   PIMAGE_NT_HEADERS pImageNtHeader =
      (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);

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
      if ((dwRVA >= pImageSectionHeader->VirtualAddress) &&
         (dwRVA <= (pImageSectionHeader->VirtualAddress + pImageSectionHeader->SizeOfRawData)))
      {
         lpSectionName = (LPSTR)pImageSectionHeader;
      }

      // ָ����һ��������Ϣ�ṹ
      pImageSectionHeader++;
   }

   return lpSectionName;
}

LPTSTR GetDateTime(DWORD dwTimeDateStamp)
{
   FILETIME ft, ftLocal;
   SYSTEMTIME st;
   ULARGE_INTEGER uli;
   LPTSTR pszDateTime = new TCHAR[64];

   st.wYear = 1970;
   st.wMonth = 1;
   st.wDay = 1;
   st.wHour = 0;
   st.wMinute = 0;
   st.wSecond = 0;
   st.wMilliseconds = 0;

   // ϵͳʱ��ת��Ϊ�ļ�ʱ��ſ��Լ����Ѿ���ȥ��ʱ��dwTimeDateStamp
   SystemTimeToFileTime(&st, &ft);

   // �ļ�ʱ�䵥λ��1/1000 0000�룬��1000���֮1��(100-nanosecond)
   // ��Ҫ��ָ��FILETIME�ṹ��ָ��ǿ��ת��ΪULARGE_INTEGER *��__int64 *ֵ
   // ��Ϊ����ܵ���64λWindows�ϵĶ������
   uli.HighPart = ft.dwHighDateTime;
   uli.LowPart = ft.dwLowDateTime;
   uli.QuadPart += (ULONGLONG)10000000 * dwTimeDateStamp;
   ft.dwHighDateTime = uli.HighPart;
   ft.dwLowDateTime = uli.LowPart;

   // ������ʱ��ת��Ϊ����ʱ��(�������������ļ�ʱ��)
   FileTimeToLocalFileTime(&ft, &ftLocal);
   // �ٽ��ļ�ʱ��ת��Ϊϵͳʱ��
   FileTimeToSystemTime(&ftLocal, &st);

   // ת��Ϊ����ʱ���ʽ�ַ���
   ZeroMemory(pszDateTime, 64);
   wsprintf(pszDateTime, TEXT("%d��%0.2d��%0.2d�� %0.2d:%0.2d:%0.2d"),
      st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wMinute);

   return pszDateTime;
}