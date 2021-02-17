# Exercise 02 Lab 13

To compute partial result, the following structure is used:

```
typedef struct PARTIAL {
  // Array that stores partial product
  DWORD *part;

  // Array of flag that states which
  // partial product is available
  BOOL *ready;

  // To synchronize access to ready flags
  CRITICAL_SECTION cs;

  DWORD counter;

  // Semaphore that signal if some partial product
  // is available
  HANDLE isReady;
} PARTIAL;
```
