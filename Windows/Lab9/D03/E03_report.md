# Exercise 3 Lab 9B

All version use the following function with some variation in the implementation:

```
// Return the type of the object found inside the directory
// 1 = FILE
// 2 = DIRECTORY
// 3 = DOT (./ or ../)
DWORD FileType(LPWIN32_FIND_DATA);

DWORD WINAPI TraverseDirectory(LPVOID param);
```

In all versions the main thread waits all other threads via:

```
WaitForMultipleObjects(
  nThreads,
  tHandles,
  TRUE,
  INFINITE);
```

## Version A

This version use **CriticalSection** to write concurrently to the standard output:

```
do {
  FType = FileType(&FindData);
  if (FType == TYPE_DIR) {
    EnterCriticalSection(&cs);
    _tprintf(_T("TID: %u - DIR: %s\n"), GetCurrentThreadId(), FindData.cFileName);
    LeaveCriticalSection(&cs);
    TraverseDirectory(FindData.cFileName);
  }
} while (FindNextFile(SearchHandle, &FindData));
```

## Version B

In this version each thread writes logs into different files; for doing this, the following three function are used:

* `GetFileSizeEx`: gets filesize to set the pointer

* `SetFilePointerEx`: sets the file pointer to append logs at the end

* `WriteFile`: effectively writes log

At the end the main thread prints on the standard output the content of all log files:

```
for (DWORD i = 0; i < nThreads; i++) {
  memset(logFileName, 0, MAX_PATH);
  _sntprintf(logFileName, MAX_PATH - 1, _T("log_%u.txt"), tIds[i]);

  hLog = CreateFile(logFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (hLog == INVALID_HANDLE_VALUE) {
    _ftprintf(stderr, _T("Error opening file log_%u.txt\n"), GetCurrentThreadId());
    return 3;
  }

  do {
    memset(logFileName, 0, MAX_PATH);
    _ftprintf(stdout, _T("%s"), buffer);
  } while (ReadFile(hLog, buffer, MAX_PATH * sizeof(TCHAR), &nRead, NULL) && nRead > 0);

  CloseHandle(hLog);
}
```

## Version C

This version implement the **producer-consumer** pattern: there are one dedicated thread that print on the standard output the available log produced by each thread.

The following custom datatype is used to implement a circular buffer:
```
typedef struct OUTPUT {
	UINT head;         // the head of the buffer, index used by producers
	UINT tail;         // the tail of the buffer, index used by consumer
	UINT bufflen;      // the size of the buffer
	LPTSTR *buffer;    // the pointer to buffer
	HANDLE semEmpty;   // semaphore to signal that buffer can store resources
	HANDLE semFull;    // semaphore to signal that buffer has resources to be consumed
};
```

In this version, the main thread waits specifically for the thread dedicated to the log printing, via:

```
WaitForSingleObject(outpuThreadHandle, 3000);
```

A timeout is used because, if there are still resources into the buffer to be printed, this thread must finish the job before exiting, so it is not synchronous with other threads.

The function used by all other threads to print log is the following:
```
VOID printLog(LPTSTR log) {
	EnterCriticalSection(&cs); // the critical section protects shared variable 'output'
	WaitForSingleObject(output.semEmpty, INFINITE); // thread blocks if the buffer is full
	output.buffer[output.head] = log;
	output.head++;
	output.head %= output.bufflen; // modulo operation realize the round buffer
	ReleaseSemaphore(output.semFull, 1, NULL);
	LeaveCriticalSection(&cs);
}
```
