#include <windows.h>
#include <tchar.h>
#include "resource.h"

// ȫ�ֱ���
HWND g_hwndDlg;                 // ���ھ��
HWND g_hwndEdit;                // ���б༭�ؼ����ھ��
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
INT RVAToFOA(PIMAGE_NT_HEADERS pImageNtHeader, DWORD dwTargetRVA);

// ͨ��һ��RVAֵ��ȡ���ڽ���������
LPSTR GetSectionNameByRVA(PIMAGE_NT_HEADERS pImageNtHeader, DWORD dwRVA);

// ͨ��IMAGE_FILE_HEADER.TimeDateStamp�ֶλ�ȡ����ʱ���ʽ�ַ���
LPTSTR GetDateTime(DWORD dwTimeDateStamp);

// ��ȡPE�ļ�������Ϣ��������Ϣ�����ݿ���Ϣ
BOOL GetBaseInfo(PIMAGE_NT_HEADERS pImageNtHeader);

// ��ȡ�����������dll�ĵ��뺯���ĺ�����źͺ�������
BOOL GetImportTable(PIMAGE_DOS_HEADER pImageDosHeader);

// ��ȡ�������е����е�������
BOOL GetExportTable(PIMAGE_DOS_HEADER pImageDosHeader);

// ��ȡ�ض�λ����������Ҫ�ض�λ�Ĳ��������Ե�ַ�ĵ�ַ(RVAֵ)
BOOL GetRelocationTable(PIMAGE_DOS_HEADER pImageDosHeader);

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

   // DOSͷָ���PE�ļ�ͷָ��
   PIMAGE_DOS_HEADER pImageDosHeader;
   PIMAGE_NT_HEADERS pImageNtHeader;

   // ���б༭�ؼ�����
   HFONT hFont;

   // �������б༭�ؼ����ڴ�С֮��
   static RECT rectWindow;
   RECT rectEdit;
   static int nWidthEdit, nHeightEdit;
   int cx, cy;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      g_hwndDlg = hwndDlg;

      g_hwndEdit = GetDlgItem(hwndDlg, IDC_EDIT_INFO);

      // ���ö��б༭�ؼ�Ϊ����
      hFont = CreateFont(12, 0, 0, 0, 0, 0, 0, 0, GB2312_CHARSET, 0, 0, 0, 0, TEXT("����"));
      SendMessage(g_hwndEdit, WM_SETFONT, (WPARAM)hFont, FALSE);

      // Ĭ������±༭�ؼ���󻺳�����СԼΪ32KB���ַ�����Ϊ���޴�С
      SendMessage(g_hwndEdit, EM_SETLIMITTEXT, 0, 0);

      // ������򴰿ڴ�С
      GetClientRect(hwndDlg, &rectWindow);
      // ������б༭�ؼ��Ŀ�ȸ߶�
      GetWindowRect(g_hwndEdit, &rectEdit);
      nWidthEdit = rectEdit.right - rectEdit.left;
      nHeightEdit = rectEdit.bottom - rectEdit.top;
      return TRUE;

   case WM_SIZE:
      cx = LOWORD(lParam) - rectWindow.right;
      cy = HIWORD(lParam) - rectWindow.bottom;
      SetWindowPos(g_hwndEdit, NULL, 0, 0, nWidthEdit + cx, nHeightEdit + cy, SWP_NOZORDER | SWP_NOMOVE);
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

         // ��ձ༭�ؼ�
         SetDlgItemText(hwndDlg, IDC_EDIT_INFO, TEXT(""));

         //PE�ļ�������Ϣ��������Ϣ�����ݿ���Ϣ
         GetBaseInfo(pImageNtHeader);

         //�������Ϣ
         GetImportTable(pImageDosHeader);

         //��������Ϣ
         GetExportTable(pImageDosHeader);

         // �ض�λ��
         GetRelocationTable(pImageDosHeader);

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

