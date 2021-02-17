# Exercise 03 (versions A, B and C)

Visit N directory trees concurrently with N threads.

A C program receives N parameters on the command line.
Each parameter indicates a relative or an absolute path to
a file system directory tree.

The program has to run N threads (one for each argument).

Each thread recursively visits one of the directories, and,
for each directory entry, it prints-out its thread identifier
and the directory entry name.

The main thread awaits for the termination of all threads.
When all threads have visited their directory also the program
ends.

## Version A

As all output lines are generated independently by each single
thread, printing messages from different threads are interleaved
on standard output.

## Version B

To avoid the problem of Version A, Version B uses files.
In other words, each thread stores its output on a file whose name
is generated (to avoid conflicts) based on the thread identifier.
When all working threads end, the main thread copies all files on
standard output with the desired strategy (and no interruption).

## Version C - Optional / Elective

(need synchronization among threads)
This version generalizes version B by using a centralized thread
(the "output thread") dedicated to generate the entire application
output.

All threads resort to the centralized and dedicated output thread to
perform all operations on standard output.

The output thread is executed within a critical section, and it just
awaits for more output to come in, and then it takes care of it with
the strategy at choice (i.e.,, it may store messages on files, like
Version B, it may use a dedicated data structure to store messages, it
may decide which verbosity level to use, it may compute output
statistics, etc.).
