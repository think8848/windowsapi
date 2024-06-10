#include <windows.h>
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// ��������
#define QUEUE_SIZE             10      // ���д�С
#define PRODUCER_SLEEP_TIME_MS 500     // �������߳�ÿ500������������һ������Ŀ
#define CONSUMER_SLEEP_TIME_MS 2000    // �������߳�ÿ2000������������һ����Ŀ

// ȫ�ֱ���
HWND g_hwndDlg;

LONG  g_lBuffer[QUEUE_SIZE];           // �������̺߳��������̹߳��õĻ�����
LONG  g_lLastItemProduced;             // �������߳���������Ŀ�ı��
ULONG g_ulQueueStartOffset;            // ��Ŀ�ڶ����е���ʼƫ��
ULONG g_ulQueueCurrentSize;            // ��ǰ���д�С

CONDITION_VARIABLE g_cvBufferNotFull;  // �������߳����ȴ�����������
CONDITION_VARIABLE g_cvBufferNotEmpty; // �������߳����ȴ�����������
CRITICAL_SECTION   g_csBufferLock;     // �ؼ��Σ�����ͬ���Թ�����Դ�ķ���

BOOL  g_bStopRequested;                // ���߳��Ƿ�Ҫ��ֹͣ����

// ��������
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
// �������̺߳���
DWORD WINAPI ProducerThreadProc(LPVOID lpParameter);
// �������̺߳���
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

      // ��ʼ���ؼ��κ���������
      InitializeCriticalSection(&g_csBufferLock);
      InitializeConditionVariable(&g_cvBufferNotEmpty);
      InitializeConditionVariable(&g_cvBufferNotFull);
      return TRUE;

   case WM_COMMAND:
      switch (LOWORD(wParam))
      {
      case IDC_BTN_START:
         // ����1���������̺߳�2���������߳�
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

   // ���ݹ������������̱߳��
   ULONG ulProducerId = (ULONG)(ULONG_PTR)lpParameter;

   while (TRUE)
   {
      // ÿ500������������һ������Ŀ��ulItem������������Ŀ���
      Sleep(rand() % PRODUCER_SLEEP_TIME_MS);
      ULONG ulItem = InterlockedIncrement(&g_lLastItemProduced);

      // ��ȡ�ؼ��ζ��������Ȩ
      EnterCriticalSection(&g_csBufferLock);

      // ������������ʱ���ͷŹؼ��β�����˯��״̬���Ա��������߳̿���������Ŀ�Լ�С���д�С
      // ֻҪ����δ�����ͼ�����������Ŀ
      while (g_ulQueueCurrentSize == QUEUE_SIZE && g_bStopRequested == FALSE)
         SleepConditionVariableCS(&g_cvBufferNotFull, &g_csBufferLock, INFINITE);

      // ��������������Ѿ�����(����δ��)
      // ������߳�Ҫ��ֹͣ�������ͷŹؼ��ζ��������Ȩ���˳�ѭ��
      if (g_bStopRequested == TRUE)
      {
         LeaveCriticalSection(&g_csBufferLock);
         break;
      }

      // �ڶ���ĩβ��������Ŀ�����ӵ�ǰ���д�С
      g_lBuffer[(g_ulQueueStartOffset + g_ulQueueCurrentSize) % QUEUE_SIZE] = ulItem;
      g_ulQueueCurrentSize++;

      wsprintf(szBuf, TEXT("������ %u: ��Ŀ��� %2d, ��ǰ���д�С %2u\r\n"),
         ulProducerId, ulItem, g_ulQueueCurrentSize);
      SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_SETSEL, -1, -1);
      SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_REPLACESEL, TRUE, (LPARAM)szBuf);

      // �ͷŹؼ��ζ��������Ȩ
      LeaveCriticalSection(&g_csBufferLock);
      // ����һ���������߳���������Ŀ
      WakeConditionVariable(&g_cvBufferNotEmpty);
   }

   // �������߳��˳�
   wsprintf(szBuf, TEXT("������ %u �Ѿ��˳�\r\n"), ulProducerId);
   SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_SETSEL, -1, -1);
   SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_REPLACESEL, TRUE, (LPARAM)szBuf);
   return 0;
}

DWORD WINAPI ConsumerThreadProc(LPVOID lpParameter)
{
   TCHAR szBuf[64] = { 0 };

   // ���ݹ������������̱߳��
   ULONG ulConsumerId = (ULONG)(ULONG_PTR)lpParameter;

   while (TRUE)
   {
      // ��ȡ�ؼ��ζ��������Ȩ
      EnterCriticalSection(&g_csBufferLock);

      // ������Ϊ�յ�ʱ���ͷŹؼ��β�����˯��״̬�Ա��������߳̿���������Ŀ
      // ֻҪ���в�Ϊ�գ��ͼ���������Ŀ
      while (g_ulQueueCurrentSize == 0 && g_bStopRequested == FALSE)
         SleepConditionVariableCS(&g_cvBufferNotEmpty, &g_csBufferLock, INFINITE);

      // ��������������Ѿ�����(���в�Ϊ��)
      // ������߳�Ҫ��ֹͣ�������ҵ�ǰ���д�СΪ�գ��ͷŹؼ��ζ��������Ȩ���˳�ѭ��
      if (g_bStopRequested == TRUE && g_ulQueueCurrentSize == 0)
      {
         LeaveCriticalSection(&g_csBufferLock);
         break;
      }

      // lItem���������߳�����������Ŀ��ţ��Ӷ��п�ͷ��ʼ������Ŀ��
      // ��С��ǰ���д�С��������Ŀ�ڶ����е�ƫ��
      LONG lItem = g_lBuffer[g_ulQueueStartOffset];
      g_ulQueueCurrentSize--;
      g_ulQueueStartOffset++;

      if (g_ulQueueStartOffset == QUEUE_SIZE)
         g_ulQueueStartOffset = 0;

      wsprintf(szBuf, TEXT("������ %u: ��Ŀ��� %2d, ��ǰ���д�С %2u\r\n"),
         ulConsumerId, lItem, g_ulQueueCurrentSize);
      SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_SETSEL, -1, -1);
      SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_REPLACESEL, TRUE, (LPARAM)szBuf);

      // �ͷŹؼ��ζ��������Ȩ
      LeaveCriticalSection(&g_csBufferLock);

      // �����������߳�
      WakeConditionVariable(&g_cvBufferNotFull);

      // ÿ2000������������һ����Ŀ
      Sleep(rand() % CONSUMER_SLEEP_TIME_MS);
   }

   // �������߳��˳�
   wsprintf(szBuf, TEXT("������ %u �Ѿ��˳�\r\n"), ulConsumerId);
   SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_SETSEL, -1, -1);
   SendMessage(GetDlgItem(g_hwndDlg, IDC_EDIT_INFO), EM_REPLACESEL, TRUE, (LPARAM)szBuf);
   return 0;
}