BOOL GetBaseInfo(PIMAGE_NT_HEADERS pImageNtHeader)
{
   PIMAGE_SECTION_HEADER pImageSectionHeader;
   TCHAR szSectionName[IMAGE_SIZEOF_SHORT_NAME + 1] = { 0 };  // Unicode��������
   TCHAR szBuf[256] = { 0 };

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
   SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
   SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

   // ������Ϣ
   wsprintf(szBuf, TEXT("��������\t����  RVA\t���� FOA\tʵ�ʴ�С\t�����С\t��������\r\n"));
   SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
   SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

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
      wsprintf(szBuf, TEXT("%-8s\t0x%08X\t0x%08X\t0x%08X\t0x%08X\t0x%08X\r\n"),
         szSectionName,
         pImageSectionHeader->VirtualAddress,
         pImageSectionHeader->PointerToRawData,
         pImageSectionHeader->Misc.VirtualSize,
         pImageSectionHeader->SizeOfRawData,
         pImageSectionHeader->Characteristics);
      SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
      SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

      // ָ����һ��������Ϣ�ṹ
      pImageSectionHeader++;
   }
   SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
   SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)TEXT("\r\n"));

   // �����������ݿ����Ϣ
   wsprintf(szBuf, TEXT("����\t\t����Ŀ¼\t\t���ݵ�RVA\t���ݵĴ�С\t���ݵ�FOA\t�����Ľ���\r\n"));
   SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
   SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

   // �����PE32+���pImageNtHeaderǿ��ת��ΪPIMAGE_NT_HEADERS64
   if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
   {
      for (DWORD i = 0; i < ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.NumberOfRvaAndSizes; i++)
      {
         if (((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[i].Size != 0)
         {
            // �����Ľ�������
            MultiByteToWideChar(CP_UTF8, 0,
               GetSectionNameByRVA(pImageNtHeader,
                  ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[i].VirtualAddress),
               IMAGE_SIZEOF_SHORT_NAME, szSectionName, IMAGE_SIZEOF_SHORT_NAME);

            // ������Ϣ
            wsprintf(szBuf, TEXT("%d\t\t%s\t0x%08X\t0x%08X\t0x%08X\t%s\r\n"),
               i,
               arrDataDirectory[i],
               ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[i].VirtualAddress,
               ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[i].Size,
               RVAToFOA(pImageNtHeader, ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[i].VirtualAddress),
               szSectionName);
            SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
            SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);
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
               GetSectionNameByRVA(pImageNtHeader,
                  ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[i].VirtualAddress),
               IMAGE_SIZEOF_SHORT_NAME, szSectionName, IMAGE_SIZEOF_SHORT_NAME);

            // ������Ϣ
            wsprintf(szBuf, TEXT("%d\t\t%s\t0x%08X\t0x%08X\t0x%08X\t%s\r\n"),
               i,
               arrDataDirectory[i],
               ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[i].VirtualAddress,
               ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[i].Size,
               RVAToFOA(pImageNtHeader, ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[i].VirtualAddress),
               szSectionName);
            SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
            SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);
         }
      }
   }

   return TRUE;
}

