# Exercise 2 Lab 9

Three main function are used to implement the logic:

  1. `VOID copyTreeDirectory(LPTSTR path1, LPTSTR path2)`

  2. `VOID attachDataFields(LPTSTR path1, LPTSTR path2)`

  3. `DWORD getFileType(LPWIN32_FIND_DATA pointerFileData)`

The first function

  1. create the new isomorphic directory

  2. check the content of source directory using the third function listed above to discriminate the type of content

    - if it finds a file, attach on the top of it the name and the size of the file using the second function listed above

    - if it finds a directory, it call itself recursively

The second function listed above reads the content of the file, than writes into new file, in the new isomorphic directory the name and the size of the file and the content of the source file.
