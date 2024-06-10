#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// I/O��������
enum IOOPERATION { IO_UNKNOWN, IO_READ, IO_WRITE };

// I/OΨһ�����࣬�̳���OVERLAPPED�ṹ
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

// ��������
#define BUFSIZE               (64 * 1024)     // ��������С���ڴ�������ȴ�С64K
#define MAX_PENDING_IO_REQS   4               // ���I/O������

// ȫ�ֱ���
HWND g_hwndDlg;                               // ���򴰿ھ��

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL FileCopy(LPCTSTR pszFileSrc, LPCTSTR pszFileDest);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   TCHAR         szFileSrc[MAX_PATH] = { 0 };         // �����û�ѡ����ļ����Ļ�����
   HANDLE        hFileSrc = INVALID_HANDLE_VALUE;     // Դ�ļ����
   LARGE_INTEGER liFileSizeSrc = { 0 };               // Դ�ļ���С
   TCHAR         szFileSize[64] = { 0 };              // Դ�ļ���С���ַ�����ʽ
   TCHAR         szFileDest[MAX_PATH] = { 0 };        // Ŀ���ļ���������
   LPTSTR        lpStr;
   TCHAR         szBuf[64] = { 0 };

   OPENFILENAME ofn = { sizeof(OPENFILENAME) };
   ofn.hwndOwner = hwndDlg;
   ofn.lpstrFilter = TEXT("All(*.*)\0*.*\0");
   ofn.lpstrFile = szFileSrc;                   // �����û�ѡ����ļ����Ļ�����
   ofn.lpstrFile[0] = NULL;                     // ����Ҫ��ʼ���ļ����༭�ؼ�
   ofn.nMaxFile = _countof(szFileSrc);
   ofn.lpstrInitialDir = TEXT("C:\\");          // ��ʼĿ¼
   ofn.lpstrTitle = TEXT("��ѡ��Ҫ�򿪵��ļ�"); // �Ի������������ʾ���ַ���
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
            // ��ʾ�ļ�·��
            SetDlgItemText(hwndDlg, IDC_EDIT_FILEPATH, szFileSrc);

            // ��ȡ����ʾ�ļ���С
            hFileSrc = CreateFile(szFileSrc, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
               FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFileSrc == INVALID_HANDLE_VALUE)
            {
               MessageBox(hwndDlg, TEXT("���ļ�ʧ��"), TEXT("��ʾ"), MB_OK);
               return TRUE;
            }

            GetFileSizeEx(hFileSrc, &liFileSizeSrc);
            _i64tot_s(liFileSizeSrc.QuadPart, szFileSize, _countof(szFileSize), 10);
            SetDlgItemText(hwndDlg, IDC_EDIT_FILESIZE, szFileSize);

            CloseHandle(hFileSrc);
         }
         break;

      case IDC_BTN_COPY:
         // ��ȡԴ�ļ�·��
         GetDlgItemText(hwndDlg, IDC_EDIT_FILEPATH, szFileSrc, _countof(szFileSrc));

         // ����Ŀ���ļ�·����ƴ��Ŀ���ļ���Ϊ��Դ�ļ���_����.xxx��
         StringCchCopy(szFileDest, _countof(szFileDest), szFileSrc);
         lpStr = _tcsrchr(szFileDest, TEXT('.'));                    // ".xxx"
         StringCchCopy(szBuf, _countof(szBuf), lpStr);               // ��ʱ����".xxx"��szBuf
         StringCchCopy(lpStr, _countof(szFileDest), TEXT("_����"));  // "Դ�ļ���_����"
         StringCchCat(szFileDest, _countof(szFileDest), szBuf);      // "Դ�ļ���_����.xxx"

         // ��ʼ���ƹ���
         FileCopy(szFileSrc, szFileDest);
         MessageBox(hwndDlg, TEXT("���ƹ������"), TEXT("��ʾ"), MB_OK);
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
   HANDLE        hFileSrc = INVALID_HANDLE_VALUE;     // Դ�ļ����
   HANDLE        hFileDest = INVALID_HANDLE_VALUE;    // Ŀ���ļ����
   LARGE_INTEGER liFileSizeSrc = { 0 };               // Դ�ļ���С
   LARGE_INTEGER liFileSizeDest = { 0 };              // Ŀ���ļ���С
   HANDLE        hCompletionPort;                     // ��ɶ˿ھ��
   PERIODATA     arrPerIOData[MAX_PENDING_IO_REQS];   // I/OΨһ���ݶ�������
   LARGE_INTEGER liNextReadOffset = { 0 };            // ��ȡԴ�ļ�ʹ�õ��ļ�ƫ��
   INT           nReadsInProgress = 0;                // ���ڽ����еĶ�ȡ����ĸ���
   INT           nWritesInProgress = 0;               // ���ڽ����е�д������ĸ���

   // ��Դ�ļ�����ʹ��ϵͳ���棬�첽I/O
   hFileSrc = CreateFile(pszFileSrc, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL);
   if (hFileSrc == INVALID_HANDLE_VALUE)
   {
      MessageBox(g_hwndDlg, TEXT("���ļ�ʧ��"), TEXT("��ʾ"), MB_OK);
      return FALSE;
   }

   // ����Ŀ���ļ������һ����������ΪhFileSrc��ʾĿ���ļ�ʹ�ú�Դ�ļ���ͬ������
   hFileDest = CreateFile(pszFileDest, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, hFileSrc);
   if (hFileDest == INVALID_HANDLE_VALUE)
   {
      MessageBox(g_hwndDlg, TEXT("�����ļ�ʧ��"), TEXT("��ʾ"), MB_OK);
      return FALSE;
   }

   // ��ȡԴ�ļ���С
   GetFileSizeEx(hFileSrc, &liFileSizeSrc);

   // Ŀ���ļ���С����Ϊ�ڴ�������ȵ�������
   liFileSizeDest.QuadPart = ((liFileSizeSrc.QuadPart / BUFSIZE) * BUFSIZE) +
      (((liFileSizeSrc.QuadPart % BUFSIZE) > 0) ? BUFSIZE : 0);

   // ����Ŀ���ļ���С����չ���ڴ�������ȵ�������������Ϊ�����ڴ��������Ϊ��λ����I/O����
   SetFilePointerEx(hFileDest, liFileSizeDest, NULL, FILE_BEGIN);
   SetEndOfFile(hFileDest);

   // ����I/O��ɶ˿ڣ���������Դ�ļ���Ŀ���ļ����ļ�����������ע��ʹ���˲�ͬ����ɼ�
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

   // �ڴ���Ͷ��MAX_PENDING_IO_REQS(������4)��д�����������ݰ����Ӷ���ʼ��ȡԴ�ļ�����
   for (int i = 0; i < MAX_PENDING_IO_REQS; i++)
   {
      arrPerIOData[i].AllocBuffer(BUFSIZE);
      PostQueuedCompletionStatus(hCompletionPort, 0, IO_WRITE, &arrPerIOData[i]);
      nWritesInProgress++;
   }

   // ѭ��ֱ������I/O�������
   while ((nReadsInProgress > 0) || (nWritesInProgress > 0))
   {
      ULONG_PTR  CompletionKey;
      DWORD      dwNumberOfBytesTransferred;
      PERIODATA* pPerIOData;

      GetQueuedCompletionStatus(hCompletionPort, &dwNumberOfBytesTransferred,
         &CompletionKey, (LPOVERLAPPED*)&pPerIOData, INFINITE);

      switch (CompletionKey)
      {
      case IO_READ:     // ��ȡԴ�ļ���һ���ֲ�����ɣ���ʼд��Ŀ���ļ�
         nReadsInProgress--;
         pPerIOData->Write(hFileDest);
         nWritesInProgress++;
         break;

      case IO_WRITE:    // д��Ŀ���ļ���һ���ֲ�����ɣ���ʼ��ȡԴ�ļ�
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

   // ���Ʋ����Ѿ���ɣ�������
   CloseHandle(hFileSrc);
   CloseHandle(hFileDest);
   if (hCompletionPort != NULL)
      CloseHandle(hCompletionPort);

   // ����Ŀ���ļ�Ϊʵ�ʴ�С����β�ʹ��FILE_FLAG_NO_BUFFERING���ļ���������������С�����������
   hFileDest = CreateFile(pszFileDest, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL, NULL);
   if (hFileDest == INVALID_HANDLE_VALUE)
   {
      MessageBox(g_hwndDlg, TEXT("����Ŀ���ļ���Сʧ��"), TEXT("��ʾ"), MB_OK);
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