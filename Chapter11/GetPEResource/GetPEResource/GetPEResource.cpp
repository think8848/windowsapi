#include <windows.h>
#include <CommCtrl.h>
#include <strsafe.h>
#include "resource.h"

// ȫ�ֱ���
HWND g_hwndDlg;                 // ���ھ��
HWND g_hwndTree;                // ����ͼ�ؼ����ھ��
HWND g_hwndEdit;                // ���б༭�ؼ����ھ��

LPCTSTR arrResType[] = { TEXT("δ֪����"), TEXT("���"), TEXT("λͼ"), TEXT("ͼ��"),
    TEXT("�˵�"), TEXT("�Ի���"), TEXT("�ַ�����"), TEXT("����Ŀ¼"), TEXT("����"),
    TEXT("���ټ�"), TEXT("�����Զ�����Դ"), TEXT("��Ϣ��"), TEXT("�����"), TEXT("δ֪����"),
    TEXT("ͼ����"), TEXT("δ֪����"), TEXT("����汾"), TEXT("�ṩ�������Ƶ�ͷ�ļ�"),
    TEXT("δ֪����"), TEXT("���弴����Դ"), TEXT("VXD"), TEXT("��̬���"), TEXT("��̬ͼ��"),
    TEXT("HTML"), TEXT("�嵥�ļ�") };

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ͨ��ָ���������ݵ�RVA�õ�FOA
INT RVAToFOA(PIMAGE_NT_HEADERS pImageNtHeader, DWORD dwTargetRVA);

// ��ȡ��Դ��Ϣ
BOOL GetResourceInfo(PIMAGE_RESOURCE_DIRECTORY pImageRes, PIMAGE_RESOURCE_DIRECTORY pImageResDir,
   HTREEITEM hTreeParent, DWORD dwLevel);

