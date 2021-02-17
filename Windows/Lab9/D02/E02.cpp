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

#define TYPE_FILE 1
#define TYPE_DIR 2
#define TYPE_DOT 3
#define MAX_PATH 128

DWORD getFileType(LPWIN32_FIND_DATA pointerFileData);
VOID copyTreeDirectory(LPTSTR path1, LPTSTR path2);
VOID attachDataFields(LPTSTR path1, LPTSTR path2);

int _tmain(INT argc, LPTSTR argv[]) {

	if (argc != 3) {
		_ftprintf(stderr, _T("USAGE: %s srcDir destDir\n"), argv[0]);
		return 1;
	}

	copyTreeDirectory(argv[1], argv[2]);

	_ftprintf(stdout, _T("\nPress enter to continue...\n"));
	_gettchar();

	return 0;
}

//copy the directory name1 into an isomorphic directory name2
VOID copyTreeDirectory(LPTSTR path1, LPTSTR path2) {

	WIN32_FIND_DATA findFileData;
	TCHAR searchPath[MAX_PATH];
	TCHAR isomorphicPath1[MAX_PATH], isomorphicPath2[MAX_PATH];
	DWORD fileType;

	// build the searchPath string
	_sntprintf(searchPath, MAX_PATH - 1, _T("%s\\*"), path1);
	searchPath[MAX_PATH - 1] = 0;

	// create isomorphic directory in the destination subtree
	if (!CreateDirectory(path2, NULL)) {
		_ftprintf(stderr, _T("Error creating directory %s %x\n"), path2, GetLastError());
		exit(2);
	}

	// search inside path1
	HANDLE handleFile = FindFirstFile(searchPath, &findFileData);
	if (handleFile == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error FindFirstFile %x\n"), GetLastError());
		return;
	}

	do {
		// append the entry to path1
		_sntprintf(isomorphicPath1, MAX_PATH, _T("%s\\%s"), path1, findFileData.cFileName);
		// append the entry to path2
		_sntprintf(isomorphicPath2, MAX_PATH, _T("%s\\%s"), path2, findFileData.cFileName);

		// check file type
		fileType = getFileType(&findFileData);

		if (fileType == TYPE_FILE) {
			_tprintf(_T("File: %s\\%s\n"), path1, findFileData.cFileName);
			// add the two data fields
			attachDataFields(isomorphicPath1, isomorphicPath2);
		}
		
		if (fileType == TYPE_DIR) {
			_tprintf(_T("Directory: %s\\%s\n"), path1, findFileData.cFileName);
			// do recursion for subdirectory
			copyTreeDirectory(isomorphicPath1, isomorphicPath2);
		}
	} while (FindNextFile(handleFile, &findFileData));

	FindClose(handleFile);
	return;
}

/*
attaching these values to file:
- the first one, the name of the file (C string)
- the second one specifies the size of the original files
*/
VOID attachDataFields(LPTSTR path1, LPTSTR path2) {

	// read path1
	HANDLE hIn = CreateFile(path1, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hIn == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error opening file %s %x\n"), path1, GetLastError());
		return;
	}

	LARGE_INTEGER fileSize; // contains file size in bytes

	if (!GetFileSizeEx(hIn, &fileSize)) {
		_ftprintf(stderr, _T("Error getting filesize for file %s\n"), path1);
		CloseHandle(hIn);
		return;
	}

	// assuming size is less than 2^32 = 4gb
	LPTCH srcFileCont = (LPTCH)malloc(fileSize.LowPart);

	if (srcFileCont == NULL) {
		_ftprintf(stderr, _T("Error memory allocation %s %x\n"), path1, GetLastError());
		CloseHandle(hIn);
		return;
	}

	DWORD nRead;

	// store content of file in path1 inside srcFileCont buffer
	if (!ReadFile(hIn, srcFileCont, fileSize.LowPart, &nRead, NULL) || nRead != fileSize.LowPart) {
		_ftprintf(stderr, _T("Error reading content of src file %s %x\n"), path1, GetLastError());
		CloseHandle(hIn);
		return;
	}

	CloseHandle(hIn);

	// length of path1
	// strnlen returns the number of characters in the string pointed to by path1,
	//excluding the terminating null byte('\0'), but at most MAX_PATH
	DWORD fileNameLength = (DWORD)_tcsnlen(path1, MAX_PATH);

	// write to path2
	HANDLE hOut = CreateFile(path2, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hOut == INVALID_HANDLE_VALUE) {
		_ftprintf(stderr, _T("Error opening file %s %x\n"), path2, GetLastError());
		CloseHandle(hIn);
		return;
	}

	DWORD nWrote;

	// attach to path2 the filename
	if (!WriteFile(hOut, path1, fileNameLength * sizeof(TCHAR), &nWrote, NULL) || nWrote != fileNameLength * sizeof(TCHAR)) {
		_ftprintf(stderr, _T("Error writing name of file %s %x\n"), path2, GetLastError());
		CloseHandle(hIn);
		CloseHandle(hOut);
		return;
	}

	DWORD nDigits = 1;
	DWORD tmp = fileSize.LowPart;

	while ((tmp /= 10) != 0)
	{
		nDigits++;
	}

	LPTCH fileSizeString = (LPTCH)malloc((nDigits + 2) * sizeof(TCHAR));
	if (_stprintf(fileSizeString, _T("\n%d\n"), fileSize.LowPart) != (nDigits + 2)) {
		_ftprintf(stderr, _T("Error building fileSizeString\n"));
		CloseHandle(hIn);
		CloseHandle(hOut);
		return;
	}

	// attach to path2 size of file
	if (!WriteFile(hOut, fileSizeString, ((nDigits + 2) * sizeof(TCHAR)), &nWrote, NULL) || nWrote != ((nDigits + 2) * sizeof(TCHAR))) {
		_ftprintf(stderr, _T("Error writing size file %s %x\n"), path2, GetLastError());
		CloseHandle(hIn);
		CloseHandle(hOut);
		return;
	}

	//copy src content to path2 file
	if (!WriteFile(hOut, srcFileCont, fileSize.LowPart, &nWrote, NULL) || nWrote != fileSize.LowPart) {
		_ftprintf(stderr, _T("Error writing src file content %s %x\n"), path2, GetLastError());
		CloseHandle(hIn);
		CloseHandle(hOut);
		return;
	}

	CloseHandle(hIn);
	CloseHandle(hOut);

	return;
}

DWORD getFileType(LPWIN32_FIND_DATA pointerFileData) {

	if ((pointerFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
	{
		if (lstrcmp(pointerFileData->cFileName, _T(".")) == 0 || lstrcmp(pointerFileData->cFileName, _T("..")) == 0)
		{
			return TYPE_DOT;
		}
		else
		{
			return TYPE_DIR;
		}
	}

	return TYPE_FILE;
}
