# Exercise 02

An ASCII file has lines with the following format:

- An identifier, i.e., an integer value varying from 1 to the number of rows in the file (e.g., 1, 2, etc.)

- A register number, i.e., a long integer of 6 digits (e.g., 164678)

- A surname, i.e., a string of maximum 30 characters (e.g., Rossi)

- A name, i.e., a string of maximum 30 characters (e.g., Mario)

- An examination mark, i.e., an integer value.

The following is a correct example of such a file:

```
1 100000 Romano Antonio 25
2 150000 Fabrizi Aldo 22
3 200000 Verdi Giacomo 15
4 250000 Rossi Luigi 30
```

Write a C program in the MS Visual Studio environment which is able to:

- Read such a file and create a new file with the exact same content
  but stored in *binary* format and with *fixed-length* records
  (integer + long integer + 30 characters + 30 characters + integer).

- To verify the previous step (i.e., to be sure the binary file has
  been stored in a proper way) re-read the binary file (the one
  just written) and write its content on standard output.

Input and output file names are passed on the command line.

### Suggestions

- Read the input file on a field by field basis, using the `_ftscanf`
  construct (see the `tchar.h` file)

- Store fields within a proper C data structure (`struct student` with
  fields for the id, register number, etc.)

- Store the structure onto the output file as a unique object
  using the `WriteFile` function.

- Read it back using `ReadFile`
