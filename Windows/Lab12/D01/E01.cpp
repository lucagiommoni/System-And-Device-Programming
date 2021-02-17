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

#define DATETIME_LENGTH 20
#define DURATION_LENGTH 9

typedef struct SERVER {
	TCHAR path[MAX_PATH];
	HANDLE sem;
	UINT nReaders;
	CRITICAL_SECTION cs;
} SERVER;

typedef struct RECORD {
	UINT ip;
	TCHAR user[MAX_PATH];
	TCHAR datetime[DATETIME_LENGTH];
	TCHAR duration[DURATION_LENGTH];
} RECORD;

typedef struct DURATION {
	UINT hour, min, sec;
} DURATION;

typedef struct DATE {
	UINT year, month, day;
	DURATION time;
} DATE;

UINT nThreads, numberOfServers;
CRITICAL_SECTION printout;
SERVER* servers;
LPHANDLE hThreads;

DWORD WINAPI threadRoutine(LPVOID);
VOID reader(UINT serverNum);
VOID writer(UINT serverNum);
static DWORD FileType(LPWIN32_FIND_DATA pFileData);

UINT _tmain(UINT argc, TCHAR* argv[]) {

	if (argc != 3)
	{
		_ftprintf(stderr, _T("USAGE: %s <fileName> <nThreads>\n"), argv[0]);
		exit(1);
	}

	// init random seed
	srand((UINT)time(NULL));

	InitializeCriticalSection(&printout);

	nThreads = _ttoi(argv[2]);

	HANDLE serverList = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (serverList == NULL || serverList == INVALID_HANDLE_VALUE)
	{
		_ftprintf(stderr, _T("MAIN - Error CreateFile %s\n: %ld"), argv[1], GetLastError());
		exit(1);
	}

	DWORD nReads;

	if (ReadFile(serverList, &numberOfServers, sizeof(numberOfServers), &nReads, 0) || nReads != sizeof(numberOfServers))
	{
		_ftprintf(stderr, _T("MAIN - Error ReadFile %s\n: %ld"), argv[1], GetLastError());
		exit(1);
	}

	servers = (SERVER*)malloc(numberOfServers * sizeof(SERVER));

	for (UINT i = 0; i < numberOfServers; i++)
	{
		if (ReadFile(serverList, servers[i].path, sizeof(servers[i].path), &nReads, 0) || nReads != sizeof(servers[i].path))
		{
			_ftprintf(stderr, _T("MAIN - Error ReadFile %s\n: %ld"), argv[1], GetLastError());
			exit(1);
		}

		servers[i].sem = CreateSemaphore(NULL, 1, 1, NULL);

		if (servers[i].sem == NULL || servers[i].sem == INVALID_HANDLE_VALUE)
		{
			_ftprintf(stderr, _T("MAIN - Error CreateSemaphore\n: %ld"), GetLastError());
			exit(1);
		}

		servers[i].nReaders = 0;

		InitializeCriticalSection(&servers[i].cs);
	}

	hThreads = (LPHANDLE)malloc(nThreads * sizeof(HANDLE));

	for (UINT i = 0; i < nThreads; i++)
	{
		hThreads[i] = CreateThread(NULL, 0, threadRoutine, NULL, TRUE, NULL);

		if (hThreads[i] == NULL || hThreads[i] == INVALID_HANDLE_VALUE)
		{
			_ftprintf(stderr, _T("MAIN - Error CreateThread\n: %ld"), GetLastError());
			exit(1);
		}
	}

	DWORD res = WaitForMultipleObjects(nThreads, hThreads, TRUE, INFINITE);

	if (res != WAIT_OBJECT_0)
	{
		_ftprintf(stderr, _T("MAIN - Error WaitForMultipleObjects\n: %ld"), GetLastError());
		exit(1);
	}

	return 0;
}

DWORD WINAPI threadRoutine(LPVOID) {

	INT n1 = rand() % 10 / 10,
		n2 = rand() % 10 / 10,
		serverID = ((rand() % 10 / 10) * numberOfServers);

	Sleep(n1 * 100);

	// reader
	if (n2 < .5)
	{
		reader(serverID);
	}

	// writer
	else
	{
		writer(serverID);
	}

	return 0;
}

