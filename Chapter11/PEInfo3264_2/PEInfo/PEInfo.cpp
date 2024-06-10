#include <windows.h>
#include <tchar.h>
#include "resource.h"

// 全局变量
HWND g_hwndDlg;                 // 窗口句柄
HWND g_hwndEdit;                // 多行编辑控件窗口句柄
LPCTSTR arrDataDirectory[] = { TEXT("导出表\t\t"), TEXT("导入表\t\t"),
                               TEXT("资源表\t\t"), TEXT("异常表\t\t"),
                               TEXT("属性证书表\t"), TEXT("重定位表\t"),
                               TEXT("调试信息\t"), TEXT("与平台相关的数据"),
                               TEXT("指向全局指针寄存器的值"), TEXT("线程局部存储\t"),
                               TEXT("加载配置信息表\t"), TEXT("绑定导入表\t"),
                               TEXT("导入函数地址表IAT"), TEXT("延迟加载导入表\t"),
                               TEXT("CLR运行时头部数据"), TEXT("保留\t\t") };

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 通过指定类型数据(例如导入表、导出表、重定位表等)的RVA得到FOA
INT RVAToFOA(PIMAGE_NT_HEADERS pImageNtHeader, DWORD dwTargetRVA);

// 通过一个RVA值获取所在节区的名称
LPSTR GetSectionNameByRVA(PIMAGE_NT_HEADERS pImageNtHeader, DWORD dwRVA);

// 通过IMAGE_FILE_HEADER.TimeDateStamp字段获取日期时间格式字符串
LPTSTR GetDateTime(DWORD dwTimeDateStamp);

// 获取PE文件基本信息、节区信息、数据块信息
BOOL GetBaseInfo(PIMAGE_NT_HEADERS pImageNtHeader);

// 获取导入表中所有dll的导入函数的函数编号和函数名称
BOOL GetImportTable(PIMAGE_DOS_HEADER pImageDosHeader);

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
   HANDLE hFile, hFileMap;
   LPVOID lpMemory;
   LARGE_INTEGER liFileSize;

   // DOS头指针和PE文件头指针
   PIMAGE_DOS_HEADER pImageDosHeader;
   PIMAGE_NT_HEADERS pImageNtHeader;

   // 多行编辑控件字体
   HFONT hFont;

   // 调整多行编辑控件窗口大小之用
   static RECT rectWindow;
   RECT rectEdit;
   static int nWidthEdit, nHeightEdit;
   int cx, cy;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      g_hwndDlg = hwndDlg;

      g_hwndEdit = GetDlgItem(hwndDlg, IDC_EDIT_INFO);

      // 设置多行编辑控件为宋体
      hFont = CreateFont(12, 0, 0, 0, 0, 0, 0, 0, GB2312_CHARSET, 0, 0, 0, 0, TEXT("宋体"));
      SendMessage(g_hwndEdit, WM_SETFONT, (WPARAM)hFont, FALSE);

      // 保存程序窗口大小
      GetClientRect(hwndDlg, &rectWindow);
      // 保存多行编辑控件的宽度高度
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

         // 清空编辑控件
         SetDlgItemText(hwndDlg, IDC_EDIT_INFO, TEXT(""));

         //PE文件基本信息、节区信息、数据块信息
         GetBaseInfo(pImageNtHeader);

         //导入表信息
         GetImportTable(pImageDosHeader);

         //导出表信息


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

