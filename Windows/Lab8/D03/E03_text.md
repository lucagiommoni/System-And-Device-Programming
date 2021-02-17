# Exercise 03 (versions A, B, and C)

A data base is given on a single *binary* file with
*fixed-length* records.

The format of the file is the one *generated in Exercise 02*.

The program has to implement a user menu with the following options:

- R n: Where R is the character "R", and n is an integer value.
      Read from file all data fields for the student number n
      and print them on standard output.

- W n: Where W is the character "W", and n is an integer value.
      Read from standard input all data fields for the student
      number n and write them on the file in position n.

- E: End the program.

The input file name is given on the command line.

The following is an example of execution (using the file presented in
Exercise 02):

```
user choice: R 3     // The user wants to read info for student 3
3 200000 Verdi Giacomo 15
user choice: R 1     // The user wants to read info for student 1
1 100000 Romano Antonio 25
user choice: W 1     // The user wants to over-write info for student 1
                     // with info read from standard input
Data: 1 100000 Romano Antonio 27y
                     // Read input data and store them for student 1
user choice: W 5     // The user wants to add data for student 5
                     // with info read from standard input
data: 5 157143 White House 30
                     // Read input data and store them for student 1
user choice: E       // The user wants to end the program
stop program
```

Noticed that

1. The input file is supposed to be the one generated in
   Exercise 02

2. `//` specifies explanatory comments not program I/O lines

3. `R 3, R 1, W 1, etc.` are the user inputs, all other characters
   belong to the program output.

Write **3** versions of the program:

- Version A
      Read the file using SetFilePointerEx

- Version B
      Read the file using an overlapped data structure

- Version C
      Lock each record before reading (or writing) it, and release the
      same record as soon as the operation has been performed.
      (Notice that locking for now is useless, but it will be useful with
      multi-threaded applications).
