# Structure of the programs:

1. Creation of 2 mutex semaphore

2. Creation of 2 thread (client - server)

3. The main thread waits the completion of client thread, then stop the server thread and print the total number of requests served.

4. The client thread calls 2 times (because 2 files) the function that reads the integer from file. When a value is read, it notify the server that the shared variable contains a value to be processed and wait for server. When no more value is read from files, the client thread terminates. If some errors occur, the thread terminates with a value different from 0.

5. The server routine is very simple: a forever loop that manipulates shared variable when notified by client thread. It is stopped by main thread when client terminates.

### NOTE:
2 semaphores is used to avoid deadlock and race condition: when a value is ready to be processes by server, client wait on one semaphore, releasing the other for server.
The entire server lifecycle is managed by main thread to keep simple the server routine.
