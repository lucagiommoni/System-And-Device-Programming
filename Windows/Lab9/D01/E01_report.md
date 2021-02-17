# Exercise 1 Lab 9

The ordering function used is the bubble sort one:

```
VOID bubblesort(LPUINT array, UINT size) {
	UINT tmp;
	for (UINT i = 0; i < size-2; i++) {
		if (array[i]>array[i+1]) {
			tmp = array[i + 1];
			array[i + 1] = array[i];
			array[i] = tmp;
		}
	}
	return;
}
```

Each thread receive as parameter the following:

```
struct FileInfoStruct {
	LPTCH fileName;        // contains the name of file
	UINT size;             // thread will store here the number of integer (the first number of the line)
	LPUINT outputArray;    // thread will store here the ordered array of the integers stored into the input file
};
```

The main thread use a for loop to generate threads using the function:

```
CreateThread(
  0,                                // security attributes
  0,                                // thread stack (0 = 1MB = default)
  threadSortFile,                   // function that thread will execute
  (LPVOID)&lpFileInfoStruct[i],     // function parameter
  0,                                // thread ready to run
  NULL                              // thread ID
);
```

and waits for all of them using the following:

```
WaitForMultipleObjects(
  numberThreads,  // number of threads to wait
  arrayHandle,    // the array of threads handles
  TRUE,           // wait for all
  INFINITE        // no timeout
);
```