BOOL GetImportTable(PIMAGE_DOS_HEADER pImageDosHeader)
{
   PIMAGE_NT_HEADERS pImageNtHeader;               // PE�ļ�ͷ��ʼ��ַ
   PIMAGE_IMPORT_DESCRIPTOR pImageImportDescriptor;// �������ʼ��ַ
   PIMAGE_THUNK_DATA32 pImageThunkData32;          // IMAGE_THUNK_DATA32������ʼ��ַ
   PIMAGE_THUNK_DATA64 pImageThunkData64;          // IMAGE_THUNK_DATA64������ʼ��ַ
   PIMAGE_IMPORT_BY_NAME pImageImportByName;       // IMAGE_IMPORT_BY_NAME�ṹָ��
   TCHAR szDllName[128] = { 0 };                   // dll����
   TCHAR szFuncName[128] = { 0 };                  // ��������
   TCHAR szBuf[256] = { 0 };
   TCHAR szImportTableHead[] = TEXT("\r\n\r\n�������Ϣ��\r\ndll�ļ���\t\t\t\t\t�������\t��������\r\n");

   // PE�ļ�ͷ��ʼ��ַ
   pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);

   // �����PE32+���pImageNtHeaderǿ��ת��ΪPIMAGE_NT_HEADERS64
   if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
   {
      // �Ƿ��е����(��Ȼ��û�еĿ����Բ���)
      if (((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[1].Size == 0)
         return FALSE;

      // �������ʼ��ַ
      pImageImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[1].VirtualAddress));

      SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
      SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szImportTableHead);
      // ���������
      while (pImageImportDescriptor->OriginalFirstThunk ||
         pImageImportDescriptor->TimeDateStamp || pImageImportDescriptor->ForwarderChain ||
         pImageImportDescriptor->Name || pImageImportDescriptor->FirstThunk)
      {
         // dll����
         MultiByteToWideChar(CP_UTF8, 0, (LPSTR)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageImportDescriptor->Name)), -1, szDllName, _countof(szDllName));

         // IMAGE_THUNK_DATA64������ʼ��ַ
         pImageThunkData64 = (PIMAGE_THUNK_DATA64)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageImportDescriptor->FirstThunk));
         while (pImageThunkData64->u1.AddressOfData != 0)
         {
            // ����ŵ��뻹�ǰ��������Ƶ���
            // IMAGE_IMPORT_BY_NAME�ṹָ��
            pImageImportByName = (PIMAGE_IMPORT_BY_NAME)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageThunkData64->u1.AddressOfData));

            if (pImageThunkData64->u1.AddressOfData & IMAGE_ORDINAL_FLAG64)
            {
               wsprintf(szFuncName, TEXT("����� 0x%04X"), pImageThunkData64->u1.AddressOfData & 0xFFFF);
               wsprintf(szBuf, TEXT("%-48s%s\r\n"), szDllName, szFuncName);
            }
            else
            {
               MultiByteToWideChar(CP_UTF8, 0, pImageImportByName->Name, -1, szFuncName, _countof(szFuncName));
               wsprintf(szBuf, TEXT("%-48s0x%04X\t\t%s\r\n"), szDllName, pImageImportByName->Hint, szFuncName);
            }
            SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
            SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

            // ָ����һ��IMAGE_THUNK_DATA64�ṹ
            pImageThunkData64++;
         }

         SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
         SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)TEXT("\r\n"));
         // ָ����һ�������������
         pImageImportDescriptor++;
      }
   }
   // �����PE���pImageNtHeaderǿ��ת��ΪPIMAGE_NT_HEADERS32
   else
   {
      // �Ƿ��е����(��Ȼ��û�еĿ����Բ���)
      if (((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[1].Size == 0)
         return FALSE;

      // �������ʼ��ַ
      pImageImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[1].VirtualAddress));

      SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
      SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szImportTableHead);
      // ���������
      while (pImageImportDescriptor->OriginalFirstThunk ||
         pImageImportDescriptor->TimeDateStamp || pImageImportDescriptor->ForwarderChain ||
         pImageImportDescriptor->Name || pImageImportDescriptor->FirstThunk)
      {
         // dll����
         MultiByteToWideChar(CP_UTF8, 0, (LPSTR)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageImportDescriptor->Name)), -1, szDllName, _countof(szDllName));

         // IMAGE_THUNK_DATA32������ʼ��ַ
         pImageThunkData32 = (PIMAGE_THUNK_DATA32)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageImportDescriptor->FirstThunk));
         while (pImageThunkData32->u1.AddressOfData != 0)
         {
            // ����ŵ��뻹�ǰ��������Ƶ���
            // IMAGE_IMPORT_BY_NAME�ṹָ��
            pImageImportByName = (PIMAGE_IMPORT_BY_NAME)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageThunkData32->u1.AddressOfData));

            if (pImageThunkData32->u1.AddressOfData & IMAGE_ORDINAL_FLAG32)
            {
               wsprintf(szFuncName, TEXT("����� 0x%04X"), pImageThunkData32->u1.AddressOfData & 0xFFFF);
               wsprintf(szBuf, TEXT("%-48s%s\r\n"), szDllName, szFuncName);
            }
            else
            {
               MultiByteToWideChar(CP_UTF8, 0, pImageImportByName->Name, -1, szFuncName, _countof(szFuncName));
               wsprintf(szBuf, TEXT("%-48s0x%04X\t\t%s\r\n"), szDllName, pImageImportByName->Hint, szFuncName);
            }
            SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
            SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

            // ָ����һ��IMAGE_THUNK_DATA32�ṹ
            pImageThunkData32++;
         }

         SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
         SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)TEXT("\r\n"));
         // ָ����һ�������������
         pImageImportDescriptor++;
      }
   }

   return TRUE;
}

