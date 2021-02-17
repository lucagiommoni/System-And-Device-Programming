# REPORT

To implement dot product with threads, reserving last operation to the last thread, has been used a shared variable that states the number of thread not yet ended. A mutex semaphore has been created to manage the shared counter. In the _pthread_ routine there is a check, performed on the shared variable, to enable last operation.

The vectors are implemented as pointers and matrix, instead, as pointer of pointers. In this way it was possible to use global variables, avoiding the use arguments for _pthread_ routines.
