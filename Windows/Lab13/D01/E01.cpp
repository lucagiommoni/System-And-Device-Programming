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

typedef struct TRDIN {
	UINT powerHigh, power;
} TRDIN;

typedef struct LAST {
	HANDLE sem;
	CRITICAL_SECTION cs;
	UINT num, max;
} LAST;

DWORD *terms;
LAST last;
HANDLE sem;
TCHAR filename[MAX_PATH];
LPHANDLE hThreads;

DWORD WINAPI threadRoutine(LPVOID);

UINT _tmain(UINT argc, TCHAR* argv[]) {

	if (argc != 2)
	{
		_ftprintf(stderr, _T("USAGE: %s <fileName>\n"), argv[0]);
		exit(1);
	}

	memset(filename, 0, MAX_PATH);
	_tcscpy(filename, argv[1]);

	HANDLE file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (file == NULL || file == INVALID_HANDLE_VALUE)
	{
		_ftprintf(stderr, _T("MAIN - Error CreateFile %s: %ld\n"), argv[1], GetLastError());
		exit(1);
	}

	DWORD nReads, nThreads;

	BOOL res = ReadFile(file, &nThreads, sizeof(nThreads), &nReads, 0);

	if (!res || nReads != sizeof(nThreads))
	{
		_ftprintf(stderr, _T("MAIN - Error ReadFile %s: %ld\n"), argv[1], GetLastError());
		exit(1);
	}

	sem = CreateSemaphore(0, 0, nThreads, 0);

	last.max = nThreads;
	InitializeCriticalSection(&last.cs);
	last.num = 0;
	last.sem = CreateSemaphore(0, 0, 1, 0);

	if (last.sem == NULL || last.sem == INVALID_HANDLE_VALUE)
	{
		_ftprintf(stderr, _T("MAIN - Error CreateSemaphore: %ld\n"), GetLastError());
		exit(1);
	}

	hThreads = (LPHANDLE)malloc(nThreads * sizeof(HANDLE));

	terms = (DWORD*)malloc((nThreads) * sizeof(DWORD));

	for (UINT i = 0; i < nThreads; i++)
	{
		TRDIN *trdin = (TRDIN*)malloc(sizeof(TRDIN));
		trdin->power = i;
		trdin->powerHigh = nThreads;

		hThreads[i] = CreateThread(0, 0, threadRoutine, trdin, TRUE, 0);

		if (hThreads[i] == NULL || hThreads[i] == INVALID_HANDLE_VALUE)
		{
			_ftprintf(stderr, _T("MAIN - Error CreateThread\n: %ld"), GetLastError());
			exit(1);
		}
	}

	if (WaitForMultipleObjects(nThreads, hThreads, TRUE, INFINITE) != WAIT_OBJECT_0)
	{
		_ftprintf(stderr, _T("MAIN - Error WaitForMultipleObjects\n: %ld"), GetLastError());
		exit(1);
	}

	return 0;
}

DWORD WINAPI threadRoutine(LPVOID param) {

	TRDIN trdin = *((TRDIN*)(param));
	OVERLAPPED ov = { 0,0,0,0,NULL };
	LARGE_INTEGER filepos;
	HANDLE file;
	DWORD nTx, *buffer, bufflen;
	BOOL res;

	filepos.QuadPart = sizeof(DWORD);
	ov.Offset = filepos.LowPart;
	ov.OffsetHigh = filepos.HighPart;

	bufflen = (trdin.powerHigh + 2) * sizeof(DWORD);
	buffer = (DWORD*)malloc(bufflen);

	file = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (file == NULL || file == INVALID_HANDLE_VALUE)
	{
		_ftprintf(stderr, _T("%ld - Error CreateFile %s: %ld\n"), GetCurrentThreadId(), filename, GetLastError());
		exit(1);
	}

	while (true)
	{
		// read v
		res = ReadFile(file, buffer, sizeof(bufflen), &nTx, &ov);

		//EOF
		if (res && nTx == 0) break;

		if (!res || nTx != sizeof(bufflen))
		{
			_ftprintf(stderr, _T("%%ld - Error ReadFile %s: %ld\n"), GetCurrentThreadId(), filename, GetLastError());
			exit(1);
		}

		for (INT i = 0; i < trdin.power-1; i++)
			buffer[0] *= buffer[0];

		terms[trdin.power-1] = buffer[trdin.power] * buffer[0];

		EnterCriticalSection(&last.cs);

		last.num++;

		if (last.num == 1)
		{
			LeaveCriticalSection(&last.cs);

			if (WaitForSingleObject(last.sem, INFINITE) != WAIT_OBJECT_0)
			{
				_ftprintf(stderr, _T("%ld - Error WaitForSingleObject: %ld\n"), GetCurrentThreadId(), GetLastError());
				exit(1);
			}

			buffer[trdin.powerHigh + 1] = buffer[1];

			for (UINT i = 0; i < trdin.powerHigh; i++)
				buffer[trdin.powerHigh + 1] += terms[i];

			if (!WriteFile(file, &buffer, sizeof(buffer), &nTx, &ov) || nTx != sizeof(buffer))
			{
				_ftprintf(stderr, _T("%ld - Error WriteFile %s: %ld\n"), GetCurrentThreadId(), filename, GetLastError());
				exit(1);
			}

			EnterCriticalSection(&last.cs);
			last.num = 0;
			LeaveCriticalSection(&last.cs);

			if (!ReleaseSemaphore(sem, trdin.powerHigh, 0))
			{
				_ftprintf(stderr, _T("%ld - Error ReleaseSemaphore: %ld\n"), GetCurrentThreadId(), GetLastError());
				exit(1);
			}

			continue;
		}
		else if (last.num == last.max)
		{
			if (!ReleaseSemaphore(last.sem, 1, 0))
			{
				_ftprintf(stderr, _T("%ld - Error ReleaseSemaphore: %ld\n"), GetCurrentThreadId(), GetLastError());
				exit(1);
			}
		}

		LeaveCriticalSection(&last.cs);
		filepos.QuadPart += sizeof(bufflen);

		if (!WaitForSingleObject(sem, INFINITE) != WAIT_OBJECT_0)
		{
			_ftprintf(stderr, _T("%ld - Error WaitForSingleObject: %ld\n"), GetCurrentThreadId(), GetLastError());
			exit(1);
		}

	}

	return 0;
}
