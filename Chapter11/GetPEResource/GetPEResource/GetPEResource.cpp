#include <windows.h>
#include <CommCtrl.h>
#include <strsafe.h>
#include "resource.h"

// 全局变量
HWND g_hwndDlg;                 // 窗口句柄
HWND g_hwndTree;                // 树视图控件窗口句柄
HWND g_hwndEdit;                // 多行编辑控件窗口句柄

LPCTSTR arrResType[] = { TEXT("未知类型"), TEXT("光标"), TEXT("位图"), TEXT("图标"),
    TEXT("菜单"), TEXT("对话框"), TEXT("字符串表"), TEXT("字体目录"), TEXT("字体"),
    TEXT("加速键"), TEXT("程序自定义资源"), TEXT("消息表"), TEXT("光标组"), TEXT("未知类型"),
    TEXT("图标组"), TEXT("未知类型"), TEXT("程序版本"), TEXT("提供符号名称的头文件"),
    TEXT("未知类型"), TEXT("即插即用资源"), TEXT("VXD"), TEXT("动态光标"), TEXT("动态图标"),
    TEXT("HTML"), TEXT("清单文件") };

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 通过指定类型数据的RVA得到FOA
INT RVAToFOA(PIMAGE_NT_HEADERS pImageNtHeader, DWORD dwTargetRVA);

// 获取资源信息
BOOL GetResourceInfo(PIMAGE_RESOURCE_DIRECTORY pImageRes, PIMAGE_RESOURCE_DIRECTORY pImageResDir,
   HTREEITEM hTreeParent, DWORD dwLevel);

// 获取选定资源的信息
BOOL GetSelectedResData(PIMAGE_DOS_HEADER pImageDosHeader, LPBYTE lpResData, DWORD dwResSize);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   // 打开文件所用变量
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

   // 内存映射文件所用变量
   static HANDLE hFile, hFileMap;
   static LPVOID lpMemory;
   LARGE_INTEGER liFileSize;

   // DOS头指针和PE文件头指针
   static PIMAGE_DOS_HEADER pImageDosHeader;
   static PIMAGE_NT_HEADERS pImageNtHeader;

   // 第1层目录中的资源目录结构起始地址，也就是资源表的起始地址
   PIMAGE_RESOURCE_DIRECTORY pImageRes;

   // 多行编辑控件字体
   HFONT hFont;

   // 调整树视图控件和多行编辑控件窗口大小之用
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

      // 设置多行编辑控件为宋体
      hFont = CreateFont(12, 0, 0, 0, 0, 0, 0, 0, GB2312_CHARSET, 0, 0, 0, 0, TEXT("宋体"));
      SendMessage(g_hwndEdit, WM_SETFONT, (WPARAM)hFont, FALSE);
      DeleteObject(hFont);

      // 默认情况下编辑控件最大缓冲区大小约为32KB个字符，设为不限大小
      SendMessage(g_hwndEdit, EM_SETLIMITTEXT, 0, 0);

      // 保存程序窗口大小
      GetClientRect(hwndDlg, &rectWindow);
      // 保存树视图控件和多行编辑控件的宽度高度
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
         // 打开一个PE文件
         GetDlgItemText(hwndDlg, IDC_EDIT_FILENAME, szFile, _countof(szFile));
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

         // 第1层目录中的资源目录结构起始地址，区分PE和PE32+
         if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
            pImageRes = (PIMAGE_RESOURCE_DIRECTORY)((LPBYTE)pImageDosHeader +
               RVAToFOA(pImageNtHeader, ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[2].VirtualAddress));
         else
            pImageRes = (PIMAGE_RESOURCE_DIRECTORY)((LPBYTE)pImageDosHeader +
               RVAToFOA(pImageNtHeader, ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[2].VirtualAddress));

         // 清空编辑控件
         SetDlgItemText(hwndDlg, IDC_EDIT_RESOURCE, TEXT(""));

         // 获取资源信息
         GetResourceInfo(pImageRes, pImageRes, TVI_ROOT, 1);
         break;

      case IDCANCEL:
         // 清理工作
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
         LPBYTE lpResData;       // 所选择资源项的资源数据起始地址
         DWORD dwResSize;        // 所选择资源项的资源数据大小

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

