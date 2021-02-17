# Exercise 02

Realize in the Windows environment a producer/consumer application
with the following characteristics:

1. There are P producers and C consumers

2. Producers and consumers communicate using a circular buffer.
   The circular buffer is implemented as a queue on a dynamic array of
   size N.

3. Each producer inserts in the queue integer values (randomly
   generated) at random time intervals (randomly generated and
   varying from 0 to T seconds).

4. Each consumer extracts from the queue an integer value at random
   time intervals (randomly generated and varying from 0 to T seconds)
   and prints it out on standard output with its personal thread
   identifier.

Note that:

- The integer values P, C, N, and T are passed to the program on the
  command line

- The circular queue has to be properly protected, to avoid:

  - insertions in a full queue

  - extractions from an empty queue

  - insertions by two producers at the same time

  - extractions by two consumers at the same time

    See previous lessons or the following scheme for more details.

- Find a reasonable way to terminate all producers and all consumers.

Suggestions
-----------

- Refer to the Producer/Consumer logical scheme with more than
  one producer and more than one consumer working concurrently.

- To stop the application in a reasonable way use the following
  strategy:

  - Each producer terminates after a predefined number of products
    has been produced

  - The main thread awaits for all producers to end, and when all
    producers have terminated it terminates all consumers
    
  - To terminate all consumers (after they have consumed all produced
    elements *not* before !) the main thread may insert in the queue
    "termination" (sentinel) values.