BOOL GetExportTable(PIMAGE_DOS_HEADER pImageDosHeader)
{
   PIMAGE_NT_HEADERS pImageNtHeader;                   // PE�ļ�ͷ��ʼ��ַ
   PIMAGE_EXPORT_DIRECTORY pImageExportDirectory;      // ������Ŀ¼�ṹ����ʼ��ַ
   PDWORD pAddressOfFunctions;                         // ����������ַ�����ʼ��ַ
   PWORD pAddressOfNameOrdinals;                       // �������������ʼ��ַ
   PDWORD pAddressOfNames;                             // �������Ƶ�ַ�����ʼ��ַ
   TCHAR szModuleName[128] = { 0 };                    // ģ���ԭʼ�ļ���
   TCHAR szFuncName[128] = { 0 };                      // ��������
   TCHAR szBuf[512] = { 0 };
   TCHAR szExportTableHead[] = TEXT("\r\n��������Ϣ��\r\n");
   TCHAR szExportTableFuncs[] = TEXT("��������\t������ַ\t��������\r\n");

   // PE�ļ�ͷ��ʼ��ַ
   pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);

   // PE��PE32+�ĵ�����Ŀ¼�ṹ��λ��ͬ
   if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
   {
      // �Ƿ��е�����
      if (((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[0].Size == 0)
         return FALSE;
      pImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[0].VirtualAddress));
   }
   else
   {
      // �Ƿ��е�����
      if (((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[0].Size == 0)
         return FALSE;
      pImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[0].VirtualAddress));
   }
   // ����������ַ�����ʼ��ַ
   pAddressOfFunctions = (PDWORD)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageExportDirectory->AddressOfFunctions));
   // �������������ʼ��ַ
   pAddressOfNameOrdinals = (PWORD)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageExportDirectory->AddressOfNameOrdinals));
   // �������Ƶ�ַ�����ʼ��ַ
   pAddressOfNames = (PDWORD)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageExportDirectory->AddressOfNames));

   SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
   SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szExportTableHead);
   // �����������Ϣ
   MultiByteToWideChar(CP_UTF8, 0, (LPSTR)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageExportDirectory->Name)), -1, szModuleName, _countof(szModuleName));
   wsprintf(szBuf, TEXT("ģ��ԭʼ�ļ���\t\t%s\r\n������������ʼ����\t0x%08X\r\n�����������ܸ���\t0x%08X\r\n�����Ƶ��������ĸ���\t0x%08X\r\n����������ַ���RVA\t0x%08X\r\n�������Ƶ�ַ���RVA\t0x%08X\r\nָ�����������RVA\t0x%08X\r\n\r\n"),
      szModuleName,
      pImageExportDirectory->Base,
      pImageExportDirectory->NumberOfFunctions,
      pImageExportDirectory->NumberOfNames,
      pImageExportDirectory->AddressOfFunctions,
      pImageExportDirectory->AddressOfNames,
      pImageExportDirectory->AddressOfNameOrdinals);
   SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
   SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

   SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
   SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szExportTableFuncs);
   // �����������е����е�������
   for (DWORD i = 0; i < pImageExportDirectory->NumberOfFunctions; i++)
   {
      // �Ƿ��ǰ��������Ƶ�������������������
      DWORD j;
      for (j = 0; j < pImageExportDirectory->NumberOfNames; j++)
      {
         if (i == pAddressOfNameOrdinals[j])
         {
            // ��ȡ��������
            MultiByteToWideChar(CP_UTF8, 0, (LPSTR)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pAddressOfNames[j])), -1, szFuncName, _countof(szFuncName));
            break;
         }
      }
      // ��������꺯��������Ҳû�ҵ�����i�����ǰ�������������
      if (j == pImageExportDirectory->NumberOfNames)
         wsprintf(szFuncName, TEXT("����������"));

      if (pAddressOfFunctions[i])
      {
         wsprintf(szBuf, TEXT("0x%08X\t0x%08X\t%s\r\n"),
            pImageExportDirectory->Base + i, pAddressOfFunctions[i], szFuncName);
         SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
         SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);
      }
   }

   return TRUE;
}