BOOL GetBaseInfo(PIMAGE_NT_HEADERS pImageNtHeader)
{
   PIMAGE_SECTION_HEADER pImageSectionHeader;
   TCHAR szSectionName[IMAGE_SIZEOF_SHORT_NAME + 1] = { 0 };  // Unicode节区名称
   TCHAR szBuf[256] = { 0 };

   // 基本信息
   // 如果是PE32+则把pImageNtHeader强制转换为PIMAGE_NT_HEADERS64
   if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
   {
      wsprintf(szBuf, TEXT("运行平台：\t0x%04X\r\n节区数量：\t0x%04X\r\n创建时间：\t%s\r\n文件属性：\t0x%04X\r\n文件格式：\t0x%04X\r\n建议装载地址：\t0x%016I64X\r\n入口地址：\t0x%08X\r\n内存对齐：\t0x%08X\r\n文件对齐：\t0x%08X\r\n内存映像大小：\t0x%08X\r\n校验和：\t0x%08X\r\n数据目录个数：\t0x%08X\r\n\r\n"),
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
   // 如果是PE则把pImageNtHeader强制转换为PIMAGE_NT_HEADERS32
   else
   {
      wsprintf(szBuf, TEXT("运行平台：\t0x%04X\r\n节区数量：\t0x%04X\r\n创建时间：\t%s\r\n文件属性：\t0x%04X\r\n文件格式：\t0x%04X\r\n建议装载地址：\t0x%08X\r\n入口地址：\t0x%08X\r\n内存对齐：\t0x%08X\r\n文件对齐：\t0x%08X\r\n内存映像大小：\t0x%08X\r\n校验和：\t0x%08X\r\n数据目录个数：\t0x%08X\r\n\r\n"),
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

   // 节区信息
   wsprintf(szBuf, TEXT("节区名称\t节区  RVA\t节区 FOA\t实际大小\t对齐大小\t节区属性\r\n"));
   SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
   SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

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
      // 节区名称
      MultiByteToWideChar(CP_UTF8, 0, (LPSTR)pImageSectionHeader, IMAGE_SIZEOF_SHORT_NAME,
         szSectionName, IMAGE_SIZEOF_SHORT_NAME);

      // 其他信息
      wsprintf(szBuf, TEXT("%-8s\t0x%08X\t0x%08X\t0x%08X\t0x%08X\t0x%08X\r\n"),
         szSectionName,
         pImageSectionHeader->VirtualAddress,
         pImageSectionHeader->PointerToRawData,
         pImageSectionHeader->Misc.VirtualSize,
         pImageSectionHeader->SizeOfRawData,
         pImageSectionHeader->Characteristics);
      SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
      SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

      // 指向下一个节区信息结构
      pImageSectionHeader++;
   }
   SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
   SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)TEXT("\r\n"));

   // 各种类型数据块的信息
   wsprintf(szBuf, TEXT("索引\t\t数据目录\t\t数据的RVA\t数据的大小\t数据的FOA\t所处的节区\r\n"));
   SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
   SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

   // 如果是PE32+则把pImageNtHeader强制转换为PIMAGE_NT_HEADERS64
   if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
   {
      for (DWORD i = 0; i < ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.NumberOfRvaAndSizes; i++)
      {
         if (((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[i].Size != 0)
         {
            // 所处的节区名称
            MultiByteToWideChar(CP_UTF8, 0,
               GetSectionNameByRVA(pImageNtHeader,
                  ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[i].VirtualAddress),
               IMAGE_SIZEOF_SHORT_NAME, szSectionName, IMAGE_SIZEOF_SHORT_NAME);

            // 其他信息
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
   // 如果是PE则把pImageNtHeader强制转换为PIMAGE_NT_HEADERS32
   else
   {
      for (DWORD i = 0; i < ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.NumberOfRvaAndSizes; i++)
      {
         if (((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[i].Size != 0)
         {
            // 所处的节区名称
            MultiByteToWideChar(CP_UTF8, 0,
               GetSectionNameByRVA(pImageNtHeader,
                  ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[i].VirtualAddress),
               IMAGE_SIZEOF_SHORT_NAME, szSectionName, IMAGE_SIZEOF_SHORT_NAME);

            // 其他信息
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
   PIMAGE_NT_HEADERS pImageNtHeader;               // PE文件头起始地址
   PIMAGE_IMPORT_DESCRIPTOR pImageImportDescriptor;// 导入表起始地址
   PIMAGE_THUNK_DATA32 pImageThunkData32;          // IMAGE_THUNK_DATA32数组起始地址
   PIMAGE_THUNK_DATA64 pImageThunkData64;          // IMAGE_THUNK_DATA64数组起始地址
   PIMAGE_IMPORT_BY_NAME pImageImportByName;       // IMAGE_IMPORT_BY_NAME结构指针
   TCHAR szDllName[128] = { 0 };                   // dll名称
   TCHAR szFuncName[128] = { 0 };                  // 函数名称
   TCHAR szBuf[256] = { 0 };
   TCHAR szImportTableHead[] = TEXT("\r\n\r\n导入表信息：\r\ndll文件名\t\t\t\t\t函数编号\t函数名称\r\n");

   // PE文件头起始地址
   pImageNtHeader = (PIMAGE_NT_HEADERS)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);

   // 如果是PE32+则把pImageNtHeader强制转换为PIMAGE_NT_HEADERS64
   if (pImageNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
   {
      // 是否有导入表(当然，没有的可能性不大)
      if (((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[1].Size == 0)
         return FALSE;

      // 导入表起始地址
      pImageImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, ((PIMAGE_NT_HEADERS64)pImageNtHeader)->OptionalHeader.DataDirectory[1].VirtualAddress));

      SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
      SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szImportTableHead);
      // 遍历导入表
      while (pImageImportDescriptor->OriginalFirstThunk ||
         pImageImportDescriptor->TimeDateStamp || pImageImportDescriptor->ForwarderChain ||
         pImageImportDescriptor->Name || pImageImportDescriptor->FirstThunk)
      {
         // dll名称
         MultiByteToWideChar(CP_UTF8, 0, (LPSTR)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageImportDescriptor->Name)), -1, szDllName, _countof(szDllName));

         // IMAGE_THUNK_DATA64数组起始地址
         pImageThunkData64 = (PIMAGE_THUNK_DATA64)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageImportDescriptor->FirstThunk));
         while (pImageThunkData64->u1.AddressOfData != 0)
         {
            // 按序号导入还是按函数名称导入
            // IMAGE_IMPORT_BY_NAME结构指针
            pImageImportByName = (PIMAGE_IMPORT_BY_NAME)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageThunkData64->u1.AddressOfData));

            if (pImageThunkData64->u1.AddressOfData & IMAGE_ORDINAL_FLAG64)
            {
               wsprintf(szFuncName, TEXT("按序号 0x%04X"), pImageThunkData64->u1.AddressOfData & 0xFFFF);
               wsprintf(szBuf, TEXT("%-48s%s\r\n"), szDllName, szFuncName);
            }
            else
            {
               MultiByteToWideChar(CP_UTF8, 0, pImageImportByName->Name, -1, szFuncName, _countof(szFuncName));
               wsprintf(szBuf, TEXT("%-48s0x%04X\t\t%s\r\n"), szDllName, pImageImportByName->Hint, szFuncName);
            }
            SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
            SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

            // 指向下一个IMAGE_THUNK_DATA64结构
            pImageThunkData64++;
         }

         SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
         SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)TEXT("\r\n"));
         // 指向下一个导入表描述符
         pImageImportDescriptor++;
      }
   }
   // 如果是PE则把pImageNtHeader强制转换为PIMAGE_NT_HEADERS32
   else
   {
      // 是否有导入表(当然，没有的可能性不大)
      if (((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[1].Size == 0)
         return FALSE;

      // 导入表起始地址
      pImageImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, ((PIMAGE_NT_HEADERS32)pImageNtHeader)->OptionalHeader.DataDirectory[1].VirtualAddress));

      SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
      SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szImportTableHead);
      // 遍历导入表
      while (pImageImportDescriptor->OriginalFirstThunk ||
         pImageImportDescriptor->TimeDateStamp || pImageImportDescriptor->ForwarderChain ||
         pImageImportDescriptor->Name || pImageImportDescriptor->FirstThunk)
      {
         // dll名称
         MultiByteToWideChar(CP_UTF8, 0, (LPSTR)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageImportDescriptor->Name)), -1, szDllName, _countof(szDllName));

         // IMAGE_THUNK_DATA32数组起始地址
         pImageThunkData32 = (PIMAGE_THUNK_DATA32)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageImportDescriptor->FirstThunk));
         while (pImageThunkData32->u1.AddressOfData != 0)
         {
            // 按序号导入还是按函数名称导入
            // IMAGE_IMPORT_BY_NAME结构指针
            pImageImportByName = (PIMAGE_IMPORT_BY_NAME)((LPBYTE)pImageDosHeader + RVAToFOA(pImageNtHeader, pImageThunkData32->u1.AddressOfData));

            if (pImageThunkData32->u1.AddressOfData & IMAGE_ORDINAL_FLAG32)
            {
               wsprintf(szFuncName, TEXT("按序号 0x%04X"), pImageThunkData32->u1.AddressOfData & 0xFFFF);
               wsprintf(szBuf, TEXT("%-48s%s\r\n"), szDllName, szFuncName);
            }
            else
            {
               MultiByteToWideChar(CP_UTF8, 0, pImageImportByName->Name, -1, szFuncName, _countof(szFuncName));
               wsprintf(szBuf, TEXT("%-48s0x%04X\t\t%s\r\n"), szDllName, pImageImportByName->Hint, szFuncName);
            }
            SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
            SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

            // 指向下一个IMAGE_THUNK_DATA32结构
            pImageThunkData32++;
         }

         SendMessage(g_hwndEdit, EM_SETSEL, -1, -1);
         SendMessage(g_hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)TEXT("\r\n"));
         // 指向下一个导入表描述符
         pImageImportDescriptor++;
      }
   }

   return TRUE;
}

