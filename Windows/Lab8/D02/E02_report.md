# Exercise 2 Lab 8

To store information about student, the following struct is used:

```
typedef struct STUDENT {
  INT id;
  DWORD reg;
  TCHAR name[MAX_CHARS + 1];		// +1 is for null terminator
  TCHAR surname[MAX_CHARS + 1];	// +1 is for null terminator
  INT mark;
};
```

To read formatted line from file the following functions are used:

```
// open file in reading mode
_tfopen(argv[1], _T("rt"))


// read each formatted line and store fields into struct variable
_ftscanf(
  fileIn,
  _T("%d %ld %s %s %d"),
  &student.id, &student.reg,
  &student.name,
  &student.surname,
  &student.mark
)
```

The structs relative to students are saved into new file using the following function:

```
WriteFile(
  hOut,
  &student,
  sizeof(student),
  &nOut,
  NULL
)
```

To verify the correctness of new written file, it is read using:

```
ReadFile(
  hIn,
  &student,
  sizeof(student),
  &nIn,
  NULL
)
```

and printing the results on the standard output via:

```
_tprintf(
  _T("%d %ld %s %s %d\n"),
  student.id, student.reg,
  student.name,
  student.surname,
  student.mark
);
```
