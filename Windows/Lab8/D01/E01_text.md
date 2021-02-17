# Exercise 01


The following program copies a file, whose name is received as
first parameter on the command line, into another file, whose
name is passed as the second parameter on the command line.

```
#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#define BUF_SIZE 100

INT _tmain (INT argc, LPTSTR argv [])
{
  HANDLE Hon, hOut;
  DWORD nIn, nOut;
  CHAR buffer [BUF_SIZE];

  if (argc != 3) {
    fprintf (stderr, "Usage: cp file1 file2\n");
    return 1;
  }

  hIn = CreateFile (argv[1], GENERIC_READ, FILE_SHARE_READ, NULL,
    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hIn == INVALID_HANDLE_VALUE) {
    fprintf ("Cannot open input file. Error: %x\n", GetLastError ());
    return 2;
  }

  hOut = CreateFile (argv[2], GENERIC_WRITE, 0, NULL,
    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hOut == INVALID_HANDLE_VALUE) {
    fprintf (stderr, "Cannot open output file. Error: %x\n", GetLastError ());
    CloseHandle(hIn);
    return 3;
  }

  while (ReadFile (hIn, buffer, BUF_SIZE, &nIn, NULL) && nIn > 0) {
    WriteFile (hOut, buffer, BUF_SIZE, &nOut, NULL);
    if (nIn != nOut) {
      fprintf (stderr, "Fatal write error: %x\n", GetLastError ());
      CloseHandle(hIn); CloseHandle(hOut);
      return 4;
    }
  }

  CloseHandle (hin);
  CloseHandle (hOut);
  return 0;
}

```


### Perform the following steps:

- Cut-and-paste the program into a .cpp file

- Create a new project and insert the program within the MS Visual Studio environment.

- Set the command line parameters.

- Build (compile) the project.

- As it contains (3?) syntax errors correct them.

- Once obtained the executable code, run it.

- As it contains a bug, debug it using the basic features of the Visual Studio debugger.

  Learn how to:

  - set a break-point in the code

  - proceed step-by-step

  - visualize variable values

  - see thread configuration

  - etc.
