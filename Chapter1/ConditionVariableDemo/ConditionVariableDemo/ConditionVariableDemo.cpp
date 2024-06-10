#include <windows.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// 常量定义
#define QUEUE_SIZE             10      // 队列大小
#define PRODUCER_SLEEP_TIME_MS 500     // 生产者线程每500毫秒以内生产一个新项目
#define CONSUMER_SLEEP_TIME_MS 2000    // 消费者线程每2000毫秒以内消费一个项目

// 全局变量
HWND g_hwndDlg;

LONG  g_lBuffer[QUEUE_SIZE];           // 生产者线程和消费者线程共用的缓冲区
LONG  g_lLastItemProduced;             // 生产者线程所生产项目的编号
ULONG g_ulQueueStartOffset;            // 项目在队列中的起始偏移
ULONG g_ulQueueCurrentSize;            // 当前队列大小

CONDITION_VARIABLE g_cvBufferNotFull;  // 生产者线程所等待的条件变量
CONDITION_VARIABLE g_cvBufferNotEmpty; // 消费者线程所等待的条件变量
CRITICAL_SECTION   g_csBufferLock;     // 关键段，用于同步对共享资源的访问

BOOL  g_bStopRequested;                // 主线程是否要求停止工作

// 函数声明
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// 生产者线程函数
DWORD WINAPI ProducerThreadProc(LPVOID lpParameter);
// 消费者线程函数
DWORD WINAPI ConsumerThreadProc(LPVOID lpParameter);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, NULL);
   return 0;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   static HANDLE hProducer1, hConsumer1, hConsumer2;

   switch (uMsg)
   {
   case WM_INITDIALOG:
      g_hwndDlg = hwndDlg;

      // 初始化关键段和条件变量
      InitializeCriticalSection(&g_csBufferLock);
      InitializeConditionVariable(&g_cvBufferNotEmpty);
      InitializeConditionVariable(&g_cvBufferNotFull);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_START:
         // 创建1个生产者线程和2个消费者线程
         hProducer1 = CreateThread(NULL, 0, ProducerThreadProc, (LPVOID)1, 0, NULL);
         hConsumer1 = CreateThread(NULL, 0, ConsumerThreadProc, (LPVOID)1, 0, NULL);
         hConsumer2 = CreateThread(NULL, 0, ConsumerThreadProc, (LPVOID)2, 0, NULL);
         EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_START), FALSE);
         break;

      case IDC_BTN_STOP:
         EnterCriticalSection(&g_csBufferLock);
         g_bStopRequested = TRUE;
         LeaveCriticalSection(&g_csBufferLock);

         WakeAllConditionVariable(&g_cvBufferNotFull);
         WakeAllConditionVariable(&g_cvBufferNotEmpty);

         if (hProducer1 != NULL)
         {
            CloseHandle(hProducer1);
            hProducer1 = NULL;
         }
         if (hConsumer1 != NULL)
         {
            CloseHandle(hConsumer1);
            hConsumer1 = NULL;
         }
         if (hConsumer2 != NULL)
         {
            CloseHandle(hConsumer2);
            hConsumer2 = NULL;
         }

         EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_STOP), FALSE);
         break;
      }
      return TRUE;

   case WM_CLOSE:
      EndDialog(hwndDlg, 0);
      DeleteCriticalSection(&g_csBufferLock);
      return TRUE;
   }

   return FALSE;
}

