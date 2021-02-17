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

HANDLE compareOk;
LPHANDLE listReady;
CRITICAL_SECTION cs;
UINT nThreads;
HANDLE compareThread;
LPHANDLE readingThreads;
BOOL isEqual, *endThread, end;
TCHAR** dirItems;

DWORD WINAPI compareRoutine(LPVOID);
DWORD WINAPI readRoutine(LPVOID);
static DWORD FileType(LPWIN32_FIND_DATA pFileData);
VOID checkDir(TCHAR* path, UINT index);

UINT _tmain(UINT argc, TCHAR* argv[]) {
	if (argc < 3)
	{
		_ftprintf(stderr, _T("USAGE: %s <dirpath1> <dirpath2> [... <dirpathN>]"), argv[0]);
		exit(1);
	}

	nThreads = argc - 1;
	isEqual = TRUE;
	end = FALSE;

	InitializeCriticalSection(&cs);

	dirItems = (TCHAR**)malloc(nThreads * sizeof(TCHAR*));
	endThread = (BOOL*)malloc(nThreads * sizeof(BOOL));

	for (UINT i = 0; i < nThreads; i++)
	{
		endThread[i] = FALSE;
		dirItems[i] = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
		memset(dirItems[i], 0, MAX_PATH);
		_tcscpy(dirItems[i], argv[i + 1]);
		if (_tcscnlen(dirItems[i], MAX_PATH) == MAX_PATH)
		{
			_ftprintf(stderr, _T("%s exceed the %d size!"), argv[i + 1], MAX_PATH);
			exit(1);
		}
	}

	// ---------------- INIT EVENTS ----------------

	compareOk = CreateEvent(0, TRUE, FALSE, 0);

	if (compareOk == NULL || compareOk == INVALID_HANDLE_VALUE)
	{
		_ftprintf(stderr, _T("Failed creating compareOk event\n"));
		exit(1);
	}

	listReady = (LPHANDLE)malloc(nThreads * sizeof(HANDLE));

	if (listReady == NULL)
	{
		_ftprintf(stderr, _T("Failed listReady malloc\n"));
		exit(1);
	}

	for (UINT i = 0; i < nThreads; i++)
	{
		listReady[i] = CreateEvent(0, TRUE, FALSE, 0);
		if (listReady[i] == NULL || listReady[i] == INVALID_HANDLE_VALUE)
		{
			_ftprintf(stderr, _T("Failed creating listReady[%u] event\n"), i);
			exit(1);
		}
	}

	// ---------------- INIT THREADS ----------------

	compareThread = CreateThread(0, 0, compareRoutine, 0, 0, 0);

	if (compareThread == NULL || compareThread == INVALID_HANDLE_VALUE)
	{
		_ftprintf(stderr, _T("Failed creating compareThread thread\n"));
		exit(1);
	}

	readingThreads = (LPHANDLE)malloc(nThreads * sizeof(HANDLE));

	if (readingThreads == NULL)
	{
		_ftprintf(stderr, _T("Failed readingThreads malloc\n"));
		exit(1);
	}

	LPUINT index;

	for (UINT i = 0; i < nThreads; i++)
	{
		index = (LPUINT)malloc(sizeof(UINT));
		*index = i;
		readingThreads[i] = CreateThread(0, 0, readRoutine, (LPVOID)index, 0, 0);

		if (readingThreads[i] == NULL || readingThreads[i] == INVALID_HANDLE_VALUE)
		{
			_ftprintf(stderr, _T("Failed creating readingThreads[%u] thread\n"), i);
			exit(1);
		}
	}

	// ---------------- WAIT THREADS ----------------

	if (WaitForMultipleObjects(nThreads, readingThreads, TRUE, INFINITE) != WAIT_OBJECT_0)
	{
		_ftprintf(stderr, _T("Failed wainting readingThreads: %ld\n"), GetLastError());
		exit(1);
	}

	TerminateThread(compareThread, NULL);

	if (!isEqual)
	{
		_tprintf(_T("Directories do not have the same content!\n"));
	}
	else
	{
		_tprintf(_T("Directories have the same content!\n"));
	}

	// ---------------- CLEAN ----------------

	DeleteCriticalSection(&cs);

	CloseHandle(compareOk);

	for (UINT i = 0; i < nThreads; i++)
	{
		CloseHandle(listReady[i]);
	}

	return 0;
}

