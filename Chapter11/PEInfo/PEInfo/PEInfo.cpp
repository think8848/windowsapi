#include <windows.h>
#include "resource.h"

// 全局变量
HWND g_hwndDlg;
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

// 通过一个RVA值获取所在节区的名称
LPSTR GetSectionNameByRVA(PIMAGE_DOS_HEADER pImageDosHeader, DWORD dwRVA);

// 通过IMAGE_FILE_HEADER.TimeDateStamp字段获取日期时间格式字符串
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
      TEXT("exe文件(*.exe)\0*.exe\0dll文件(*.dll)\0*.dll\0All(*.*)\0*.*\0");
   ofn.lpstrFile = szFile;
   ofn.lpstrFile[0] = NULL;
   ofn.nMaxFile = _countof(szFile);
   ofn.lpstrTitle = TEXT("请选择要打开的PE文件");
   ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;


   HANDLE hFile, hFileMap;
   LPVOID lpMemory;
   LARGE_INTEGER liFileSize;

   PIMAGE_DOS_HEADER pImageDosHeader;
   PIMAGE_NT_HEADERS32 pImageNtHeader32;
   PIMAGE_SECTION_HEADER pImageSectionHeader;
   TCHAR szSectionName[IMAGE_SIZEOF_SHORT_NAME + 1] = { 0 };  // Unicode节区名称
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
         // 打开一个PE文件
         GetDlgItemText(g_hwndDlg, IDC_EDIT_PATH, szFile, _countof(szFile));
         hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, NULL);
         if (hFile == INVALID_HANDLE_VALUE)
         {
            MessageBox(g_hwndDlg, TEXT("CreateFile函数调用失败"), TEXT("提示"), MB_OK);
            return TRUE;
         }
         else
         {
            GetFileSizeEx(hFile, &liFileSize);
            if (liFileSize.QuadPart == 0)
            {
               MessageBox(g_hwndDlg, TEXT("文件大小为0"), TEXT("提示"), MB_OK);
               return TRUE;
            }
         }

         // 为hFile文件对象创建一个文件映射内核对象
         hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
         if (!hFileMap)
         {
            MessageBox(g_hwndDlg, TEXT("CreateFileMapping调用失败"), TEXT("提示"), MB_OK);
            return TRUE;
         }

         // 把文件映射对象hFileMap的全部映射到进程的虚拟地址空间中
         lpMemory = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
         if (!lpMemory)
         {
            MessageBox(g_hwndDlg, TEXT("MapViewOfFile调用失败"), TEXT("提示"), MB_OK);
            return TRUE;
         }

         // 打开的文件是不是PE文件
         pImageDosHeader = (PIMAGE_DOS_HEADER)lpMemory;
         if (pImageDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
         {
            MessageBox(g_hwndDlg, TEXT("打开的不是PE文件"), TEXT("提示"), MB_OK);
            return TRUE;
         }

         pImageNtHeader32 = (PIMAGE_NT_HEADERS32)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);
         if (pImageNtHeader32->Signature != IMAGE_NT_SIGNATURE)
         {
            MessageBox(g_hwndDlg, TEXT("打开的不是PE文件"), TEXT("提示"), MB_OK);
            return TRUE;
         }

         // 清空编辑控件
         SetDlgItemText(hwndDlg, IDC_EDIT_INFO, TEXT(""));

         // 基本信息
         wsprintf(szBuf, TEXT("运行平台：\t0x%04X\r\n节区数量：\t0x%04X\r\n创建时间：\t%s\r\n文件属性：\t0x%04X\r\n文件格式：\t0x%04X\r\n入口地址：\t0x%08X\r\n内存对齐：\t0x%08X\r\n文件对齐：\t0x%08X\r\n内存映像大小：\t0x%08X\r\n校验和：\t0x%08X\r\n数据目录个数：\t0x%08X\r\n\r\n"),
            pImageNtHeader32->FileHeader.Machine,
            pImageNtHeader32->FileHeader.NumberOfSections,
            GetDateTime(pImageNtHeader32->FileHeader.TimeDateStamp),
            pImageNtHeader32->FileHeader.Characteristics,
            pImageNtHeader32->OptionalHeader.Magic,
            pImageNtHeader32->OptionalHeader.AddressOfEntryPoint,
            pImageNtHeader32->OptionalHeader.SectionAlignment,
            pImageNtHeader32->OptionalHeader.FileAlignment,
            pImageNtHeader32->OptionalHeader.SizeOfImage,
            pImageNtHeader32->OptionalHeader.CheckSum,
            pImageNtHeader32->OptionalHeader.NumberOfRvaAndSizes);
         SendMessage(hwndEdit, EM_SETSEL, -1, -1);
         SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

         // 节区信息
         wsprintf(szBuf, TEXT("节区名称\t节区  RVA\t节区 FOA\t实际大小\t对齐大小\t节区属性\r\n"));
         SendMessage(hwndEdit, EM_SETSEL, -1, -1);
         SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

         pImageSectionHeader =
            (PIMAGE_SECTION_HEADER)((LPBYTE)pImageNtHeader32 + sizeof(IMAGE_NT_HEADERS32));
         // 遍历节表
         for (int i = 0; i < pImageNtHeader32->FileHeader.NumberOfSections; i++)
         {
            // 节区名称
            MultiByteToWideChar(CP_UTF8, 0, (LPSTR)pImageSectionHeader, IMAGE_SIZEOF_SHORT_NAME,
               szSectionName, IMAGE_SIZEOF_SHORT_NAME);

            // 其他信息
            wsprintf(szBuf, TEXT("%s\t\t0x%08X\t0x%08X\t0x%08X\t0x%08X\t0x%08X\r\n"),
               szSectionName,
               pImageSectionHeader->VirtualAddress,
               pImageSectionHeader->PointerToRawData,
               pImageSectionHeader->Misc.VirtualSize,
               pImageSectionHeader->SizeOfRawData,
               pImageSectionHeader->Characteristics);
            SendMessage(hwndEdit, EM_SETSEL, -1, -1);
            SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

            // 指向下一个节区信息结构
            pImageSectionHeader++;
         }
         SendMessage(hwndEdit, EM_SETSEL, -1, -1);
         SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)TEXT("\r\n"));

         // 各种类型数据块的信息
         wsprintf(szBuf, TEXT("索引\t\t数据目录\t\t数据的RVA\t所处的节区\r\n"));
         SendMessage(hwndEdit, EM_SETSEL, -1, -1);
         SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);

         for (DWORD i = 0; i < pImageNtHeader32->OptionalHeader.NumberOfRvaAndSizes; i++)
         {
            if (pImageNtHeader32->OptionalHeader.DataDirectory[i].Size != 0)
            {
               // 所处的节区名称
               MultiByteToWideChar(CP_UTF8, 0,
                  GetSectionNameByRVA(pImageDosHeader, pImageNtHeader32->OptionalHeader.DataDirectory[i].VirtualAddress),
                  IMAGE_SIZEOF_SHORT_NAME, szSectionName, IMAGE_SIZEOF_SHORT_NAME);

               // 其他信息
               wsprintf(szBuf, TEXT("%d\t\t%s\t0x%08X\t%s\r\n"),
                  i,
                  arrDataDirectory[i],
                  pImageNtHeader32->OptionalHeader.DataDirectory[i].VirtualAddress,
                  szSectionName);
               SendMessage(hwndEdit, EM_SETSEL, -1, -1);
               SendMessage(hwndEdit, EM_REPLACESEL, TRUE, (LPARAM)szBuf);
            }
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

/*********************************************************************************
  * 函数功能：通过一个RVA值获取所在节区的名称
  * 输入参数的说明：
    1. pImageDosHeader参数表示PE内存映射文件对象在内存中的起始地址，必须指定
    2. dwRVA参数表示一个RVA值，必须指定
  * 返回值：  返回NULL表示函数执行失败，请注意返回的节区名称字符串并不一定以零结尾
**********************************************************************************/
LPSTR GetSectionNameByRVA(PIMAGE_DOS_HEADER pImageDosHeader, DWORD dwRVA)
{
   LPSTR lpSectionName = NULL;

   // PE文件头的地址
   PIMAGE_NT_HEADERS32 pImageNtHeader32 =
      (PIMAGE_NT_HEADERS32)((LPBYTE)pImageDosHeader + pImageDosHeader->e_lfanew);

   // PE文件头的地址 + sizeof(IMAGE_NT_HEADERS32)就等于节表地址
   PIMAGE_SECTION_HEADER pImageSectionHeader =
      (PIMAGE_SECTION_HEADER)((LPBYTE)pImageNtHeader32 + sizeof(IMAGE_NT_HEADERS32));

   // 遍历节表
   for (int i = 0; i < pImageNtHeader32->FileHeader.NumberOfSections; i++)
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