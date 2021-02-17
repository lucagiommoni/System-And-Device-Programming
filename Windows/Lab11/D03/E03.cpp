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

UINT timeL2R, timeR2L, timeARRIVAL, nL2R, nR2L, actualL2R, actualR2L;
LPHANDLE producers, consumers;
HANDLE l2rThread, r2lThread, busy;

CRITICAL_SECTION cs_l2r, cs_r2l, output;

DWORD WINAPI l2rRoutine(LPVOID);
DWORD WINAPI r2lRoutine(LPVOID);
DWORD WINAPI traverseBridgeL2R(LPVOID);
DWORD WINAPI traverseBridgeR2L(LPVOID);

UINT _tmain(UINT argc, TCHAR* argv[]) {

	if (argc != 6)
	{
		_ftprintf(stderr, _T("USAGE: %s <timeL2R> <timeR2L> <timeARRIVAL> <nL2R> <nR2L>\n"), argv[0]);
		exit(1);
	}

	timeL2R = _ttoi(argv[1]);
	timeR2L = _ttoi(argv[2]);
	timeARRIVAL = _ttoi(argv[3]);
	nL2R = _ttoi(argv[4]);
	nR2L = _ttoi(argv[5]);

	actualL2R = actualR2L = 0;

	srand((UINT)time(NULL));

	InitializeCriticalSection(&cs_l2r);
	InitializeCriticalSection(&cs_r2l);
	InitializeCriticalSection(&output);

	busy = CreateSemaphore(0, 1, 1, 0);

	if (busy == NULL || busy == INVALID_HANDLE_VALUE)
	{
		_ftprintf(stderr, _T("Error creating busy\n"));
		return 1;
	}

	l2rThread = CreateThread(0, 0, l2rRoutine, 0, TRUE, 0);

	if (l2rThread == NULL || l2rThread == INVALID_HANDLE_VALUE)
	{
		_ftprintf(stderr, _T("Error creating l2rThread\n"));
		return 1;
	}

	r2lThread = CreateThread(0, 0, r2lRoutine, 0, TRUE, 0);

	if (r2lThread == NULL || r2lThread == INVALID_HANDLE_VALUE)
	{
		EnterCriticalSection(&output);
		_ftprintf(stderr, _T("Error creating r2lThreaad\n"));
		LeaveCriticalSection(&output);
		return 1;
	}

	if (WaitForSingleObject(l2rThread, INFINITE) != WAIT_OBJECT_0 || WaitForSingleObject(r2lThread, INFINITE) != WAIT_OBJECT_0)
	{
		_ftprintf(stderr, _T("MAIN - Error waiting threads\n"));
		return 1;
	}

	return 0;
}

DWORD WINAPI l2rRoutine(LPVOID) {

	LPHANDLE l2rCars = (LPHANDLE)malloc(nL2R * sizeof(HANDLE));

	for (UINT i = 0; i < nL2R; i++)
	{
		Sleep(rand() % timeARRIVAL);

		l2rCars[i] = CreateThread(0, 0, traverseBridgeL2R, 0, TRUE, 0);

		if (l2rCars[i] == NULL || l2rCars[i] == INVALID_HANDLE_VALUE)
		{
			EnterCriticalSection(&output);
			_ftprintf(stderr, _T("l2rRoutine - Error creating l2rCars[%d]\n"), i);
			LeaveCriticalSection(&output);
			exit(1);
		}
	}

	if (WaitForMultipleObjects(nL2R, l2rCars, TRUE, INFINITE) != WAIT_OBJECT_0)
	{
		EnterCriticalSection(&output);
		_ftprintf(stderr, _T("l2rRoutine - Error waiting threads\n"));
		LeaveCriticalSection(&output);
		exit(1);
	}

	return 0;
}

