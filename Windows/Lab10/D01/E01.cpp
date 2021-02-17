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

struct FileInfoStruct {
	LPTCH fileName;
	UINT size;
	LPUINT outputArray;
	BOOL done;
};

typedef FileInfoStruct* LPFileInfoStruct;
CRITICAL_SECTION cs;

// sorting algo
VOID bubblesort(LPUINT array, UINT size);

// Thread routine
DWORD WINAPI threadSortFile(LPVOID param);


int _tmain(INT argc, LPTSTR argv[]) {

	LPHANDLE arrayHandle = nullptr;
	LPFileInfoStruct lpFileInfoStruct = nullptr;
	UINT numberThreads;
	LPDWORD threadIDs = nullptr;
	HANDLE hOut;
	DWORD nOut;

	if (argc < 3) {
		_ftprintf(stderr, _T("USAGE: %s <input file(s)> <output file>"), argv[0]);
		return 1;
	}

	numberThreads = argc - 2;

	//protects printf, so that only one thread can print at a time
	InitializeCriticalSection(&cs);

	// allocate the handles for threads
	arrayHandle = (LPHANDLE)malloc(numberThreads * sizeof(HANDLE));
	threadIDs = (LPDWORD)malloc(numberThreads * sizeof(DWORD));

	// allocate the array of structures for data sharing with the threads
	lpFileInfoStruct = (LPFileInfoStruct)malloc((numberThreads) * sizeof(FileInfoStruct));

	// threads creation
	for (UINT i = 0; i < numberThreads; i++) {
		// save file name for each thread
		lpFileInfoStruct[i].fileName = argv[i + 1];
		lpFileInfoStruct[i].done = FALSE;

		// create and start thread
		arrayHandle[i] = CreateThread(0, 0, threadSortFile, (LPVOID)&lpFileInfoStruct[i], 0, &threadIDs[i]);

		if (arrayHandle[i] == INVALID_HANDLE_VALUE || arrayHandle[i] == NULL) {
			_ftprintf(stderr, _T("Error creating thread %d\n"), i + 1);
			return 2;
		}
	}

	// main thread waits
	UINT finalArraySize = 0;
	LPUINT finalArray = nullptr;
	DWORD ret;
	UINT EndedThreadIndex;

	for (UINT i = numberThreads; i > 0; i--) {
		ret = WaitForMultipleObjects(i, arrayHandle, FALSE, INFINITE);
		switch (ret) {
		case WAIT_ABANDONED_0:
			EnterCriticalSection(&cs);
			_ftprintf(stderr, _T("WAIT_ABANDONED_0"));
			LeaveCriticalSection(&cs);
			return 15;
		case WAIT_TIMEOUT:
			EnterCriticalSection(&cs);
			_ftprintf(stderr, _T("WAIT_TIMEOUT"));
			LeaveCriticalSection(&cs);
			return 16;
		case WAIT_FAILED:
			EnterCriticalSection(&cs);
			_ftprintf(stderr, _T("WAIT_FAILED: %ld"), GetLastError());
			LeaveCriticalSection(&cs);
			return 17;
		default:
			EndedThreadIndex = (UINT)ret - (UINT)WAIT_OBJECT_0;
			break;
		}

		CloseHandle(arrayHandle[EndedThreadIndex]);
		//arrayHandle[EndedThreadIndex] = arrayHandle[i-1];

		EnterCriticalSection(&cs);
		_tprintf(_T("Thread %ld Terminated.\n\n"), threadIDs[EndedThreadIndex]);
		LeaveCriticalSection(&cs);

		UINT shift = EndedThreadIndex;

		// first round
		if (i == numberThreads) {
			finalArray = lpFileInfoStruct[shift].outputArray;
			finalArraySize = lpFileInfoStruct[shift].size;
		} else {
			UINT oldSize = finalArraySize;
			LPUINT tmp = finalArray;

			while (lpFileInfoStruct[shift].done) shift++;
			finalArraySize += lpFileInfoStruct[shift].size;
			finalArray = (LPUINT)malloc(finalArraySize * sizeof(UINT));

			for (UINT j = 0; j < oldSize; j++)
				finalArray[j] = tmp[j];

			for (UINT j = 0; j < lpFileInfoStruct[shift].size; j++)
				finalArray[j + oldSize] = lpFileInfoStruct[shift].outputArray[j];

			free(tmp);
			bubblesort(finalArray, finalArraySize);
		}

		lpFileInfoStruct[shift].done = TRUE;

		if ((i - 1) == 0) break;

		LPHANDLE tmpHandle = arrayHandle;
		arrayHandle = (LPHANDLE)malloc((i - 1) * sizeof(HANDLE));

		LPDWORD tmpTID = threadIDs;
		threadIDs = (LPDWORD)malloc((i - 1) * sizeof(DWORD));

		for (UINT j = 0, k = 0; j < (i - 1); j++, k++) {
			if (k == EndedThreadIndex) k++;
			arrayHandle[j] = tmpHandle[k];
			threadIDs[j] = tmpTID[k];
		}

		free(tmpHandle);
		free(tmpTID);

	}

	free(arrayHandle);
	free(lpFileInfoStruct);

	// print to stdout
	_tprintf(_T("Writing to file %s\n\n"), argv[argc - 1]);
	_tprintf(_T("%d "), finalArraySize);

	for (UINT i = 0; i < finalArraySize; i++)
		_tprintf(_T("%d "), finalArray[i]);

	_tprintf(_T("\n\n"));

	// save final array to output file
	hOut = CreateFile(argv[argc - 1], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	if (hOut == INVALID_HANDLE_VALUE) {
		_tprintf(_T("Error creating file %s %x\n"), argv[argc - 1], GetLastError());
	}

	if (!WriteFile(hOut, &finalArraySize, sizeof(finalArraySize), &nOut, NULL) || nOut != sizeof(finalArraySize)) {
		_ftprintf(stderr, _T("Error writing %x\n"), GetLastError());
		free(finalArray);
		CloseHandle(hOut);
		return 3;
	}

	if (!WriteFile(hOut, finalArray, finalArraySize * sizeof(UINT), &nOut, NULL) || nOut != (finalArraySize * sizeof(UINT))) {
		_ftprintf(stderr, _T("Error %x\n"), GetLastError());
		free(finalArray);
		CloseHandle(hOut);
		return 4;
	}

	free(finalArray);
	CloseHandle(hOut);

	_ftprintf(stdout, _T("Press enter to continue...\n"));
	_gettchar();

	return 0;
}

VOID bubblesort(LPUINT array, UINT size) {
	UINT tmp;
	BOOL flag = true;

	while (flag) {

		flag = false;

		for (UINT i = 0; i < size - 1; i++) {
			if (array[i]>array[i + 1]) {
				flag = true;
				tmp = array[i + 1];
				array[i + 1] = array[i];
				array[i] = tmp;
			}
		}
	}
	return;
}

// used by thread to sort his file
DWORD WINAPI threadSortFile(LPVOID param) {
	LPFileInfoStruct lpFilestructInfo = (LPFileInfoStruct)param;
	HANDLE hIn;
	DWORD nRead;

	hIn = CreateFile(lpFilestructInfo->fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// protect printf, only one thread can print on stdout at a time
	EnterCriticalSection(&cs);

	if (hIn == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("%ld - Error opening file %s %x\n"), GetCurrentThreadId(), lpFilestructInfo->fileName, GetLastError());
		return 5;
	}
	else {
		_ftprintf(stdout, _T("%ld - Opened: %s\n\n"), GetCurrentThreadId(), lpFilestructInfo->fileName);
	}

	UINT num;

	if (!ReadFile(hIn, &num, sizeof(UINT), &nRead, NULL) || nRead != sizeof(UINT)) {
		_ftprintf(stderr, _T("%ld - Error %s %x\n"), GetCurrentThreadId(), lpFilestructInfo->fileName, GetLastError());
		return 6;
	} else {
		_tprintf(_T("%ld - n=%d :\n\n"), GetCurrentThreadId(), num);
	}

	LPUINT arrayValues = (LPUINT)malloc(num * sizeof(UINT));

	for (UINT i = 0; i < num; i++) {
		if (!ReadFile(hIn, &arrayValues[i], sizeof(i), &nRead, NULL) || nRead != sizeof(i)) {
			_ftprintf(stderr, _T("Error reading values %s %x\n"), lpFilestructInfo->fileName, GetLastError());
			return 7;
		}
		else {
			_tprintf(_T("%d "), arrayValues[i]);
		}

	}
	_tprintf(_T("\n\n"));
	LeaveCriticalSection(&cs);

	//sort array
	bubblesort(arrayValues, num);

	EnterCriticalSection(&cs);
	_tprintf(_T("%ld - The ordered array is:\n\n"), GetCurrentThreadId());

	for (UINT i = 0; i < num; i++)
		_tprintf(_T("%d "), arrayValues[i]);

	_tprintf(_T("\n\n"));
	LeaveCriticalSection(&cs);

	// set the results inside the structure
	lpFilestructInfo->size = num;
	lpFilestructInfo->outputArray = arrayValues;

	// release resources
	CloseHandle(hIn);

	// then return 0
	return 0;
}