/*********************************************************************************
  * 函数功能：通过指定类型数据(例如导入表、导出表、重定位表等)的RVA得到FOA
  * 输入参数的说明：
    1. pImageNtHeader参数表示PE内存映射文件对象中PE文件头的起始地址，必须指定
    2. dwTargetRVA参数表示目标类型数据的RVA，必须指定
  * 返回值：  返回-1表示函数执行失败
**********************************************************************************/
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
  * 函数功能：通过一个RVA值获取所在节区的名称
  * 输入参数的说明：
    1. pImageNtHeader参数表示PE内存映射文件对象中PE文件头的起始地址，必须指定
    2. dwRVA参数表示一个RVA值，必须指定
  * 返回值：  返回NULL表示函数执行失败，请注意返回的节区名称字符串并不一定以零结尾
**********************************************************************************/
LPSTR GetSectionNameByRVA(PIMAGE_NT_HEADERS pImageNtHeader, DWORD dwRVA)
{
   LPSTR lpSectionName = NULL;
   PIMAGE_SECTION_HEADER pImageSectionHeader;

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
      if ((dwRVA >= pImageSectionHeader->VirtualAddress) &&
         (dwRVA <= (pImageSectionHeader->VirtualAddress + pImageSectionHeader->SizeOfRawData)))
      {
         lpSectionName = (LPSTR)pImageSectionHeader;
      }

      // 指向下一个节区信息结构
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

   // 系统时间转换为文件时间才可以加上已经逝去的时间dwTimeDateStamp
   SystemTimeToFileTime(&st, &ft);

   // 文件时间单位是1/1000 0000秒，即1000万分之1秒(100-nanosecond)
   // 不要将指向FILETIME结构的指针强制转换为ULARGE_INTEGER *或__int64 *值
   // 因为这可能导致64位Windows上的对齐错误
   uli.HighPart = ft.dwHighDateTime;
   uli.LowPart = ft.dwLowDateTime;
   uli.QuadPart += (ULONGLONG)10000000 * dwTimeDateStamp;
   ft.dwHighDateTime = uli.HighPart;
   ft.dwLowDateTime = uli.LowPart;

   // 将世界时间转换为本地时间(两个参数都是文件时间)
   FileTimeToLocalFileTime(&ft, &ftLocal);
   // 再将文件时间转换为系统时间
   FileTimeToSystemTime(&ftLocal, &st);

   // 转换为日期时间格式字符串
   ZeroMemory(pszDateTime, 64);
   wsprintf(pszDateTime, TEXT("%d年%0.2d月%0.2d日 %0.2d:%0.2d:%0.2d"),
      st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wMinute);

   return pszDateTime;
}