VOID reader(INT serverID) {

	DWORD res;

	EnterCriticalSection(&servers[serverID].cs);

	servers[serverID].nReaders++;

	if (servers[serverID].nReaders == 1)
	{
		res = WaitForSingleObject(servers[serverID].sem, INFINITE);

		if (res != WAIT_OBJECT_0)
		{
			EnterCriticalSection(&printout);
			_ftprintf(stderr, _T("%ld - Error WaitForSingleObject\n: %ld"), GetCurrentThreadId(), GetLastError());
			LeaveCriticalSection(&printout);
			exit(1);
		}
	}

	LeaveCriticalSection(&servers[serverID].cs);

	// read directory files
	WIN32_FIND_DATA FindData;
	HANDLE SearchHandle = FindFirstFile(servers[serverID].path, &FindData);
	DWORD FType, nRead;
	RECORD record;
	HANDLE accessFile;
	DURATION totDuration = { 0,0,0 };
	struct lastConn {
		UINT lcn;
		TCHAR lcs[DATETIME_LENGTH];
	} lastConn;

	lastConn.lcn = 0;

	do {
		FType = FileType(&FindData);

		if (FType != TYPE_FILE) continue;

		accessFile = CreateFile((TCHAR*)FindData.cFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

		if (accessFile == NULL || accessFile == INVALID_HANDLE_VALUE)
		{
			EnterCriticalSection(&printout);
			_ftprintf(stderr, _T("%ld - Error CreateFile\n: %ld"), GetCurrentThreadId(), GetLastError());
			LeaveCriticalSection(&printout);
			exit(1);
		}

		while (true)
		{
			BOOL hasRead = ReadFile(accessFile, &record, sizeof(record), &nRead, 0);

			// EOF
			if (hasRead && nRead == 0) break;

			if (!hasRead || nRead != sizeof(record))
			{
				EnterCriticalSection(&printout);
				_ftprintf(stderr, _T("%ld - Error ReadFile\n: %ld"), GetCurrentThreadId(), GetLastError());
				LeaveCriticalSection(&printout);
				exit(1);
			}

			UINT lct = 0;
			for (UINT i = DATETIME_LENGTH - 1; i >= 0; i++)
			{
				lct += _ttoi(&record.datetime[i]) * 10 ^ i;
			}

			if (lct > lastConn.lcn)
			{
				lastConn.lcn = lct;
				_tcscpy(lastConn.lcs, record.datetime);
			}

			totDuration.hour += (record.duration[0] * 10 + record.duration[1]);
			totDuration.min += (record.duration[3] * 10 + record.duration[4]);
			totDuration.sec += (record.duration[6] * 10 + record.duration[7]);
		}

		CloseHandle(accessFile);

	} while (FindNextFile(SearchHandle, &FindData));

	EnterCriticalSection(&printout);
	_tprintf(_T("%ld - Total connection time for server %u = %u:%u:%u \n"), GetCurrentThreadId(), serverID, totDuration.hour, totDuration.min, totDuration.sec);
	_tprintf(_T("%ld - Last connection for server %u = %s \n"), GetCurrentThreadId(), serverID, lastConn.lcs);
	LeaveCriticalSection(&printout);

	FindClose(SearchHandle);

	EnterCriticalSection(&servers[serverID].cs);

	servers[serverID].nReaders--;

	if (servers[serverID].nReaders == 0)
	{
		if (!ReleaseSemaphore(servers[serverID].sem, 1, NULL))
		{
			EnterCriticalSection(&printout);
			_ftprintf(stderr, _T("%ld - Error ReleaseSemaphore\n: %ld"), GetCurrentThreadId(), GetLastError());
			LeaveCriticalSection(&printout);
			exit(1);
		}
	}

	LeaveCriticalSection(&servers[serverID].cs);
}

VOID writer(INT serverID) {

	DWORD res = WaitForSingleObject(servers[serverID].sem, INFINITE);

	if (res != WAIT_OBJECT_0)
	{
		EnterCriticalSection(&printout);
		_ftprintf(stderr, _T("%ld - Error WaitForSingleObject\n: %ld"), GetCurrentThreadId(), GetLastError());
		LeaveCriticalSection(&printout);
		exit(1);
	}

	// read directory files
	WIN32_FIND_DATA FindData;
	HANDLE SearchHandle = FindFirstFile(servers[serverID].path, &FindData);
	DWORD FType, nProcessed;
	RECORD record;
	HANDLE accessFile;
	DURATION totDuration = { 0,0,0 };
	struct lastConn {
		UINT lcn;
		TCHAR lcs[DATETIME_LENGTH];
	} lastConn;

	lastConn.lcn = 0;

	do {
		FType = FileType(&FindData);

		if (FType != TYPE_FILE) continue;

		accessFile = CreateFile((TCHAR*)FindData.cFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

		if (accessFile == NULL || accessFile == INVALID_HANDLE_VALUE)
		{
			EnterCriticalSection(&printout);
			_ftprintf(stderr, _T("%ld - Error CreateFile\n: %ld"), GetCurrentThreadId(), GetLastError());
			LeaveCriticalSection(&printout);
			exit(1);
		}

		UINT count = 0;

		while (true)
		{
			BOOL hasRead = ReadFile(accessFile, &record, sizeof(record), &nProcessed, 0);

			// EOF
			if (hasRead && nProcessed == 0) break;

			if (!hasRead || nProcessed != sizeof(record))
			{
				EnterCriticalSection(&printout);
				_ftprintf(stderr, _T("%ld - Error ReadFile\n: %ld"), GetCurrentThreadId(), GetLastError());
				LeaveCriticalSection(&printout);
				exit(1);
			}

			_stprintf(record.datetime + 11, _T("%d%d:%d%d:%d%d"), rand() % 24, rand() % 60, rand() % 60);
			_stprintf(record.duration, _T("%d%d:%d%d:%d%d"), rand() % 100, rand() % 60, rand() % 60);

			if (SetFilePointer(accessFile, count, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
			{
				EnterCriticalSection(&printout);
				_ftprintf(stderr, _T("%ld - Error SetFilePointer\n: %ld"), GetCurrentThreadId(), GetLastError());
				LeaveCriticalSection(&printout);
				exit(1);
			}

			if (!WriteFile(accessFile, &record, sizeof(record), &nProcessed, 0) || nProcessed == sizeof(record))
			{
				EnterCriticalSection(&printout);
				_ftprintf(stderr, _T("%ld - Error WriteFile\n: %ld"), GetCurrentThreadId(), GetLastError());
				LeaveCriticalSection(&printout);
				exit(1);
			}

			count += sizeof(record);
		}

		CloseHandle(accessFile);

	} while (FindNextFile(SearchHandle, &FindData));

	FindClose(SearchHandle);

	if (!ReleaseSemaphore(servers[serverID].sem, 1, NULL))
	{
		EnterCriticalSection(&printout);
		_ftprintf(stderr, _T("%ld - Error ReleaseSemaphore\n: %ld"), GetCurrentThreadId(), GetLastError());
		LeaveCriticalSection(&printout);
		exit(1);
	}
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
