// ---------------- START HEADERS ----------------
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include "conio.h"

#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif

#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif

#define TYPE_FILE 1
#define TYPE_DIR 2
#define TYPE_DOT 3

// ---------------- END HEADERS ----------------

#include "stdafx.h"

typedef struct PARTIAL {
  // Array that stores partial product
  DWORD *part;

  // Array of flag that states which
  // partial product is available
  BOOL *ready;

  // To synchronize access to ready flags
  CRITICAL_SECTION cs;

  DWORD counter;

  // Semaphore that signal if some partial product
  // is available
  HANDLE isReady;
} PARTIAL;

typedef struct RECORD {
  DWORD f1, f2, res;
} RECORD;

DWORD nThreads;
TCHAR dirName[MAX_PATH];
LPHANDLE hThreads;
HANDLE summThread;
PARTIAL partial;
HANDLE sumReady;

static DWORD FileType(LPWIN32_FIND_DATA pFileData);
DWORD WINAPI product(LPVOID);
DWORD WINAPI summation(LPVOID);

UINT _tmain(UINT argc, TCHAR* argv[]) {

  if (argc != 3)
  {
    _ftprintf(stderr, _T("USAGE: %s <n> <dirName>\n"), argv[0]);
    exit(1);
  }

  nThreads = _ttoi(argv[1]);

  memset(dirName, 0, MAX_PATH);
  _tcscpy(dirName, argv[2]);

  // init partial
  partial.part = (DWORD*)malloc(nThreads * sizeof(DWORD));
  partial.ready = (BOOL*)malloc(nThreads * sizeof(BOOL));
  memset(partial.ready, FALSE, nThreads);

  if ((partial.isReady = CreateSemaphore(0, 0, nThreads, 0)) == INVALID_HANDLE_VALUE) {
    _ftprintf(stderr, _T("MAIN - Error CreateSemaphore: %ld\n"), GetLastError());
    exit(1);
  }

  InitializeCriticalSection(&partial.cs);

  sumReady = CreateEvent(0, TRUE, FALSE, 0);

  // create product threads
  hThreads = (LPHANDLE)malloc(nThreads * sizeof(HANDLE));

  for (DWORD i = 0; i < nThreads; i++) {
    DWORD *fnum = (DWORD*)malloc(sizeof(DWORD));
    *fnum = i;
    if ((hThreads[i] = CreateThread(0, 0, product, fnum, TRUE, 0)) == INVALID_HANDLE_VALUE) {
      _ftprintf(stderr, _T("MAIN - Error CreateSemaphore: %ld\n"), GetLastError());
      exit(1);
    }
  }

  // create summation thread
  if ((summThread = CreateThread(0, 0, summation, 0, TRUE, 0)) == INVALID_HANDLE_VALUE) {
    _ftprintf(stderr, _T("MAIN - Error CreateThread: %ld\n"), GetLastError());
    exit(1);
  }

  if (WaitForSingleObject(summThread, INFINITE) != WAIT_OBJECT_0) {
    _ftprintf(stderr, _T("MAIN - Error WaitForSingleObject: %ld\n"), GetLastError());
    exit(1);
  }

  return 0;
}

DWORD WINAPI product(LPVOID param) {

  DWORD index = *((DWORD*)(param));
  WIN32_FIND_DATA FindData;
  HANDLE file, SearchHandle;
  DWORD nTx;
  BOOL res;
  RECORD record;

  SearchHandle = FindFirstFile(dirName, &FindData);

  do
  {
    if (FileType(&FindData) != TYPE_FILE) continue;

    file = CreateFile(FindData.cFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    while (true)
    {
      res = ReadFile(file, &record, sizeof(record), &nTx, 0);

      // EOF
      if (res && nTx == 0) break;

      if (!res || nTx != sizeof(record))
      {
        _ftprintf(stderr, _T("%ld - Error ReadFile: %ld\n"), GetCurrentThreadId(), GetLastError());
        exit(1);
      }

      DWORD digit = record.f2;

      for (DWORD i = 0; i < index; i++) digit /= 10;

      digit %= 10;

      partial.part[index] = record.f1 * digit;

      for (DWORD i = 0; i < index; i++) partial.part[index] *= 10;

      if (!ReleaseSemaphore(partial.isReady, 1, 0))
      {
        _ftprintf(stderr, _T("%ld - Error ReleaseSemaphore: %ld\n"), GetCurrentThreadId(), GetLastError());
        exit(1);
      }

      if (WaitForSingleObject(sumReady, INFINITE) != WAIT_OBJECT_0)
      {
        _ftprintf(stderr, _T("%ld - Error WaitForSingleObject: %ld\n"), GetCurrentThreadId(), GetLastError());
        exit(1);
      }
    }

    CloseHandle(file);

  } while (FindNextFile(SearchHandle, &FindData));

  FindClose(SearchHandle);

  return 0;
}

DWORD WINAPI summation(LPVOID param) {

  WIN32_FIND_DATA FindData;
  HANDLE file, SearchHandle;
  DWORD nTx, result;
  BOOL res;
  RECORD record;

  SearchHandle = FindFirstFile(dirName, &FindData);

  do
  {
    if (FileType(&FindData) != TYPE_FILE) continue;

    file = CreateFile(FindData.cFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    while (true)
    {
      res = ReadFile(file, &record, sizeof(record), &nTx, 0);

      // EOF
      if (res && nTx == 0) break;

      if (!res || nTx != sizeof(record))
      {
        _ftprintf(stderr, _T("%ld - Error ReadFile: %ld\n"), GetCurrentThreadId(), GetLastError());
        exit(1);
      }

      while (true)
      {
        if (WaitForSingleObject(partial.isReady, INFINITE) != WAIT_OBJECT_0)
        {
          _ftprintf(stderr, _T("%ld - Error WaitForSingleObject: %ld\n"), GetCurrentThreadId(), GetLastError());
          exit(1);
        }

        for (DWORD i = 0; i < nThreads; i++)
        {
          if (partial.ready[i]) {
            partial.ready[i] = FALSE;
            result += partial.part[i];
            break;
          }
        }

        EnterCriticalSection(&partial.cs);
        partial.counter--;
        if (partial.counter == 0) break;
        LeaveCriticalSection(&partial.cs);
      }

      partial.counter = nThreads;
      LeaveCriticalSection(&partial.cs);

      record.res = result;

      if (SetFilePointer(file, -sizeof(record), 0, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
      {
        _ftprintf(stderr, _T("%ld - Error SetFilePointer: %ld\n"), GetCurrentThreadId(), GetLastError());
        exit(1);
      }

      if (!WriteFile(file, &record, sizeof(record), &nTx, 0) || nTx != sizeof(record))
      {
        _ftprintf(stderr, _T("%ld - Error WriteFile: %ld\n"), GetCurrentThreadId(), GetLastError());
        exit(1);
      }

      if (!PulseEvent(sumReady))
      {
        _ftprintf(stderr, _T("%ld - Error WaitForSingleObject: %ld\n"), GetCurrentThreadId(), GetLastError());
        exit(1);
      }
    }

    CloseHandle(file);

  } while (FindNextFile(SearchHandle, &FindData));

  FindClose(SearchHandle);

  return 0;
}

static DWORD FileType(LPWIN32_FIND_DATA pFileData) {
  if ((pFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
  return (lstrcmp(pFileData->cFileName, _T(".")) == 0 || lstrcmp(pFileData->cFileName, _T("..")) == 0) ? TYPE_DOT : TYPE_DIR;
  return TYPE_FILE;
}
