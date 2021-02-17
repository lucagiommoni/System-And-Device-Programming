# Exercise 3 Lab 8

The common part of the three version is an infinite loop that read user inputs and call the functions `printUser` and `writeUser`.

In those function there are some code differences based on the version.

## Version A

### Read the file using SetFilePointerEx

```
LARGE_INTEGER filePos;
...

filePos.QuadPart = (id - 1) * sizeof(student);

SetFilePointerEx(
  hIn,
  filePos,
  NULL,
  FILE_BEGIN
);
```

## Version B

### Read the file using an overlapped data structure

```
LARGE_INTEGER filePos;
OVERLAPPED ov = {0, 0, 0, NULL};
filePos.QuadPart = (id - 1) * sizeof(student);
ov.Offset = filePos.LowPart;
ov.OffsetHigh = filePos.HighPart;
...

ReadFile(
  hIn,
  &student,
  sizeof(student),
  &bytesRead,
  &ov
)
```

## Version C

### Locking / Unlocking each record

```
LARGE_INTEGER filePos;
OVERLAPPED ov = {0, 0, 0, NULL};
LARGE_INTEGER size;
...

filePos.QuadPart = (id - 1) * sizeof(student);
ov.Offset = filePos.LowPart;
ov.OffsetHigh = filePos.HighPart;
size.QuadPart = sizeof(student);
...

LockFileEx(
  hIn,
  0,
  0,
  size.LowPart,
  size.HighPart,
  &ov
)
...

ReadFile(
  hIn,
  &student,
  sizeof(student),
  &bytesRead,
  &ov
)
```
