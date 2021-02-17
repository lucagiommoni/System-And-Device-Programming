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
#define MAX_PATH 128

CRITICAL_SECTION cs;

DWORD WINAPI TraverseDirectory(LPVOID param);

DWORD FileType(LPWIN32_FIND_DATA);

INT _tmain(INT argc, LPTSTR argv[]) {

	if (argc < 2) {
		_ftprintf(stderr, _T("Usage: %s <list folders>\n"), argv[0]);
		return 1;
	}

	DWORD nThreads = argc - 1;
	LPHANDLE tHandles = (LPHANDLE)malloc(nThreads * sizeof(HANDLE));
	LPDWORD tIds = (LPDWORD)malloc(nThreads * sizeof(LPDWORD));

	InitializeCriticalSection(&cs);

	for (DWORD i = 0; i < nThreads; i++)
	{
		tHandles[i] = CreateThread(0, 0, TraverseDirectory, argv[i + 1], 0, &tIds[i]);
		if (tHandles[i] == INVALID_HANDLE_VALUE)
		{
			_ftprintf(stderr, _T("Error CreateThread %d: %x\n"), i, GetLastError());
			return 2;
		}
	}

	WaitForMultipleObjects(nThreads, tHandles, TRUE, INFINITE);

	return 0;
}

DWORD WINAPI TraverseDirectory(LPVOID PathName) {

	WIN32_FIND_DATA FindData;
	DWORD FType;

	SetCurrentDirectory((LPTSTR)PathName);

	HANDLE SearchHandle = FindFirstFile(_T("*"), &FindData);

	do {
		FType = FileType(&FindData);
		if (FType == TYPE_DIR) {
			EnterCriticalSection(&cs);
			_tprintf(_T("TID: %u - DIR: %s\n"), GetCurrentThreadId(), FindData.cFileName);
			LeaveCriticalSection(&cs);
			TraverseDirectory(FindData.cFileName);
		}
	} while (FindNextFile(SearchHandle, &FindData));

	FindClose(SearchHandle);

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
