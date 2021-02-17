#define _CRT_SECURE_NO_DEPRECATE
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
};

typedef FileInfoStruct* LPFileInfoStruct;
CRITICAL_SECTION cs;

// generate bin files with random integers
VOID binFilesWithRandomInt(UINT numOfFiles);

// sorting algo
VOID bubblesort(LPUINT array, UINT size);

// Thread routine
DWORD WINAPI threadSortFile(LPVOID param);


int _tmain(INT argc, LPTSTR argv[]) {

	LPHANDLE arrayHandle;
	LPFileInfoStruct lpFileInfoStruct;
	UINT numberThreads;
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
	arrayHandle = (LPHANDLE)malloc((numberThreads) * sizeof(HANDLE));

	// allocate the array of structures for data sharing with the threads
	lpFileInfoStruct = (LPFileInfoStruct)malloc((numberThreads) * sizeof(FileInfoStruct));

	// threads creation
	for (UINT i = 0; i < numberThreads; i++) {
		// save file name for each thread
		lpFileInfoStruct[i].fileName = argv[i+1];

		// create and start thread
		arrayHandle[i] = CreateThread(0, 0, threadSortFile, (LPVOID)&lpFileInfoStruct[i], 0, NULL);

		if (arrayHandle[i] == INVALID_HANDLE_VALUE) {
			_ftprintf(stderr, _T("Error creating thread %d\n"), i+1);
			return 2;
		}
	}

	// main thread waits
	WaitForMultipleObjects(numberThreads, arrayHandle, TRUE, INFINITE);

	for (UINT i = 0; i < numberThreads; i++) {
		CloseHandle(arrayHandle[i]);
	}

	free(arrayHandle);

	UINT finalArraySize = 0;

	for (UINT i = 0; i < numberThreads; i++)
	{
		finalArraySize += lpFileInfoStruct[i].size;
	}

	LPUINT finalArray = (LPUINT)malloc(finalArraySize * sizeof(UINT));

	for (UINT i = 0; i < numberThreads; i++)
	{
		for (UINT j = 0; j < lpFileInfoStruct[i].size; j++)
		{
			finalArray[i+j] = lpFileInfoStruct[i].outputArray[j];
		}
	}

	free(lpFileInfoStruct);

	bubblesort(finalArray, finalArraySize);

	// print to stdout
	_ftprintf(stdout, _T("Writing to file %s\n"), argv[argc - 1]);
	_ftprintf(stdout, _T("%d "), finalArraySize);

	for (UINT i = 0; i < finalArraySize; i++)
		_ftprintf(stdout, _T("%d "), finalArray[i]);

	_ftprintf(stdout, _T("\n"));

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
	for (UINT i = 0; i < size-2; i++)
	{
		if (array[i]>array[i+1])
		{
			tmp = array[i + 1];
			array[i + 1] = array[i];
			array[i] = tmp;
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
		_ftprintf(stderr, _T("Error opening file %s %x\n"), lpFilestructInfo->fileName, GetLastError());
		return 5;
	} else {
		_ftprintf(stdout, _T("Opened: %s\n"), lpFilestructInfo->fileName);
	}

	UINT num;

	if (!ReadFile(hIn, &num, sizeof(UINT), &nRead, NULL) || nRead != sizeof(UINT)) {
		_ftprintf(stderr, _T("Error %s %x\n"), lpFilestructInfo->fileName, GetLastError());
		return 6;
	} else {
		_ftprintf(stdout, _T("n=%d : "), num);
	}

	LPUINT arrayValues = (LPUINT)malloc(num * sizeof(UINT));

	for (UINT i = 0; i < num; i++) {
		if (!ReadFile(hIn, &arrayValues[i], sizeof(i), &nRead, NULL) || nRead != sizeof(i)) {
			_ftprintf(stderr, _T("Error reading values %s %x\n"), lpFilestructInfo->fileName, GetLastError());
			return 7;
		} else {
			_ftprintf(stdout, _T("%d "), arrayValues[i]);
		}

	}
	_ftprintf(stdout, _T("\n"));
	LeaveCriticalSection(&cs);

	//sort array
	bubblesort(arrayValues, num);

	// set the results inside the structure
	lpFilestructInfo->size = num;
	lpFilestructInfo->outputArray = arrayValues;

	// release resources
	CloseHandle(hIn);

	// then return 0
	return 0;
}

VOID binFilesWithRandomInt(UINT numOfFiles) {

	TCHAR filename[10];

	for (UINT i = 0; i < numOfFiles; i++)
	{
		_stscanf(filename, _T("file_%d"), i);

		HANDLE hOut = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hOut == INVALID_HANDLE_VALUE) {
			_ftprintf(stderr, _T("Error opening file %s %x\n"), filename, GetLastError());
			exit(10);
		}

		srand((UINT)time(NULL));
		UINT numInt = rand()%100;
		DWORD nOut;

		if (!WriteFile(hOut, &numInt, sizeof(UINT), &nOut, NULL) || nOut != sizeof(UINT)) {
			_ftprintf(stderr, _T("Error %x\n"), GetLastError());
			CloseHandle(hOut);
			exit(11);
		}

		UINT n;

		for (UINT i = 0; i < numInt; i++)
		{
			n = rand() % 100;
			if (!WriteFile(hOut, &n, sizeof(UINT), &nOut, NULL) || nOut != sizeof(UINT)) {
				_ftprintf(stderr, _T("Error %x\n"), GetLastError());
				CloseHandle(hOut);
				exit(11);
			}
		}

		CloseHandle(hOut);
	}
}
