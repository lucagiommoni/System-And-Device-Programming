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

#define products 10
#define intRange 100

UINT nProd, nCons;
LPHANDLE producers, consumers;
UINT timeIntervall;

CRITICAL_SECTION cs_R, cs_W;

typedef struct QUEUE {
	UINT head, tail;
	UINT size;
	UINT *buffer;
	HANDLE semFull, semEmpty;
} QUEUE;

QUEUE queue;

DWORD WINAPI prodRoutine(LPVOID);
DWORD WINAPI consRoutine(LPVOID);

UINT _tmain(UINT argc, TCHAR* argv[]) {

	if (argc != 5)
	{
		_ftprintf(stderr, _T("USAGE: %s <Producers> <Consumers> <Queue Size> <Time Interval(s)>\n"), argv[0]);
		exit(1);
	}

	timeIntervall = _ttoi(argv[4]);
	srand((unsigned)time(NULL));

	queue.size = _ttoi(argv[3]);
	queue.buffer = (UINT*)malloc(queue.size * sizeof(UINT));
	queue.tail = queue.head = 0;
	queue.semEmpty = CreateSemaphore(0, queue.size, queue.size, 0);
	queue.semFull = CreateSemaphore(0, 0, queue.size, 0);

	InitializeCriticalSection(&cs_R);
	InitializeCriticalSection(&cs_W);

	nProd = _ttoi(argv[1]);
	nCons = _ttoi(argv[2]);

	producers = (LPHANDLE)malloc(nProd * sizeof(HANDLE));
	consumers = (LPHANDLE)malloc(nCons * sizeof(HANDLE));

	// Create producers
	for (UINT i = 0; i < nProd; i++)
	{
		producers[i] = CreateThread(0, 0, prodRoutine, 0, TRUE, NULL);

		if (producers[i] == NULL || producers[i] == INVALID_HANDLE_VALUE)
		{
			_ftprintf(stderr, _T("Error creating producers[%u] - %ld\n"), i, GetLastError());
			exit(1);
		}
	}

	// Create consumers
	for (UINT i = 0; i < nCons; i++)
	{
		consumers[i] = CreateThread(0, 0, consRoutine, 0, TRUE, NULL);

		if (consumers[i] == NULL || producers[i] == INVALID_HANDLE_VALUE)
		{
			_ftprintf(stderr, _T("Error creating consumers[%u] - %ld\n"), i, GetLastError());
			exit(1);
		}
	}

	if (WaitForMultipleObjects(nProd, producers, TRUE, INFINITE) != WAIT_OBJECT_0)
	{
		_ftprintf(stderr, _T("MAIN - Error waiting for producers threads - %ld\n"), GetLastError());
		return 1;
	}

	if (WaitForSingleObject(queue.semEmpty, INFINITE) != WAIT_OBJECT_0)
	{
		_ftprintf(stderr, _T("MAIN - Error waiting for semEmpty - %ld\n"), GetLastError());
		return 1;
	}

	EnterCriticalSection(&cs_W);

	queue.buffer[queue.head] = intRange + 1;
	queue.head = (queue.head + 1) % queue.size;

	LeaveCriticalSection(&cs_W);

	ReleaseSemaphore(queue.semFull, 1, 0);

	if (WaitForMultipleObjects(nCons, consumers, TRUE, INFINITE) != WAIT_OBJECT_0)
	{
		_ftprintf(stderr, _T("MAIN - Error waiting for consumers threads - %ld\n"), GetLastError());
		return 1;
	}

	return 0;
}

DWORD WINAPI prodRoutine(LPVOID) {

	UINT s, num;

	for (UINT i = 0; i < products; i++)
	{
		s = rand() % timeIntervall;
		Sleep(s * 1000);

		num = rand() % intRange;

		if (WaitForSingleObject(queue.semEmpty, INFINITE) != WAIT_OBJECT_0)
		{
			_ftprintf(stderr, _T("%ld - Error waiting for semEmpty - %ld\n"), GetCurrentThreadId(), GetLastError());
			return 1;
		}

		EnterCriticalSection(&cs_W);

		queue.buffer[queue.head] = num;
		queue.head = (queue.head + 1) % queue.size;

		LeaveCriticalSection(&cs_W);

		ReleaseSemaphore(queue.semFull, 1, 0);
	}

	return 0;
}

DWORD WINAPI consRoutine(LPVOID) {

	while (true)
	{
		if (WaitForSingleObject(queue.semFull, INFINITE) != WAIT_OBJECT_0)
		{
			_ftprintf(stderr, _T("%ld - Error waiting for semEmpty - %ld\n"), GetCurrentThreadId(), GetLastError());
			return 1;
		}

		EnterCriticalSection(&cs_R);

		if (queue.buffer[queue.tail] == (intRange + 1))
		{
			LeaveCriticalSection(&cs_R);
			ReleaseSemaphore(queue.semFull, 1, 0);
			return 0;
		}

		_tprintf(_T("%ld - read: %d\n"), GetCurrentThreadId(), queue.buffer[queue.tail]);
		queue.tail = (queue.tail + 1) % queue.size;

		LeaveCriticalSection(&cs_R);

		ReleaseSemaphore(queue.semEmpty, 1, 0);
	}

	return 0;
}