BOOL GetRelocationTable(PIMAGE_DOS_HEADER pImageDosHeader)
{
   PIMAGE_NT_HEADERS pImageNtHeader;                   // PE�ļ�ͷ��ʼ��ַ
   PIMAGE_BASE_RELOCATION pImageBaseRelocation;        // �ض�λ�����ʼ��ַ
   PWORD pRelocationItem;                              // �ض�λ���������ʼ��ַ
   DWORD dwRelocationItem;                             // �ض�λ��ĸ���
   TCHAR szBuf[64] = { 0 };
   TCHAR szRelocationTableHead[] = TEXT("\r\n�ض�λ����Ϣ��\r\n");
   TCHAR szRelocationItemInfo[] = TEXT("����\t�ض�λ��ַ\t����\t�ض�λ��ַ\t����\t�ض�λ��ַ\t����\t�ض�λ��ַ\t");

   // PE�ļ�ͷ��ʼ��ַ
   pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);

   // PE��PE32+���ض�λ��Ķ�λ��ͬ
   if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
   {
      // �Ƿ����ض�λ��
      if (((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[5].Size == 0)
         return FALSE;
      pImageBaseRelocation = (PIMAGE_BASE_RELOCATION)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[5].VirtualAddress));
   }
   else
   {
      // �Ƿ����ض�λ��
      if (((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[5].Size == 0)
         return FALSE;
      pImageBaseRelocation = (PIMAGE_BASE_RELOCATION)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[5].VirtualAddress));
   }

   SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
   SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szRelocationTableHead);
   SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
   SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szRelocationItemInfo);

   // �����ض�λ��
   while (pImageBaseRelocation->VirtualAddress != 0)
   {
      // �ض�λ���������ʼ��ַ
      pRelocationItem = (PWORD)((LPBYTE)pImageBaseRelocation + sizeof(IMAGE_BASE_RELOCATION));
      // �ض�λ��ĸ���
      dwRelocationItem = (pImageBaseRelocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);

      for (DWORD i = 0; i < dwRelocationItem; i++)
      {
         wsprintf(szBuf, TEXT("0x%X\t0x%08X\t"), pRelocationItem[i] >> 12, pImageBaseRelocation->VirtualAddress + (pRelocationItem[i] & 0x0FFF));
         // 4��һ��
         if (i % 4 == 0)
         {
            SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
            SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)TEXT("\r\n"));
         }
         SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
         SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);
      }
      // ҳ��ҳ֮���һ��
      SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
      SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)TEXT("\r\n"));

      // ָ����һ���ض�λ��ṹ
      pImageBaseRelocation = (PIMAGE_BASE_RELOCATION)((LPBYTE)pImageBaseRelocation + pImageBaseRelocation->SizeOfBlock);
   }

   return TRUE;
}

/*********************************************************************************
  * �������ܣ�ͨ��ָ����������(���絼����������ض�λ���)��RVA�õ�FOA
  * ���������˵����
    1. pImageNtHeader������ʾPE�ڴ�ӳ���ļ�������PE�ļ�ͷ����ʼ��ַ������ָ��
    2. dwTargetRVA������ʾĿ���������ݵ�RVA������ָ��
  * ����ֵ��  ����-1��ʾ����ִ��ʧ��
**********************************************************************************/
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

/*********************************************************************************
  * �������ܣ�ͨ��һ��RVAֵ��ȡ���ڽ���������
  * ���������˵����
    1. pImageNtHeader������ʾPE�ڴ�ӳ���ļ�������PE�ļ�ͷ����ʼ��ַ������ָ��
    2. dwRVA������ʾһ��RVAֵ������ָ��
  * ����ֵ��  ����NULL��ʾ����ִ��ʧ�ܣ���ע�ⷵ�صĽ��������ַ�������һ�������β
**********************************************************************************/
LPSTR GetSectionNameByRVA(PIMAGE_NT_HEADERS pImageNtHeader, DWORD dwRVA)
{
   LPSTR lpSectionName = NULL;
   PIMAGE_SECTION_HEADER pImageSectionHeader;

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