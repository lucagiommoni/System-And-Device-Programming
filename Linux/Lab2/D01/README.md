# Exercise 1

Write a C program that generate a child. The parent opens a file given as argument in the command
line, then loops forever, 1) reading each line, 2) printing the line number and the line content 3)
rewinding the file. The child process sends a SIGUSR1 signal to the parent at random intervals
between 1 and 10 seconds. The first received signal makes the parent skip the print statement. It will restart printing after receiving the next SIGUSR1 signal. This behavior is repeated for all the received SIGUSR1 signals. After 60 seconds, the child must send a SIGUSR2 signal to the parent, and then terminate. Receiving this signal, also the parent will terminate.

#### Hint:
the child forks a process that sleeps 60 seconds then send to it a SIGUSR2, which the child catches and “forward” to the parent.