/*********************************************************************************
  * 函数功能：		获取资源信息
  * 输入参数的说明：
    1. pImageRes参数表示第1层目录中的资源目录结构起始地址，也就是资源表的起始地址，必须指定
    2. pImageResDir参数表示第1～3层目录中的资源目录结构起始地址，必须指定
    3. hTreeParent参数表示树视图控件中父节点的句柄，必须指定
    4. dwLevel参数指定为数值1～3，表示当前调用本函数是为了获取第几层目录的信息，必须指定
  * 该函数为递归函数
**********************************************************************************/
BOOL GetResourceInfo(PIMAGE_RESOURCE_DIRECTORY pImageRes, PIMAGE_RESOURCE_DIRECTORY pImageResDir,
   HTREEITEM hTreeParent, DWORD dwLevel)
{
   PIMAGE_RESOURCE_DIRECTORY pImageResDirSub;          // 下一层资源目录结构起始地址
   PIMAGE_RESOURCE_DIRECTORY_ENTRY pImageResDirEntry;  // 资源目录入口结构数组起始地址
   WORD wNums;                                         // 资源目录入口结构数组个数
   PIMAGE_RESOURCE_DATA_ENTRY pImageResDataEntry;      // 资源数据入口结构起始地址
   PIMAGE_RESOURCE_DIR_STRING_U pString;
   HTREEITEM hTree;
   TVINSERTSTRUCT tvi = { 0 };
   TCHAR szResType[128] = { 0 }, szResID[128] = { 0 }, szLanguageID[128] = { 0 };
   TCHAR szBuf[256] = { 0 };

   // 资源目录入口结构数组起始地址
   pImageResDirEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((LPBYTE)pImageResDir +
      sizeof(IMAGE_RESOURCE_DIRECTORY));
   // 资源目录入口结构数组个数
   wNums = pImageResDir->NumberOfNamedEntries + pImageResDir->NumberOfIdEntries;

   tvi.item.mask = TVIF_TEXT;
   tvi.hInsertAfter = TVI_LAST;
   tvi.hParent = hTreeParent;

   if (dwLevel == 1)
   {
      // 遍历资源目录入口结构数组
      for (WORD i = 0; i < wNums; i++)
      {
         // 资源类型
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
               wsprintf(szResType, TEXT("%d(自定义ID)"), LOWORD(pImageResDirEntry[i].Name));
         }
         tvi.item.pszText = szResType;
         hTree = (HTREEITEM)SendMessage(g_hwndTree, TVM_INSERTITEM, 0, (LPARAM)&tvi);

         // 递归进入第2层
         pImageResDirSub = (PIMAGE_RESOURCE_DIRECTORY)
            ((LPBYTE)pImageRes + (pImageResDirEntry[i].OffsetToData & 0x7FFFFFFF));
         GetResourceInfo(pImageRes, pImageResDirSub, hTree, 2);
      }
   }
   else if (dwLevel == 2)
   {
      // 遍历资源目录入口结构数组
      for (WORD i = 0; i < wNums; i++)
      {
         // 资源ID
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

         // 递归进入第3层
         pImageResDirSub = (PIMAGE_RESOURCE_DIRECTORY)
            ((LPBYTE)pImageRes + (pImageResDirEntry[i].OffsetToData & 0x7FFFFFFF));
         GetResourceInfo(pImageRes, pImageResDirSub, hTree, 3);
      }
   }
   else
   {
      // 遍历资源目录入口结构数组
      for (WORD i = 0; i < wNums; i++)
      {
         // 语言ID
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

         // 资源数据入口结构起始地址
         pImageResDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)
            ((LPBYTE)pImageRes + (pImageResDirEntry[i].OffsetToData));
         wsprintf(szBuf, TEXT("%s  RVA：0x%08X 大小：0x%X"),
            szLanguageID, pImageResDataEntry->OffsetToData, pImageResDataEntry->Size);

         tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
         tvi.item.pszText = szBuf;
         tvi.item.lParam = (LPARAM)pImageResDataEntry;// 保存资源数据入口结构起始地址到项目数据
         SendMessage(g_hwndTree, TVM_INSERTITEM, 0, (LPARAM)&tvi);

         // 递归出口
         return TRUE;
      }
   }

   return TRUE;
}

BOOL GetSelectedResData(PIMAGE_DOS_HEADER pImageDosHeader, LPBYTE lpResData, DWORD dwResSize)
{
   DWORD dwFileOffset;             // 文件偏移

   dwFileOffset = lpResData - (LPBYTE)pImageDosHeader;



   return TRUE;
}