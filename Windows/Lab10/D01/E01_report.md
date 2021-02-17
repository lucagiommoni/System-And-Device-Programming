# Exercise 01 Lab 10

The strategy is to modify, in each round, the arrays storing threads' ids and the threads' handles. New arrays are created to exclude the processed index (the arrays are therefore shortened).

For what concern the data produced by each thread, the array of the struct containing the ordered array of integer is not modified because is shared with the other running threads. But the field `done` of the struct associated with the terminated thread is updated with the value `TRUE` when it is processed by main thread. Variable `UINT shift` is used to address the correct struct for each terminated thread without the need to modify the shared array.

The key function for catching terminated threads is the following:

[WaitForMultipleObjects function](https://msdn.microsoft.com/en-us/library/windows/desktop/ms687025%28v=vs.85%29.aspx)
```
DWORD WINAPI WaitForMultipleObjects(
  DWORD  nCount,
  HANDLE *lpHandles,
  BOOL   bWaitAll,
  DWORD  dwMilliseconds
);

If bWaitAll is FALSE, the return value minus WAIT_OBJECT_0 indicates the
lpHandles array index of the object that satisfied the wait. If more than
one object became signaled during the call, this is the array index of the
signaled object with the smallest index value of all the signaled objects.
```

To check the correctness of the function, the following switch is used:

```
switch (WaitForMultipleObjects(i, arrayHandle, FALSE, INFINITE)) {			

  case WAIT_OBJECT_0:
    ...
    break;

  case WAIT_ABANDONED_0:
    ...
    break;

  case WAIT_TIMEOUT:
    ...
    break;

  case WAIT_FAILED:
    ...
    break;

  default:
    ...
    break;
}
```

To check the error:

```
_ftprintf(stderr, _T("WAIT_FAILED: %ld"), GetLastError());
```

Usually the return code 6 states _Invalid Handle_.


### Comments

Be carefull on `HANDLE *lpHandles` parameter of `WaitForMultipleObjects` function: it must contains only valid handle, otherwise the function return with error.