DWORD WINAPI compareRoutine(LPVOID) {
	while (1)
	{
		if (WaitForMultipleObjects(nThreads, listReady, TRUE, INFINITE) != WAIT_OBJECT_0)
		{
			_ftprintf(stderr, _T("Failed waiting listReady: %ld\n"), GetLastError());
			exit(1);
		}

		if (!isEqual) return 0;

		for (UINT i = 0; i < nThreads; i++)
		{
			if (endThread[i] == TRUE) end = TRUE;
			else
			{
				EnterCriticalSection(&cs);
				_tprintf(_T("Different number of directory items\n"));
				LeaveCriticalSection(&cs);
				if (end == TRUE) isEqual = FALSE;
			}
		}

		if (end)
		{
			PulseEvent(compareOk);
			return 0;
		}

		for (UINT i = 0; i < nThreads - 1; i++)
		{
			ResetEvent(listReady[i]);

			if (_tcscmp(dirItems[i], dirItems[i + 1]) != 0)
			{
				EnterCriticalSection(&cs);
				_tprintf(_T("Files %s and %s are not equal\n"), dirItems[i], dirItems[i + 1]);
				LeaveCriticalSection(&cs);
				isEqual = FALSE;
			}
			else
			{
				EnterCriticalSection(&cs);
				_tprintf(_T("Files %s and %s are equal\n"), dirItems[i], dirItems[i + 1]);
				LeaveCriticalSection(&cs);
			}
		}

		PulseEvent(compareOk);
	}

	return 0;
}


DWORD WINAPI readRoutine(LPVOID index) {

	UINT i = *((UINT*)index);

	checkDir(dirItems[i], i);

	EnterCriticalSection(&cs);
	endThread[i] = TRUE;
	LeaveCriticalSection(&cs);

	SetEvent(listReady[i]);

	return 0;
}

VOID checkDir(TCHAR* dirPath, UINT index) {

	if (!isEqual) return;

	TCHAR* currPath = (TCHAR*)malloc(MAX_PATH * sizeof(TCHAR));
	memset(currPath, 0, MAX_PATH);
	_tcscpy(currPath, dirPath);

	UINT len = _tcscnlen(currPath, MAX_PATH);

	if (len > MAX_PATH - 3)
	{
		EnterCriticalSection(&cs);
		_ftprintf(stderr, _T("Path %s exceed the %d size!"), currPath, MAX_PATH);
		LeaveCriticalSection(&cs);
		exit(1);
	}

	EnterCriticalSection(&cs);
	_tprintf(_T("%ld \t Start analysis of %s\n"), GetCurrentThreadId(), currPath);
	LeaveCriticalSection(&cs);

	if (currPath[len-1] == '\\')
	{
		_tcsncat(currPath, _T("*"), 2);
	}
	else
	{
		_tcsncat(currPath, _T("\*"), 1);
	}

	WIN32_FIND_DATA FindData;
	HANDLE SearchHandle = FindFirstFile(currPath, &FindData);
	DWORD FType;
	do {
		FType = FileType(&FindData);

		if (FType != TYPE_DIR && FType != TYPE_FILE) continue;

		memset(dirItems[index], 0, MAX_PATH);
		_tcscpy(dirItems[index], (TCHAR*)FindData.cFileName);

		EnterCriticalSection(&cs);
		_tprintf(_T("%ld - Checking %s\n"), GetCurrentThreadId(), dirItems[index]);
		LeaveCriticalSection(&cs);

		SetEvent(listReady[index]);

		if (WaitForSingleObject(compareOk, INFINITE) != WAIT_OBJECT_0)
		{
			_ftprintf(stderr, _T("Failed waiting compareOk: %ld\n"), GetLastError());
			exit(1);
		}

		if (FType == TYPE_DIR)
		{
			currPath[len + 1] = '\0';
			_tcscat(currPath, FindData.cFileName);

			checkDir(currPath, index);
		}
	} while (FindNextFile(SearchHandle, &FindData) && isEqual);

	FindClose(SearchHandle);
}

static DWORD FileType(LPWIN32_FIND_DATA pFileData) {
	BOOL IsDir;
	DWORD FType;
	FType = TYPE_FILE;
	IsDir = (pFileData->dwFileAttributes &
		FILE_ATTRIBUTE_DIRECTORY) != 0;
	if (IsDir)
		if (lstrcmp(pFileData->cFileName, _T(".")) == 0
			|| lstrcmp(pFileData->cFileName, _T("..")) == 0)
			FType = TYPE_DOT;
		else FType = TYPE_DIR;
		return FType;
}
