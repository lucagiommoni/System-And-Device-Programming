# REPORT

To implement a threaded quicksort, first of all I define a new type that include two integer: these will be used to store the two index (left and right) used by quicksort algorithm.

The phread routine is very simple because it deals only with the call to quicksort function.

The quicksort algorithm's code has been modified in order to create pthread when the difference between right and left indexes is greater than or equal to a reference value, passed as parameter by user.

For what concern file mapping, first of all the correctness of file is checked by the program: if file is regular, it is opened in read only mode. Then it is mapped into memory, casting it as integer pointer; the mapping is done with "PROT_READ | PROT_WRITE" and "MAP_PRIVATE" parameters in order to enable shared writing for concurrent threads.
After file mapping, the file is closed and all the operations are done using data mapped into memory.