DWORD WINAPI r2lRoutine(LPVOID) {

	LPHANDLE r2lCars = (LPHANDLE)malloc(nR2L * sizeof(HANDLE));

	for (UINT i = 0; i < nR2L; i++)
	{
		Sleep(rand() % timeARRIVAL);

		r2lCars[i] = CreateThread(0, 0, traverseBridgeR2L, 0, TRUE, 0);

		if (r2lCars[i] == NULL || r2lCars[i] == INVALID_HANDLE_VALUE)
		{
			EnterCriticalSection(&output);
			_ftprintf(stderr, _T("r2lRoutine - Error creating r2lCars[%d]\n"), i);
			LeaveCriticalSection(&output);
			exit(1);
		}
	}

	if (WaitForMultipleObjects(nR2L, r2lCars, TRUE, INFINITE) != WAIT_OBJECT_0)
	{
		EnterCriticalSection(&output);
		_ftprintf(stderr, _T("r2lRoutine - Error waiting threads\n"));
		LeaveCriticalSection(&output);
		exit(1);
	}

	return 0;
}

DWORD WINAPI traverseBridgeL2R(LPVOID) {
	DWORD res;
	EnterCriticalSection(&cs_l2r);
	actualL2R++;
	if (actualL2R == 1)
	{
		res = WaitForSingleObject(busy, INFINITE);
		if (res == WAIT_FAILED)
		{
			EnterCriticalSection(&output);
			_ftprintf(stderr, _T("%ld - Error waiting busy: %ld\n"), GetCurrentThreadId(), GetLastError());
			_flushall();
			LeaveCriticalSection(&output);
			exit(1);
		}
	}
	EnterCriticalSection(&output);
	_ftprintf(stdout, _T("Car l2r arrived - actualL2R = %d\n"), actualL2R);
	_flushall();
	LeaveCriticalSection(&output);
	LeaveCriticalSection(&cs_l2r);

	Sleep(timeL2R);

	EnterCriticalSection(&cs_l2r);
	EnterCriticalSection(&output);
	_ftprintf(stdout, _T("Car l2r passed - actualL2R = %d\n"), actualL2R);
	_flushall();
	LeaveCriticalSection(&output);
	actualL2R--;
	if (actualL2R == 0)
	{
		if (!ReleaseSemaphore(busy, 1, 0))
		{
			EnterCriticalSection(&output);
			_ftprintf(stderr, _T("ReleaseMutex Error = %ld\n"), GetLastError());
			_flushall();
			LeaveCriticalSection(&output);
			exit(1);
		}
	}
	LeaveCriticalSection(&cs_l2r);

	return 0;
}

DWORD WINAPI traverseBridgeR2L(LPVOID) {
	DWORD res;
	EnterCriticalSection(&cs_r2l);
	actualR2L++;
	if (actualR2L == 1)
	{
		res = WaitForSingleObject(busy, INFINITE);
		if (res == WAIT_FAILED)
		{
			EnterCriticalSection(&output);
			_ftprintf(stderr, _T("%ld - Error waiting busy: %ld\n"), GetCurrentThreadId(), GetLastError());
			_flushall();
			LeaveCriticalSection(&output);
			exit(1);
		}
	}
	EnterCriticalSection(&output);
	_ftprintf(stdout, _T("Car r2l arrived - actualR2L = %d\n"), actualR2L);
	_flushall();
	LeaveCriticalSection(&output);
	LeaveCriticalSection(&cs_r2l);

	Sleep(timeR2L);

	EnterCriticalSection(&cs_r2l);
	EnterCriticalSection(&output);
	_ftprintf(stdout, _T("Car r2l passed - actualR2L = %d\n"), actualR2L);
	_flushall();
	LeaveCriticalSection(&output);
	actualR2L--;
	if (actualR2L == 0)
	{
		if (!ReleaseSemaphore(busy, 1, 0))
		{
			EnterCriticalSection(&output);
			_ftprintf(stderr, _T("ReleaseMutex Error = %ld\n"), GetLastError());
			_flushall();
			LeaveCriticalSection(&output);
			exit(1);
		}
	}
	LeaveCriticalSection(&cs_r2l);

	return 0;
}
