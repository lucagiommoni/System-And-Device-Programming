# Exercise 01

## Optimize Exercise 01 of Laboratory 09.

The main thread instead of waiting for the termination of all ordering
threads **before** merging their results, it starts merging ordered
sequences as soon as possible, i.e., every time one ordering thread
has done its job.

In other word the main thread initially has an "empty" ordered
sequence, then:

1. It runs all ordering threads

2. As soon as one ordering thread terminates, it merges the ordered
   sequence coming from this thread with its own ordered sequence.
   (initially empty).
   It generates a unique *new* ordered sequence.

4. It proceeds until all ordering threads have terminated.
   At this point the generated sequence is the desired one.

5. It stores the final sequence into the output file.

## Suggestions

- Adapt the merge function to merge lists of values of increasing size
  starting from an empty list.

- Use function `WaitForMultipleObjects` such that it triggers a "merge"
  operation as soon as one ordering thread has finished.
  Remind that function `WaitForMultipleObject` can wait for an array of
  handles and unlock the caller each time one handle signals it.
  Nevertheless, notice that the array of handles has to be a neat and
  clean array with no spurious elements or already terminated threads.
