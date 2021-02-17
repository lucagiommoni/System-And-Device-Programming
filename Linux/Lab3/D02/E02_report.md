# REPORT

To implement the Writers/Reades protocol with readers precedence:

1. A semaphore is used to implement mutual exclusion between the several readers and the same is done for writers

2. Another semaphore 'w' is used to implement mutual exclusion upon the resource that must be written/read

3. A variable 'nr' is used to implement reader's precedence

4. The main thread waits all child threads calling `pthread_join` function