// ��ȡѡ����Դ����Ϣ
BOOL GetSelectedResData(PIMAGE_DOS_HEADER pImageDosHeader, LPBYTE lpResData, DWORD dwResSize);

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
   static HANDLE hFile, hFileMap;
   static LPVOID lpMemory;
   LARGE_INTEGER liFileSize;

   // DOSͷָ���PE�ļ�ͷָ��
   static PIMAGE_DOS_HEADER pImageDosHeader;
   static PIMAGE_NT_HEADERS pImageNtHeader;

   // ��1��Ŀ¼�е���ԴĿ¼�ṹ��ʼ��ַ��Ҳ������Դ�����ʼ��ַ
   PIMAGE_RESOURCE_DIRECTORY pImageRes;

   // ���б༭�ؼ�����
   HFONT hFont;

   // ��������ͼ�ؼ��Ͷ��б༭�ؼ����ڴ�С֮��
   static RECT rectWindow;
   RECT rectTree, rectEdit;
   static int nWidthTree, nHeightTree, nWidthEdit, nHeightEdit;
   int cx, cy;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      g_hwndDlg = hwndDlg;
      g_hwndTree = GetDlgItem(hwndDlg, IDC_TREE_RESOURCE);
      g_hwndEdit = GetDlgItem(hwndDlg, IDC_EDIT_RESOURCE);

      // ���ö��б༭�ؼ�Ϊ����
      hFont = CreateFont(12, 0, 0, 0, 0, 0, 0, 0, GB2312_CHARSET, 0, 0, 0, 0, TEXT("����"));
      SendMessage(g_hwndEdit, WM_SETFONT, (WPARAM)hFont, FALSE);
      DeleteObject(hFont);

      // Ĭ������±༭�ؼ���󻺳�����СԼΪ32KB���ַ�����Ϊ���޴�С
      SendMessage(g_hwndEdit, EM_SETLIMITTEXT, 0, 0);

      // ������򴰿ڴ�С
      GetClientRect(hwndDlg, &rectWindow);
      // ��������ͼ�ؼ��Ͷ��б༭�ؼ��Ŀ�ȸ߶�
      GetWindowRect(g_hwndTree, &rectTree);
      nWidthTree = rectTree.right - rectTree.left;
      nHeightTree = rectTree.bottom - rectTree.top;
      GetWindowRect(g_hwndEdit, &rectEdit);
      nWidthEdit = rectEdit.right - rectEdit.left;
      nHeightEdit = rectEdit.bottom - rectEdit.top;
      return TRUE;

   case WM_SIZE:
      cx = LOWORD(lParam) - rectWindow.right;
      cy = HIWORD(lParam) - rectWindow.bottom;
      SetWindowPos(g_hwndTree, NULL, 0, 0, nWidthTree, nHeightEdit + cy, SWP_NOZORDER | SWP_NOMOVE);
      SetWindowPos(g_hwndEdit, NULL, 0, 0, nWidthEdit + cx, nHeightEdit + cy, SWP_NOZORDER | SWP_NOMOVE);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_BROWSE:
         if (GetOpenFileName(&ofn))
            SetDlgItemText(hwndDlg, IDC_EDIT_FILENAME, szFile);
         break;

      case IDC_BTN_GET:
         // ��һ��PE�ļ�
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

         // ��1��Ŀ¼�е���ԴĿ¼�ṹ��ʼ��ַ������PE��PE32+
         if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
            pImageRes = (PIMAGE_RESOURCE_DIRECTORY)((LPBYTE)pImageDosHeader +
               RVAToFOA(pImageNtHeader, ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[2].VirtualAddress));
         else
            pImageRes = (PIMAGE_RESOURCE_DIRECTORY)((LPBYTE)pImageDosHeader +
               RVAToFOA(pImageNtHeader, ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[2].VirtualAddress));

         // ��ձ༭�ؼ�
         SetDlgItemText(hwndDlg, IDC_EDIT_RESOURCE, TEXT(""));

         // ��ȡ��Դ��Ϣ
         GetResourceInfo(pImageRes, pImageRes, TVI_ROOT, 1);
         break;

      case IDCANCEL:
         // ������
         UnmapViewOfFile(lpMemory);
         CloseHandle(hFileMap);
         CloseHandle(hFile);
         EndDialog(hwndDlg, 0);
         break;
      }
      return TRUE;

   case WM_NOTIFY:
      switch (((LPNMHDR)lParam)->code)
      {
      case TVN_SELCHANGED:
         LPNMTREEVIEW lpnmTreeView;
         LPBYTE lpResData;       // ��ѡ����Դ�����Դ������ʼ��ַ
         DWORD dwResSize;        // ��ѡ����Դ�����Դ���ݴ�С

         lpnmTreeView = (LPNMTREEVIEW)lParam;
         if (lpnmTreeView->itemNew.lParam != 0)
         {
            lpResData = (LPBYTE)pImageDosHeader +
               RVAToFOA(pImageNtHeader, ((PIMAGE_RESOURCE_DATA_ENTRY)(lpnmTreeView->itemNew.lParam))->OffsetToData);
            dwResSize = ((PIMAGE_RESOURCE_DATA_ENTRY)(lpnmTreeView->itemNew.lParam))->Size;
            GetSelectedResData(pImageDosHeader, lpResData, dwResSize);
         }
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

/*********************************************************************************
  * �������ܣ�		��ȡ��Դ��Ϣ
  * ���������˵����
    1. pImageRes������ʾ��1��Ŀ¼�е���ԴĿ¼�ṹ��ʼ��ַ��Ҳ������Դ�����ʼ��ַ������ָ��
    2. pImageResDir������ʾ��1��3��Ŀ¼�е���ԴĿ¼�ṹ��ʼ��ַ������ָ��
    3. hTreeParent������ʾ����ͼ�ؼ��и��ڵ�ľ��������ָ��
    4. dwLevel����ָ��Ϊ��ֵ1��3����ʾ��ǰ���ñ�������Ϊ�˻�ȡ�ڼ���Ŀ¼����Ϣ������ָ��
  * �ú���Ϊ�ݹ麯��
**********************************************************************************/
BOOL GetResourceInfo(PIMAGE_RESOURCE_DIRECTORY pImageRes, PIMAGE_RESOURCE_DIRECTORY pImageResDir,
   HTREEITEM hTreeParent, DWORD dwLevel)
{
   PIMAGE_RESOURCE_DIRECTORY pImageResDirSub;          // ��һ����ԴĿ¼�ṹ��ʼ��ַ
   PIMAGE_RESOURCE_DIRECTORY_ENTRY pImageResDirEntry;  // ��ԴĿ¼��ڽṹ������ʼ��ַ
   WORD wNums;                                         // ��ԴĿ¼��ڽṹ�������
   PIMAGE_RESOURCE_DATA_ENTRY pImageResDataEntry;      // ��Դ������ڽṹ��ʼ��ַ
   PIMAGE_RESOURCE_DIR_STRING_U pString;
   HTREEITEM hTree;
   TVINSERTSTRUCT tvi = { 0 };
   TCHAR szResType[128] = { 0 }, szResID[128] = { 0 }, szLanguageID[128] = { 0 };
   TCHAR szBuf[256] = { 0 };

   // ��ԴĿ¼��ڽṹ������ʼ��ַ
   pImageResDirEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((LPBYTE)pImageResDir +
      sizeof(IMAGE_RESOURCE_DIRECTORY));
   // ��ԴĿ¼��ڽṹ�������
   wNums = pImageResDir->NumberOfNamedEntries + pImageResDir->NumberOfIdEntries;

   tvi.item.mask = TVIF_TEXT;
   tvi.hInsertAfter = TVI_LAST;
   tvi.hParent = hTreeParent;

   if (dwLevel == 1)
   {
      // ������ԴĿ¼��ڽṹ����
      for (WORD i = 0; i < wNums; i++)
      {
         // ��Դ����
         if (pImageResDirEntry[i].Name & 0x80000000)
         {
            pString = (PIMAGE_RESOURCE_DIR_STRING_U)
               ((LPBYTE)pImageRes + (pImageResDirEntry[i].Name & 0x7FFFFFFF));
            StringCchCopy(szResType, pString->Length + 1, pString->NameString);
         }
         else
         {
            if (LOWORD(pImageResDirEntry[i].Name) <= 24)
               wsprintf(szResType, TEXT("%s"), arrResType[LOWORD(pImageResDirEntry[i].Name)]);
            else
               wsprintf(szResType, TEXT("%d(�Զ���ID)"), LOWORD(pImageResDirEntry[i].Name));
         }
         tvi.item.pszText = szResType;
         hTree = (HTREEITEM)SendMessage(g_hwndTree, TVM_INSERTITEM, 0, (LPARAM)&tvi);

         // �ݹ�����2��
         pImageResDirSub = (PIMAGE_RESOURCE_DIRECTORY)
            ((LPBYTE)pImageRes + (pImageResDirEntry[i].OffsetToData & 0x7FFFFFFF));
         GetResourceInfo(pImageRes, pImageResDirSub, hTree, 2);
      }
   }
   else if (dwLevel == 2)
   {
      // ������ԴĿ¼��ڽṹ����
      for (WORD i = 0; i < wNums; i++)
      {
         // ��ԴID
         if (pImageResDirEntry[i].Name & 0x80000000)
         {
            pString = (PIMAGE_RESOURCE_DIR_STRING_U)
               ((LPBYTE)pImageRes + (pImageResDirEntry[i].Name & 0x7FFFFFFF));
            StringCchCopy(szResID, pString->Length + 1, pString->NameString);
         }
         else
         {
            wsprintf(szResID, TEXT("%d"), LOWORD(pImageResDirEntry[i].Name));
         }
         tvi.item.pszText = szResID;
         hTree = (HTREEITEM)SendMessage(g_hwndTree, TVM_INSERTITEM, 0, (LPARAM)&tvi);

         // �ݹ�����3��
         pImageResDirSub = (PIMAGE_RESOURCE_DIRECTORY)
            ((LPBYTE)pImageRes + (pImageResDirEntry[i].OffsetToData & 0x7FFFFFFF));
         GetResourceInfo(pImageRes, pImageResDirSub, hTree, 3);
      }
   }
   else
   {
      // ������ԴĿ¼��ڽṹ����
      for (WORD i = 0; i < wNums; i++)
      {
         // ����ID
         if (pImageResDirEntry[i].Name & 0x80000000)
         {
            pString = (PIMAGE_RESOURCE_DIR_STRING_U)
               ((LPBYTE)pImageRes + (pImageResDirEntry[i].Name & 0x7FFFFFFF));
            StringCchCopy(szLanguageID, pString->Length + 1, pString->NameString);
         }
         else
         {
            wsprintf(szLanguageID, TEXT("0x%04X"), LOWORD(pImageResDirEntry[i].Name));
         }

         // ��Դ������ڽṹ��ʼ��ַ
         pImageResDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)
            ((LPBYTE)pImageRes + (pImageResDirEntry[i].OffsetToData));
         wsprintf(szBuf, TEXT("%s  RVA��0x%08X ��С��0x%X"),
            szLanguageID, pImageResDataEntry->OffsetToData, pImageResDataEntry->Size);

         tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
         tvi.item.pszText = szBuf;
         tvi.item.lParam = (LPARAM)pImageResDataEntry;// ������Դ������ڽṹ��ʼ��ַ����Ŀ����
         SendMessage(g_hwndTree, TVM_INSERTITEM, 0, (LPARAM)&tvi);

         // �ݹ����
         return TRUE;
      }
   }

   return TRUE;
}

BOOL GetSelectedResData(PIMAGE_DOS_HEADER pImageDosHeader, LPBYTE lpResData, DWORD dwResSize)
{
   DWORD dwFileOffset;             // �ļ�ƫ��

   dwFileOffset = lpResData - (LPBYTE)pImageDosHeader;



   return TRUE;
}