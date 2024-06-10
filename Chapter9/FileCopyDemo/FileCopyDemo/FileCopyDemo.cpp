#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// I/O操作类型
enum IOOPERATION { IO_UNKNOWN, IO_READ, IO_WRITE };

// I/O唯一数据类，继承自OVERLAPPED结构
class PERIODATA : public OVERLAPPED {
public:
   PERIODATA()
   {
      Internal = InternalHigh = 0;
      Offset = OffsetHigh = 0;
      hEvent = NULL;

      m_nBuffSize = 0;
      m_pData = NULL;
   }

   ~PERIODATA()
   {
      if (m_pData != NULL)
         VirtualFree(m_pData, 0, MEM_RELEASE);
   }

   BOOL AllocBuffer(SIZE_T nBuffSize)
   {
      m_nBuffSize = nBuffSize;
      m_pData = VirtualAlloc(NULL, m_nBuffSize, MEM_COMMIT, PAGE_READWRITE);

      return m_pData != NULL;
   }

   BOOL Read(HANDLE hFile, PLARGE_INTEGER pliOffset = NULL)
   {
      if (pliOffset != NULL)
      {
         Offset = pliOffset->LowPart;
         OffsetHigh = pliOffset->HighPart;
      }

      return ReadFile(hFile, m_pData, m_nBuffSize, NULL, this);
   }

   BOOL Write(HANDLE hFile, PLARGE_INTEGER pliOffset = NULL)
   {
      if (pliOffset != NULL)
      {
         Offset = pliOffset->LowPart;
         OffsetHigh = pliOffset->HighPart;
      }

      return WriteFile(hFile, m_pData, m_nBuffSize, NULL, this);
   }

private:
   SIZE_T m_nBuffSize;
   LPVOID  m_pData;
};

// 常量定义
#define BUFSIZE               (64 * 1024)     // 缓冲区大小，内存分配粒度大小64K
#define MAX_PENDING_IO_REQS   4               // 最大I/O请求数

// 全局变量
HWND g_hwndDlg;                               // 程序窗口句柄

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL FileCopy(LPCTSTR pszFileSrc, LPCTSTR pszFileDest);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   TCHAR         szFileSrc[MAX_PATH] = { 0 };         // 返回用户选择的文件名的缓冲区
   HANDLE        hFileSrc = INVALID_HANDLE_VALUE;     // 源文件句柄
   LARGE_INTEGER liFileSizeSrc = { 0 };               // 源文件大小
   TCHAR         szFileSize[64] = { 0 };              // 源文件大小的字符串形式
   TCHAR         szFileDest[MAX_PATH] = { 0 };        // 目标文件名缓冲区
   LPTSTR        lpStr;
   TCHAR         szBuf[64] = { 0 };

   OPENFILENAME ofn = { sizeof(OPENFILENAME) };
   ofn.hwndOwner = hwndDlg;
   ofn.lpstrFilter = TEXT("All(*.*)\0*.*\0");
   ofn.lpstrFile = szFileSrc;                   // 返回用户选择的文件名的缓冲区
   ofn.lpstrFile[0] = NULL;                     // 不需要初始化文件名编辑控件
   ofn.nMaxFile = _countof(szFileSrc);
   ofn.lpstrInitialDir = TEXT("C:\\");          // 初始目录
   ofn.lpstrTitle = TEXT("请选择要打开的文件"); // 对话框标题栏中显示的字符串
   ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_CREATEPROMPT;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      g_hwndDlg = hwndDlg;
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_OPEN:
         if (GetOpenFileName(&ofn))
         {
            // 显示文件路径
            SetDlgItemText(hwndDlg, IDC_EDIT_FILEPATH, szFileSrc);

            // 获取、显示文件大小
            hFileSrc = CreateFile(szFileSrc, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
               FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFileSrc == INVALID_HANDLE_VALUE)
            {
               MessageBox(hwndDlg, TEXT("打开文件失败"), TEXT("提示"), MB_OK);
               return TRUE;
            }

            GetFileSizeEx(hFileSrc, &liFileSizeSrc);
            _i64tot_s(liFileSizeSrc.QuadPart, szFileSize, _countof(szFileSize), 10);
            SetDlgItemText(hwndDlg, IDC_EDIT_FILESIZE, szFileSize);

            CloseHandle(hFileSrc);
         }
         break;

      case IDC_BTN_COPY:
         // 获取源文件路径
         GetDlgItemText(hwndDlg, IDC_EDIT_FILEPATH, szFileSrc, _countof(szFileSrc));

         // 设置目标文件路径，拼接目标文件名为“源文件名_复制.xxx”
         StringCchCopy(szFileDest, _countof(szFileDest), szFileSrc);
         lpStr = _tcsrchr(szFileDest, TEXT('.'));                    // ".xxx"
         StringCchCopy(szBuf, _countof(szBuf), lpStr);               // 临时保存".xxx"到szBuf
         StringCchCopy(lpStr, _countof(szFileDest), TEXT("_复制"));  // "源文件名_复制"
         StringCchCat(szFileDest, _countof(szFileDest), szBuf);      // "源文件名_复制.xxx"

         // 开始复制工作
         FileCopy(szFileSrc, szFileDest);
         MessageBox(hwndDlg, TEXT("复制工作完成"), TEXT("提示"), MB_OK);
         break;
      }
      return TRUE;

   case WM_CLOSE:
      EndDialog(hwndDlg, 0);
      return TRUE;
   }

   return FALSE;
}