//////////////////////////////////////////////////////////////////////////
DWORD WINAPI ProducerThreadProc(LPVOID lpParameter)
{
   TCHAR szBuf[64] = { 0 };

   // 传递过来的生产者线程编号
   ULONG ulProducerId = (ULONG)(ULONG_PTR)lpParameter;

   while (TRUE)
   {
      // 每500毫秒以内生产一个新项目，ulItem是所生产的项目编号
      Sleep(rand() % PRODUCER_SLEEP_TIME_MS);
      ULONG ulItem = InterlockedIncrement(&g_lLastItemProduced);

      // 获取关键段对象的所有权
      EnterCriticalSection(&g_csBufferLock);

      // 当队列已满的时候，释放关键段并进入睡眠状态，以便消费者线程可以消费项目以减小队列大小
      // 只要队列未满，就继续生产新项目
      while (g_ulQueueCurrentSize == QUEUE_SIZE && g_bStopRequested == FALSE)
         SleepConditionVariableCS(&g_cvBufferNotFull, &g_csBufferLock, INFINITE);

      // “所需的条件”已经成立(队列未满)
      // 如果主线程要求停止工作，释放关键段对象的所有权并退出循环
      if (g_bStopRequested == TRUE)
      {
         LeaveCriticalSection(&g_csBufferLock);
         break;
      }

      // 在队列末尾插入新项目，增加当前队列大小
      g_lBuffer[(g_ulQueueStartOffset + g_ulQueueCurrentSize) % QUEUE_SIZE] = ulItem;
      g_ulQueueCurrentSize++;

      wsprintf(szBuf, TEXT("生产者 %u: 项目编号 %2d, 当前队列大小 %2u\r\n"),
         ulProducerId, ulItem, g_ulQueueCurrentSize);
      SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_SETSEL, -1, -1);
      SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_REPLACESEL, TRUE, (LPARAM)szBuf);

      // 释放关键段对象的所有权
      LeaveCriticalSection(&g_csBufferLock);
      // 唤醒一个消费者线程以消费项目
      WakeConditionVariable(&g_cvBufferNotEmpty);
   }

   // 生产者线程退出
   wsprintf(szBuf, TEXT("生产者 %u 已经退出\r\n"), ulProducerId);
   SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_SETSEL, -1, -1);
   SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_REPLACESEL, TRUE, (LPARAM)szBuf);
   return 0;
}

DWORD WINAPI ConsumerThreadProc(LPVOID lpParameter)
{
   TCHAR szBuf[64] = { 0 };

   // 传递过来的消费者线程编号
   ULONG ulConsumerId = (ULONG)(ULONG_PTR)lpParameter;

   while (TRUE)
   {
      // 获取关键段对象的所有权
      EnterCriticalSection(&g_csBufferLock);

      // 当队列为空的时候，释放关键段并进入睡眠状态以便生产者线程可以生产项目
      // 只要队列不为空，就继续消费项目
      while (g_ulQueueCurrentSize == 0 && g_bStopRequested == FALSE)
         SleepConditionVariableCS(&g_cvBufferNotEmpty, &g_csBufferLock, INFINITE);

      // “所需的条件”已经成立(队列不为空)
      // 如果主线程要求停止工作并且当前队列大小为空，释放关键段对象的所有权并退出循环
      if (g_bStopRequested == TRUE && g_ulQueueCurrentSize == 0)
      {
         LeaveCriticalSection(&g_csBufferLock);
         break;
      }

      // lItem是生产者线程所生产的项目编号，从队列开头开始消费项目，
      // 减小当前队列大小，增加项目在队列中的偏移
      LONG lItem = g_lBuffer[g_ulQueueStartOffset];
      g_ulQueueCurrentSize--;
      g_ulQueueStartOffset++;

      if (g_ulQueueStartOffset == QUEUE_SIZE)
         g_ulQueueStartOffset = 0;

      wsprintf(szBuf, TEXT("消费者 %u: 项目编号 %2d, 当前队列大小 %2u\r\n"),
         ulConsumerId, lItem, g_ulQueueCurrentSize);
      SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_SETSEL, -1, -1);
      SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_REPLACESEL, TRUE, (LPARAM)szBuf);

      // 释放关键段对象的所有权
      LeaveCriticalSection(&g_csBufferLock);

      // 唤醒生产者线程
      WakeConditionVariable(&g_cvBufferNotFull);

      // 每2000毫秒以内消费一个项目
      Sleep(rand() % CONSUMER_SLEEP_TIME_MS);
   }

   // 消费者线程退出
   wsprintf(szBuf, TEXT("消费者 %u 已经退出\r\n"), ulConsumerId);
   SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_SETSEL, -1, -1);
   SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_REPLACESEL, TRUE, (LPARAM)szBuf);
   return 0;
}