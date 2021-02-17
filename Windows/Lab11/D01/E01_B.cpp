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

#define SL 31

UINT nThreads;
LPHANDLE hThreads;
CRITICAL_SECTION cs;

DWORD accountSize;

typedef struct ACCDATA {
	INT index;
	DWORD bankAcc;
	TCHAR surname[SL];
	TCHAR name[SL];
	INT balance;
} ACCDATA;

DWORD WINAPI threadRoutine(LPVOID);

UINT _tmain(UINT argc, TCHAR* argv[]) {

	if (argc < 3)
	{
		_ftprintf(stderr, _T("USAGE: %s <ACCOUNT filepath> <OPERATION filepath> [... <OPERATION filepath>]\n"), argv[0]);
		exit(1);
	}

	struct _stat buffer;
	_tstat(argv[1], &buffer);
	accountSize = buffer.st_size;

	InitializeCriticalSection(&cs);

	nThreads = argc - 2;

	hThreads = (LPHANDLE)malloc(nThreads * sizeof(HANDLE));

	for (UINT i = 0; i < nThreads; i++)
	{
		hThreads[i] = CreateThread(0, 0, threadRoutine, (LPVOID)argv[i + 2], TRUE, NULL);

		if (hThreads[i] == NULL || hThreads[i] == INVALID_HANDLE_VALUE)
		{
			_ftprintf(stderr, _T("Error creating thread - %ld\n"), GetLastError());
			exit(1);
		}
	}

	if (WaitForMultipleObjects(nThreads, hThreads, TRUE, INFINITE) != WAIT_OBJECT_0)
	{
		_ftprintf(stderr, _T("Error waiting thread - %ld\n"), GetLastError());
		exit(1);
	}

	for (UINT i = 0; i < nThreads; i++) CloseHandle(hThreads[i]);

	HANDLE accFile = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (accFile == INVALID_HANDLE_VALUE)
	{
		_ftprintf(stderr, _T("Error opening %s - %ld\n"), argv[1], GetLastError());
		exit(1);
	}

	// Move pointer to the beginning of ACCOUNT file
	SetFilePointer(accFile, 0, 0, FILE_BEGIN);

	BOOL res;
	ACCDATA accdata;
	DWORD nRead;

	while (true)
	{
		FlushFileBuffers(accFile);
		res = ReadFile(accFile, &accdata, sizeof(accdata), &nRead, 0);

		if (res && nRead == 0) break;

		if (!res || nRead != sizeof(accdata))
		{
			_ftprintf(stderr, _T("MAIN - Cannot read ACCOUNT file - %ld\n"), GetLastError());
			return 1;
		}

		_tprintf(_T("%d %ld %s %s %d\n"), accdata.index, accdata.bankAcc, accdata.surname, accdata.name, accdata.balance);
	}

	CloseHandle(accFile);

	return 0;
}

DWORD WINAPI threadRoutine(LPVOID filePath) {
	OVERLAPPED ov = { 0,0,0,0,NULL };
	ACCDATA accdata, opdata;
	DWORD nRead, nWrite, counter;
	BOOL res;
	HANDLE accFile, opFile;

	// Open OPERATION file
	accFile = CreateFile(_T(".\\account.bin"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	opFile = CreateFile((TCHAR*)filePath, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);

	if (opFile == INVALID_HANDLE_VALUE)
	{
		_ftprintf(stderr, _T("%ld - Error opening %s - %ld\n"), GetCurrentThreadId(), (TCHAR*)filePath, GetLastError());
		return 1;
	}

	// Read OPERATION file
	while (true)
	{
		// Read one line from OPERATION file
		res = ReadFile(opFile, &opdata, sizeof(opdata), &nRead, 0);

		// EOF
		if (res && nRead == 0) break;

		// Error
		if (!res || nRead != sizeof(opdata))
		{
			_ftprintf(stderr, _T("%ld - Cannot read OPERATION file - %ld\n"), GetCurrentThreadId(), GetLastError());
			CloseHandle(opFile);
			return 1;
		}

		// Lock ACCOUNT file
		EnterCriticalSection(&cs);

		// Move pointer to the beginning of ACCOUNT file
		SetFilePointer(accFile, 0, 0, FILE_BEGIN);
		FlushFileBuffers(accFile);

		counter = 0;

		// Search the line, in ACCOUNT file, corresponding to the Bank ID read from OPERATION file
		while (true)
		{
			res = ReadFile(accFile, &accdata, sizeof(accdata), &nRead, 0);

			if (res && nRead == 0) break;

			if (!res || nRead != sizeof(accdata))
			{
				_ftprintf(stderr, _T("%ld - Cannot read ACCOUNT file - %ld\n"), GetCurrentThreadId(), GetLastError());
				CloseHandle(opFile);
				return 1;
			}

			if (accdata.bankAcc == opdata.bankAcc)
			{
				accdata.balance += opdata.balance;

				SetFilePointer(accFile, counter, 0, FILE_BEGIN);

				if (!WriteFile(accFile, &accdata, sizeof(accdata), &nWrite, 0) || nWrite != sizeof(accdata))
				{
					_ftprintf(stderr, _T("%ld - Cannot write ACCOUNT file - %ld\n"), GetCurrentThreadId(), GetLastError());
					CloseHandle(opFile);
					return 1;
				}

				break;
			}

			counter += sizeof(ACCDATA);

		}


		// Unlock ACCOUNT file
		LeaveCriticalSection(&cs);
	}

	return 0;
}