BOOL FileCopy(LPCTSTR pszFileSrc, LPCTSTR pszFileDest)
{
   HANDLE        hFileSrc = INVALID_HANDLE_VALUE;     // 源文件句柄
   HANDLE        hFileDest = INVALID_HANDLE_VALUE;    // 目标文件句柄
   LARGE_INTEGER liFileSizeSrc = { 0 };               // 源文件大小
   LARGE_INTEGER liFileSizeDest = { 0 };              // 目标文件大小
   HANDLE        hCompletionPort;                     // 完成端口句柄
   PERIODATA     arrPerIOData[MAX_PENDING_IO_REQS];   // I/O唯一数据对象数组
   LARGE_INTEGER liNextReadOffset = { 0 };            // 读取源文件使用的文件偏移
   INT           nReadsInProgress = 0;                // 正在进行中的读取请求的个数
   INT           nWritesInProgress = 0;               // 正在进行中的写入请求的个数

   // 打开源文件，不使用系统缓存，异步I/O
   hFileSrc = CreateFile(pszFileSrc, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
   if (hFileSrc == INVALID_HANDLE_VALUE)
   {
      MessageBox(g_hwndDlg, TEXT("打开文件失败"), TEXT("提示"), MB_OK);
      return FALSE;
   }

   // 创建目标文件，最后一个参数设置为hFileSrc表示目标文件使用和源文件相同的属性
   hFileDest = CreateFile(pszFileDest, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, hFileSrc);
   if (hFileDest == INVALID_HANDLE_VALUE)
   {
      MessageBox(g_hwndDlg, TEXT("创建文件失败"), TEXT("提示"), MB_OK);
      return FALSE;
   }

   // 获取源文件大小
   GetFileSizeEx(hFileSrc, &liFileSizeSrc);

   // 目标文件大小设置为内存分配粒度的整数倍
   liFileSizeDest.QuadPart = ((liFileSizeSrc.QuadPart / BUFSIZE) * BUFSIZE) +
      (((liFileSizeSrc.QuadPart % BUFSIZE) > 0) ? BUFSIZE : 0);

   // 设置目标文件大小，扩展到内存分配粒度的整数倍，这是为了以内存分配粒度为单位进行I/O操作
   SetFilePointerEx(hFileDest, liFileSizeDest, NULL, FILE_BEGIN);
   SetEndOfFile(hFileDest);

   // 创建I/O完成端口，并将其与源文件和目标文件的文件句柄相关联，注意使用了不同的完成键
   hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
   if (hCompletionPort != NULL)
   {
      CreateIoCompletionPort(hFileSrc, hCompletionPort, IO_READ, 0);
      CreateIoCompletionPort(hFileDest, hCompletionPort, IO_WRITE, 0);
   }
   else
   {
      return FALSE;
   }

   // 在此先投递MAX_PENDING_IO_REQS(这里是4)个写入操作完成数据包，从而开始读取源文件工作
   for (int i = 0; i < MAX_PENDING_IO_REQS; i++)
   {
      arrPerIOData[i].AllocBuffer(BUFSIZE);
      PostQueuedCompletionStatus(hCompletionPort, 0, IO_WRITE, &arrPerIOData[i]);
      nWritesInProgress++;
   }

   // 循环直至所有I/O操作完成
   while ((nReadsInProgress > 0) || (nWritesInProgress > 0))
   {
      ULONG_PTR  CompletionKey;
      DWORD      dwNumberOfBytesTransferred;
      PERIODATA* pPerIOData;

      GetQueuedCompletionStatus(hCompletionPort, &dwNumberOfBytesTransferred,
         &CompletionKey, (LPOVERLAPPED*)&pPerIOData, INFINITE);

      switch (CompletionKey)
      {
      case IO_READ:     // 读取源文件的一部分操作完成，开始写入目标文件
         nReadsInProgress--;
         pPerIOData->Write(hFileDest);
         nWritesInProgress++;
         break;

      case IO_WRITE:    // 写入目标文件的一部分操作完成，开始读取源文件
         nWritesInProgress--;
         if (liNextReadOffset.QuadPart < liFileSizeDest.QuadPart)
         {
            pPerIOData->Read(hFileSrc, &liNextReadOffset);
            nReadsInProgress++;
            liNextReadOffset.QuadPart += BUFSIZE;
         }
         break;
      }
   }

   // 复制操作已经完成，清理工作
   CloseHandle(hFileSrc);
   CloseHandle(hFileDest);
   if (hCompletionPort != NULL)
      CloseHandle(hCompletionPort);

   // 设置目标文件为实际大小，这次不使用FILE_FLAG_NO_BUFFERING，文件操作不受扇区大小对齐这个限制
   hFileDest = CreateFile(pszFileDest, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL, NULL);
   if (hFileDest == INVALID_HANDLE_VALUE)
   {
      MessageBox(g_hwndDlg, TEXT("设置目标文件大小失败"), TEXT("提示"), MB_OK);
      return FALSE;
   }
   else
   {
      SetFilePointerEx(hFileDest, liFileSizeSrc, NULL, FILE_BEGIN);
      SetEndOfFile(hFileDest);
      CloseHandle(hFileDest);
   }

   return TRUE;
}