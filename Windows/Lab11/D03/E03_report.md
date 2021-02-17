# Exercise 03 Lab 11

Implementation of 2 readers pattern.

4 thread routines:

```
DWORD WINAPI l2rRoutine(LPVOID)
DWORD WINAPI r2lRoutine(LPVOID)
```
The above routines manage cars that travel from left to right and from right to left;
for `nL2R` (the first) and `nR2L` (the second) times, every `timeARRIVAL` seconds, they create a new thread that represent a car.

```
DWORD WINAPI traverseBridgeL2R(LPVOID)
DWORD WINAPI traverseBridgeR2L(LPVOID)
```

The above routines manage threads that represent cars and the readers logic.

The following is an extract of the routine:

```
DWORD WINAPI traverseBridgeL2R() {
  ...

  EnterCriticalSection(&cs_l2r);

  actualL2R++;

  if (actualL2R == 1) {
  	WaitForSingleObject(busy, INFINITE);
    ...
  }

  ...

  LeaveCriticalSection(&cs_l2r);

  Sleep(timeL2R);

  EnterCriticalSection(&cs_l2r);

  ...

  actualL2R--;

  if (actualL2R == 0) {
     ReleaseSemaphore(busy, 1, 0);
     ...
  }

  LeaveCriticalSection(&cs_l2r);

  return 0;
}
```

As usual, critical section is used to synchronize console output.

NOTE
---

Semaphore is used instead of Mutex to address ownership issue. When Mutex is
used, indeed, the thread that get ownership of mutex must release it; with
semaphore, instead, any thread can manipulate the value.
