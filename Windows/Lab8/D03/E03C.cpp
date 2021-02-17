#include "stdafx.h"

#define MAX_CHARS 30
#define INPUT_MAX_LEN 255

typedef struct STUDENT {
	INT id;
	DWORD reg;
	TCHAR name[MAX_CHARS + 1];		// +1 is for null terminator
	TCHAR surname[MAX_CHARS + 1];	// +1 is for null terminator
	INT mark;
};

VOID printUser(INT id, HANDLE hIn);
VOID writeUser(INT id, HANDLE hIn);

int _tmain( INT argc, LPTSTR argv[] ) {

	if ( argc != 2 ) {
		_ftprintf(stderr, _T( "USAGE: %s dbFile" ), argv[0]);
		return 1;
	}

	HANDLE hIn;
	BOOL end;
	TCHAR userInput[INPUT_MAX_LEN];
	TCHAR operation;
	INT id;

	// Open dbFile
	hIn = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if ( hIn == INVALID_HANDLE_VALUE ) {
		_ftprintf( stderr, _T( "Cannot open %s file\n" ), argv[1] );
		return 2;
	}

	end = FALSE;

	while ( !end ) {
		_tprintf( _T( "user choice: " ) );
		_fgetts( userInput, INPUT_MAX_LEN, stdin );

		if (_stscanf(userInput, _T("%c %d"), &operation, &id) == 2) {
			switch (operation) {
			case 'R' :
				printUser( id, hIn );
				break;
			case 'W' :
				writeUser( id, hIn );
				break;
			case 'E' :
				end = TRUE;
				break;
			default:
				_tprintf( _T( "Invalid operation!\n" ) );
				break;
			}
		}
		else
		{
			_tprintf(_T("Invalid operation!\n"));
		}
	}

	CloseHandle( hIn );

	return 0;
}

VOID printUser(INT id, HANDLE hIn) {
	STUDENT student;
	LARGE_INTEGER filePos;
	DWORD bytesRead;
	OVERLAPPED ov = { 0,0,0,NULL };
	LARGE_INTEGER size;

	// id starts from 1 while file position starts from 0
	filePos.QuadPart = (id - 1) * sizeof(student);

	ov.Offset = filePos.LowPart;
	ov.OffsetHigh = filePos.HighPart;

	size.QuadPart = sizeof(student);

	// locking before reading
	if (!LockFileEx(hIn,0,0,size.LowPart,size.HighPart,&ov))
	{
		_ftprintf(stderr,_T("Error locking file: %x\n"), GetLastError());
		return;
	}

	if ( ReadFile( hIn, &student, sizeof(student), &bytesRead, &ov) && bytesRead != sizeof(student) ) {
		_ftprintf(stderr, _T("Error in reading file\n"));
	}
	else
	{
		_tprintf(_T("%d %ld %s %s %d\n"), student.id, student.reg, student.name, student.surname, student.mark);
	}

	//unlock file
	if (!UnlockFileEx(hIn,0, size.LowPart, size.HighPart,&ov))
	{
		_ftprintf(stderr, _T("Error unlocking file: %x\n"), GetLastError());
		exit(3);
	}
}

VOID writeUser(INT id, HANDLE hIn) {
	STUDENT student;
	LARGE_INTEGER filePos;
	DWORD bytesWritten;
	TCHAR command[INPUT_MAX_LEN];
	OVERLAPPED ov = { 0,0,0,NULL };
	LARGE_INTEGER size;

	filePos.QuadPart = (id - 1) * sizeof(student);
	ov.Offset = filePos.LowPart;
	ov.OffsetHigh = filePos.HighPart;
	size.QuadPart = sizeof(student);

	student.id = id;

	while (_fgetts(command, INPUT_MAX_LEN,stdin))
	{
		_tprintf(_T("Data: "));

		if (_stscanf(command, _T("%ld %s %s %d"), &student.reg, &student.name, &student.surname, &student.mark) == 4)
		{
			//lock file
			if (!LockFileEx(hIn, 0, 0, size.LowPart,size.HighPart,&ov))
			{
				_ftprintf(stderr, _T("Error locking file: %x\n"), GetLastError());
				return;
			}
			if (!WriteFile(hIn, &student, sizeof(student), &bytesWritten, &ov) || bytesWritten != sizeof(student))
			{
				_ftprintf(stderr, _T("Error on writing\n"));
				if (!UnlockFileEx(hIn,0,size.LowPart,size.HighPart,&ov))
				{
					_ftprintf(stderr, _T("Error unlocking file: %x\n"), GetLastError());
					exit(3);
				}
			}
			else
			{
				if (!UnlockFileEx(hIn, 0, size.LowPart, size.HighPart, &ov))
				{
					_ftprintf(stderr, _T("Error unlocking file: %x\n"), GetLastError());
					exit(3);
				}
				return;
			}
		}
	}
}
