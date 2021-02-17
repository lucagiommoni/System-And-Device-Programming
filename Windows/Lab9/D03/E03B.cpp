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

DWORD WINAPI TraverseDirectory(LPVOID param);

//static void TraverseDirectory(LPTSTR);

DWORD FileType(LPWIN32_FIND_DATA);

INT _tmain(INT argc, LPTSTR argv[]) {

	if (argc < 2) {
		_ftprintf(stderr, _T("Usage: %s <list folders>\n"), argv[0]);
		return 1;
	}

	DWORD nThreads = argc - 1;
	LPHANDLE tHandles = (LPHANDLE)malloc(nThreads * sizeof(HANDLE));
	LPDWORD tIds = (LPDWORD)malloc(nThreads * sizeof(LPDWORD));

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

	TCHAR buffer[MAX_PATH];
	DWORD nRead;
	TCHAR logFileName[MAX_PATH];
	HANDLE hLog;

	for (DWORD i = 0; i < nThreads; i++) {
		memset(logFileName, 0, MAX_PATH);
		_sntprintf(logFileName, MAX_PATH - 1, _T("log_%u.txt"), tIds[i]);

		hLog = CreateFile(logFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (hLog == INVALID_HANDLE_VALUE) {
			_ftprintf(stderr, _T("Error opening file log_%u.txt\n"), GetCurrentThreadId());
			return 3;
		}

		do {
			memset(logFileName, 0, MAX_PATH);
			_ftprintf(stdout, _T("%s"), buffer);
		} while (ReadFile(hLog, buffer, MAX_PATH * sizeof(TCHAR), &nRead, NULL) && nRead > 0);

		CloseHandle(hLog);
	}

	return 0;
}

DWORD WINAPI TraverseDirectory(LPVOID PathName) {

	WIN32_FIND_DATA FindData;
	DWORD FType;

	TCHAR dir2traverse[MAX_PATH];
	memset(dir2traverse, 0, MAX_PATH);
	_sntprintf(dir2traverse, MAX_PATH - 1, _T("%s\\*"), (LPTSTR)PathName);

	HANDLE SearchHandle = FindFirstFile(dir2traverse, &FindData);

	do {
		FType = FileType(&FindData);

		if (FType == TYPE_DIR) {

			TCHAR logFileName[MAX_PATH];
			memset(logFileName, 0, MAX_PATH);
			_sntprintf(logFileName, MAX_PATH - 1, _T("log_%u.txt"), GetCurrentThreadId());

			TCHAR dir2check[MAX_PATH];
			memset(dir2check, 0, MAX_PATH);
			_sntprintf(dir2check, MAX_PATH - 1, _T("%s\\%s"), (LPTSTR)PathName, FindData.cFileName);

			TCHAR buffer[MAX_PATH];
			memset(buffer, 0, MAX_PATH);
			_sntprintf(buffer, MAX_PATH - 1, _T("TID: %u - DIR: %s\n"), GetCurrentThreadId(), dir2check);
			DWORD buffLen = (DWORD)_tcscnlen(buffer, MAX_PATH);
			//_tprintf(_T("buffer: %s\n"), buffer);
			//_tprintf(_T("buffLen: %u\n"), buffLen);
			HANDLE hLog = CreateFile(logFileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

			if (hLog == INVALID_HANDLE_VALUE) {
				_ftprintf(stderr, _T("Error opening file log_%u.txt\n"), GetCurrentThreadId());
				return 3;
			}

			LARGE_INTEGER fileSize;

			if (!GetFileSizeEx(hLog, &fileSize)) {
				_ftprintf(stderr, _T("TID: %u - Error getting filesize\n"), GetCurrentThreadId());
				CloseHandle(hLog);
				return 4;
			}

			if (!SetFilePointerEx(hLog, fileSize, NULL, FILE_BEGIN)) {
				_ftprintf(stderr, _T("TID: %u - Error moving file pointer\n"), GetCurrentThreadId());
				CloseHandle(hLog);
				return 5;
			}

			DWORD nWrt;

			if (!WriteFile(hLog, buffer, buffLen * sizeof(TCHAR), &nWrt, NULL) || nWrt != buffLen * sizeof(TCHAR))
			{
				_ftprintf(stderr, _T("Error writing file log_%u.txt\n"), GetCurrentThreadId());
				CloseHandle(hLog);
				return 6;
			}

			CloseHandle(hLog);

			TraverseDirectory(dir2check);
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
