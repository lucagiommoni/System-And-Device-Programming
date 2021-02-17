# Exercise 1 Lab 11

Specific struct is used to read/write data into binary file

```
typedef struct ACCDATA {
	INT index;
	DWORD bankAcc;
	TCHAR surname[SL];
	TCHAR name[SL];
	INT balance;
} ACCDATA;
```

Every time a thread read one record from OPERATION file, it scan all ACCOUNT
file to find the record having the same bank account of the record read.
To do that, file pointer is reset to 0 every scanning:

```
SetFilePointer(accFile, 0, 0, FILE_BEGIN);
```

Once the thread has read on record from OPERATION file, it tries to lock
ACCOUNT file. When it has finished to read/write to ACCOUNT file, it unlock the file.

The following code is used to get the file size to be used for locking file:

```
struct _stat buffer;
_tstat(argv[1], &buffer);
accountSize = buffer.st_size;
```

## Version A - LOCK/UNLOCK FILE

```
// Lock ACCOUNT file
if (!LockFileEx(accFile, LOCKFILE_EXCLUSIVE_LOCK, 0, accountSize, 0, &ov))
{
  _ftprintf(stderr, _T("%ld - Cannot lock ACCOUNT file - %ld\n"), GetCurrentThreadId(), GetLastError());
  CloseHandle(opFile);
  return 1;
}

...

// Unlock ACCOUNT file
if (!UnlockFileEx(accFile, 0, accountSize, 0, &ov))
{
  _ftprintf(stderr, _T("%ld - Cannot unlock ACCOUNT file - %ld\n"), GetCurrentThreadId(), GetLastError());
  CloseHandle(opFile);
  return 1;
}
```

## Version B - CRITICAL SECTION

```
...
CRITICAL_SECTION cs;
...

UINT _tmain(UINT argc, TCHAR* argv[]) {
  ...
  InitializeCriticalSection(&cs);
  ...
}

DWORD WINAPI threadRoutine(LPVOID filePath) {
  ...
  EnterCriticalSection(&cs);
  ...
  LeaveCriticalSection(&cs);
  ...
}
```

## Version C - MUTEX

```
...
HANDLE mutex;
...

UINT _tmain(UINT argc, TCHAR* argv[]) {
  ...

  mutex = CreateMutex(0, FALSE, 0);
  if (mutex == NULL || INVALID_HANDLE_VALUE)
  {
      printf("CreateMutex error: %d\n", GetLastError());
      return 1;
  }

  ...
}

DWORD WINAPI threadRoutine(LPVOID filePath) {
  ...

  // Get mutex
  if (WaitForSingleObject(mutex, INFINITE) != WAIT_OBJECT_0) {
    _ftprintf(stderr, _T("%ld - Failed waiting for mutex - %ld\n"), GetCurrentThreadId(), GetLastError());
    CloseHandle(accFile);
    CloseHandle(opFile);
    return 1;
  }

  ...

  // Release mutex
  if (!ReleaseMutex(mutex))
  {
    _ftprintf(stderr, _T("%ld - Failed releasing mutex - %ld\n"), GetCurrentThreadId(), GetLastError());
    CloseHandle(accFile);
    CloseHandle(opFile);
    return 1;
  }

  ...
}
```

## Version D - SEMAPHORE

```
...
HANDLE sem;
...

UINT _tmain(UINT argc, TCHAR* argv[]) {
  ...

  sem = CreateSemaphore(0, 1, 1, 0);
  if (sem == NULL || sem == INVALID_HANDLE_VALUE)
  {
      printf("CreateSemaphore error: %d\n", GetLastError());
      return 1;
  }

  ...
}

DWORD WINAPI threadRoutine(LPVOID filePath) {
  ...

  // Get semaphore
  if (WaitForSingleObject(sem, INFINITE) != WAIT_OBJECT_0) {
    _ftprintf(stderr, _T("%ld - Failed waiting for mutex - %ld\n"), GetCurrentThreadId(), GetLastError());
    CloseHandle(accFile);
    CloseHandle(opFile);
    return 1;
  }

  ...

  // Release semaphore
  ReleaseSemaphore(sem, 1, 0);

  ...
}
```
