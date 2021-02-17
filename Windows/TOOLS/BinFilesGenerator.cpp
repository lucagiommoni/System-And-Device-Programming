#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
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

int _tmain(INT argc, LPTSTR argv[]) {

	UINT numOfFiles;

	if (argc != 2 || _stscanf(argv[1], _T("%u"), &numOfFiles) != 1) {
		_ftprintf(stderr, _T("USAGE: %s <number of files to generate>\n"), argv[0]);
		_ftprintf(stderr, _T("%s\n"), argv[1]);
		exit(1);
	}

	TCHAR filename[10];

	for (UINT i = 0; i < numOfFiles; i++)
	{
		_stprintf(filename, _T("file_%u"), i);

		HANDLE hOut = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hOut == INVALID_HANDLE_VALUE) {
			_ftprintf(stderr, _T("Error opening file %s %x\n"), filename, GetLastError());
			exit(10);
		}

		srand((UINT)time(NULL));
		UINT numInt = rand() % 100;
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

	return 0;
}
