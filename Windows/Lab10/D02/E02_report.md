# Exercise 02 Lab 10

Strategy:

- one thread for each user-entered path is created (_reader_); the thread call a function that will analyze the content of the directory;

- when an item is found, its name is stored into a shared variable that will be read by comparator thread;

- if the found item is a directory, after the comparison with the other items found by other threads, it is traversed through a recursive call of the same method;

- if a _reader_ finishes in traversing directory, it set a flag to state the end of routine and signal the `readyList` event one last time

- comparator thread checks if all _reader_ threads end up and, in case some end before others, it states the inequality of directories through `isEqual` flag.

Events are used to synchronize reading threads:

```
compareOk = CreateEvent(0, TRUE, FALSE, 0);

if (compareOk == NULL || compareOk == INVALID_HANDLE_VALUE)
{
  _ftprintf(stderr, _T("Failed creating compareOk event\n"));
  exit(1);
}

listReady = (LPHANDLE)malloc(nThreads * sizeof(HANDLE));

if (listReady == NULL)
{
  _ftprintf(stderr, _T("Failed listReady malloc\n"));
  exit(1);
}

for (UINT i = 0; i < nThreads; i++)
{
  listReady[i] = CreateEvent(0, TRUE, FALSE, 0);
  if (listReady[i] == NULL || listReady[i] == INVALID_HANDLE_VALUE)
  {
    _ftprintf(stderr, _T("Failed creating listReady[%u] event\n"), i);
    exit(1);
  }
}
```

`compareOk` is used by comparator thread to signal every reader threads that comparison finished.
In this way reader threads can keep on retrieving directories' items.

`listReady` are an event array, one for each reader thread. It is used to signal the comparator that data are available for comparation. The comparator waits for all listReady event.

```
if (WaitForMultipleObjects(nThreads, listReady, TRUE, INFINITE) != WAIT_OBJECT_0)
{
	_ftprintf(stderr, _T("Failed waiting listReady: %ld\n"), GetLastError());
	exit(1);
}
```
