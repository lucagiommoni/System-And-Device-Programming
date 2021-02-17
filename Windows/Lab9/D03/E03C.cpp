#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif

#include "conio.h"

#define TYPE_FILE 1
#define TYPE_DIR 2
#define TYPE_DOT 3
#define MAX_PATH_LOCAL 512
#define MAX_BUFF_OUT 512

typedef struct OUTPUT {
	UINT head;
	UINT tail;
	UINT bufflen;
	LPTSTR *buffer;
	HANDLE semEmpty;
	HANDLE semFull;
};

OUTPUT output;
HANDLE outpuThreadHandle;
CRITICAL_SECTION cs;
DWORD activeThreads;

VOID printLog(LPTSTR log);

DWORD WINAPI TraverseDirectory(LPVOID);

DWORD WINAPI outputThread(LPVOID);

DWORD FileType(LPWIN32_FIND_DATA);

INT _tmain(INT argc, LPTSTR argv[]) {

	if (argc < 2) {
		_ftprintf(stderr, _T("Usage: %s <list folders>\n"), argv[0]);
		return 1;
	}

	output.head = 0;
	output.tail = 0;
	output.bufflen = 10;
	output.buffer = (LPTSTR*)malloc(output.bufflen * sizeof(LPTSTR));
	output.semEmpty = CreateSemaphore(NULL, output.bufflen, output.bufflen, NULL);
	output.semFull = CreateSemaphore(NULL, 0, output.bufflen, NULL);

	InitializeCriticalSection(&cs);

	// Create output thread and start it in suspended mode
	outpuThreadHandle = CreateThread(0, 0, outputThread, NULL, 0, NULL);
	if (outpuThreadHandle == INVALID_HANDLE_VALUE)
	{
		_ftprintf(stderr, _T("Error creating output thread\n"));
		return 8;
	}

	DWORD nThreads = argc - 1;
	LPHANDLE tHandles = (LPHANDLE)malloc(nThreads * sizeof(HANDLE));
	LPDWORD tIds = (LPDWORD)malloc(nThreads * sizeof(LPDWORD));
	activeThreads = 0;

	for (DWORD i = 0; i < nThreads; i++)
	{
		tHandles[i] = CreateThread(0, 0, TraverseDirectory, argv[i + 1], 0, &tIds[i]);
		if (tHandles[i] == INVALID_HANDLE_VALUE)
		{
			_ftprintf(stderr, _T("Error CreateThread %d: %x\n"), i, GetLastError());
			return 2;
		}
		activeThreads++;
	}

	WaitForMultipleObjects(nThreads, tHandles, TRUE, INFINITE);
	WaitForSingleObject(outpuThreadHandle, 3000);

	return 0;
}

DWORD WINAPI TraverseDirectory(LPVOID PathName) {
	WIN32_FIND_DATA FindData;
	DWORD FType;

	TCHAR dir2traverse[MAX_PATH_LOCAL];
	memset(dir2traverse, 0, MAX_PATH_LOCAL);
	_sntprintf(dir2traverse, MAX_PATH_LOCAL - 1, _T("%s\\*"), (LPTSTR)PathName);

	HANDLE SearchHandle = FindFirstFile(dir2traverse, &FindData);

	do {
		FType = FileType(&FindData);

		if (FType == TYPE_DIR)
		{
			TCHAR dir2check[MAX_PATH_LOCAL];
			memset(dir2check, 0, MAX_PATH_LOCAL);
			_sntprintf(dir2check, MAX_PATH_LOCAL - 1, _T("%s\\%s"), (LPTSTR)PathName, FindData.cFileName);

			LPTSTR buffer = (LPTSTR)malloc(MAX_BUFF_OUT * sizeof(TCHAR));
			memset(buffer, 0, MAX_BUFF_OUT * sizeof(TCHAR));
			_sntprintf(buffer, (sizeof(TCHAR) * (MAX_BUFF_OUT - 1)), _T("TID: %u - DIR: %s\n"), GetCurrentThreadId(), dir2check);
			printLog(buffer);

			TraverseDirectory(dir2check);
		}
	} while (FindNextFile(SearchHandle, &FindData));

	FindClose(SearchHandle);

	activeThreads--;
	return 0;
}

static DWORD FileType(LPWIN32_FIND_DATA pFileData) {

	DWORD FType = TYPE_FILE;
	BOOL IsDir = (pFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	if (IsDir)
		if (lstrcmp(pFileData->cFileName, _T(".")) == 0 ||
			lstrcmp(pFileData->cFileName, _T("..")) == 0) {
			FType = TYPE_DOT;
		} else {
			FType = TYPE_DIR;
		}

	return FType;
}

DWORD WINAPI outputThread(LPVOID param) {
	DWORD semret;
	BOOL flag = true;
	while (flag)
	{
		switch ((semret = WaitForSingleObject(output.semFull, 10)))
		{
		case WAIT_TIMEOUT:
			if (activeThreads > 0) {
				continue;
			} else {
				flag = false;
				break;
			}
		case WAIT_OBJECT_0:
			_tprintf(_T("%s\n"), output.buffer[output.tail]);
			output.tail++;
			output.tail %= output.bufflen;
			ReleaseSemaphore(output.semEmpty, 1, NULL);
			break;
		case WAIT_FAILED:
			_ftprintf(stderr, _T("Error waiting semaphore: WAIT_FAILED\n"));
			return 11;
		case WAIT_ABANDONED:
			_ftprintf(stderr, _T("Error waiting semaphore: WAIT_ABANDONED\n"));
			return 12;
		default:
			_ftprintf(stderr, _T("Error waiting semaphore: %x"), GetLastError());
			return 13;
		}
	}

	return 0;
}

VOID printLog(LPTSTR log) {
	EnterCriticalSection(&cs);
	WaitForSingleObject(output.semEmpty, INFINITE);
	output.buffer[output.head] = log;
	output.head++;
	output.head %= output.bufflen;
	ReleaseSemaphore(output.semFull, 1, NULL);
	LeaveCriticalSection(&cs);
}
