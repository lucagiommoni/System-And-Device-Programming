# Exercise 02

### Copy a directory tree and modify each source file while copying it.

A C program is run with two parameters `name1 name2`
where `name1` and `name2` are C strings which indicates relative
or absolute paths of two directory trees.

The program must copy the directory `name1` into an isomorphic directory `name2`.

For each file copied from the first to the second directory tree,
the program has to add two data fields on top of the destination file
specifying:

- the first one, the name of the file (C string)

- the second one, the size of the source (original) file
  (a 32-bit or 64-bit integer value at choice).
