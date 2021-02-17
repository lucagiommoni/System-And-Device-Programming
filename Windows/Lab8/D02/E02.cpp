#define _CRT_SECURE_NO_DEPRECATE
#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif

#define MAX_CHARS 30

typedef struct STUDENT {
	INT id;
	DWORD reg;
	TCHAR name[MAX_CHARS + 1];		// +1 is for null terminator
	TCHAR surname[MAX_CHARS + 1];	// +1 is for null terminator
	INT mark;
};

// Expands to main or wmain depending on definition of _UNICODE
// This assures correct operations in all combinations
int _tmain(INT argc, LPTSTR argv[]) {
	if ( argc < 3 ) {
		_ftprintf( stderr, _T( "USAGE: %s file1 file2" ), argv[0] );
		return 1;
	}

	HANDLE hIn, hOut;
	FILE* fileIn;
	DWORD nIn, nOut;
	STUDENT student;

	fileIn = _tfopen(argv[1], _T("rt")); // read text

	if ( fileIn == NULL ) {
		_ftprintf( stderr, _T( "Cannot open input file. Error: %x\n" ), GetLastError() );
		return 2;
	}

	// Open the output file
	hOut = CreateFile(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Cannot open input file. Error: %x\n"), GetLastError());
		return 3;
	}

	while (_ftscanf(fileIn, _T("%d %ld %s %s %d"), &student.id, &student.reg, &student.name, &student.surname, &student.mark) == 5) {

		if (!WriteFile(hOut, &student, sizeof(student), &nOut, NULL) || nOut != sizeof(student)) {
			_ftprintf(stderr, _T("Cannot write correctly the output. Error: %x\n"), GetLastError());
			fclose(fileIn);
			CloseHandle(hOut);
			return 4;
		}
	}

	fclose(fileIn);
	CloseHandle(hOut);

	// Open the new created file
	hIn = CreateFile(argv[2], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Cannot open input file. Error: %x\n"), GetLastError());
		return 5;
	}

	while (ReadFile(hIn, &student, sizeof(student), &nIn, NULL) && nIn > 0) {
		if (nIn != sizeof(student)) {
			_ftprintf(stderr, _T("Error reading, file shorter than expected\n"));
			CloseHandle(hIn);
			return 6;
		}
		_tprintf(_T("%d %ld %s %s %d\n"), student.id, student.reg, student.name, student.surname, student.mark);
	}

	CloseHandle(hIn);

	return 0;
}
