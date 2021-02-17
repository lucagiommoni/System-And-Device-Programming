# Exercise 2

Write a concurrent C program using only conditions and mutexes, that implement the **Readers & Writers** protocol, with precedence to the Readers.

In particular, the main threads creates N readers and N writers (N given as an argument of the command line) and waits for their termination.

Each reader/writer

- sleeps for a random time of millisecond (0-500)

- `printf(“Thread %d trying to read/write at time %d\n”, …)`

- when it is able to read, it `printf(“Thread %d reading/writing at time %d1n”, … )`

- simulates the reading/writing operation sleeping 500 milliseconds

- terminates
