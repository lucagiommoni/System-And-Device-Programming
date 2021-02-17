# Exercise 01


A file stores integer values in **binary format** on 32 bits.

The first integer stored in the file indicates the number of values
following the first one.

For example, the following three lines specify (obviously in ASCII
format) the content of three possible files (that have to be stored in
binary format):

```
File 1: 5 23 45 67 9 23
File 2: 12 90 65 34 24 12 0 89 29 54 12 78 3
File 3: 3 9 5 1
```

Write a WIN 32 application which:

- Receives a variable number of strings, let us say `n` strings, on the
  command line.
  The first `(n-1)` strings are input file names.
  The last string is an output file name.
  Each input file has the format previously described.

- Runs one thread for each input file passing to each of them one of
  the input file names.
  We will refer to these `(n-1)` working threads as "ordering" threads.

- After running all ordering threads, the main application awaits for
  the termination of all of them.

- When the main threads awaits, each ordering thread:

  - opens "its own" input file

  - reads the first integer value

  - allocates a dynamic array of integers to store all other integer
    numbers stored in the file

  - read those numbers into the array

  - orders the array (in ascending order) using whatever ordering
    algorithm it is deemed appropriate

  - ends (returning the control to the main application thread).

- The main application thread, once collected the termination of all
  ordering threads, merges all ordered array into a unique array,
  i.e., a unique sequence of integers.

- It stores the resulting array into the output file, using the
  same format of the input files.

For the previous files the output file should be:

    20 0 1 3 5 9 9 12 15 23 23 24 29 34 45 54 65 67 78 89 90
