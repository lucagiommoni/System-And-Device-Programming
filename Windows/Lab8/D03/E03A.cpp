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

	// id starts from 1 while file position starts from 0
	filePos.QuadPart = (id - 1) * sizeof(student);
	SetFilePointerEx(hIn, filePos, NULL, FILE_BEGIN);

	if ( ReadFile( hIn, &student, sizeof(student), &bytesRead, NULL) && bytesRead != sizeof(student) ) {
		_ftprintf(stderr, _T("Error in reading file\n"));
		return;
	}

	_tprintf(_T("%d %ld %s %s %d\n"), student.id, student.reg, student.name, student.surname, student.mark);

	return;
}

VOID writeUser(INT id, HANDLE hIn) {
	STUDENT student;
	LARGE_INTEGER filePos;
	DWORD bytesWritten;
	TCHAR command[INPUT_MAX_LEN];

	filePos.QuadPart = (id - 1) * sizeof(student);
	SetFilePointerEx(hIn, filePos, NULL, FILE_BEGIN);

	student.id = id;

	while (_fgetts(command, INPUT_MAX_LEN,stdin))
	{
		_tprintf(_T("Data: "));

		if (_stscanf(command, _T("%ld %s %s %d"), &student.reg, &student.name, &student.surname, &student.mark) == 4)
		{
			if (!WriteFile(hIn, &student, sizeof(student), &bytesWritten, NULL) || bytesWritten != sizeof(student))
			{
				_ftprintf(stderr, _T("Error on writing\n"));
				return;
			}
			else
			{
				return;
			}
		}
	}	
}
