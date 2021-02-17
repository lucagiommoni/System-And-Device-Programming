# REPORT

To implement producer/consumer protocol:

1. A new type has been defined: a structure that implements

  - Two semaphores to regulate full/empty situation of queue

  - Two indexes, in/out, to pull/push items from/to tail/head of queue

  - A pointer of type long long, suitable to store long long variable related to milliseconds value

  - A variable, size, that states the size of the queue

2. The function 'buffer_init', used to initialize the queue; the function returns a pointer to new dynamically memory allocated queue, setting size of queue, setting indexes to zero, allocating semaphores and the buffer

3. The function 'put', used to insert new items into queue: the function uses existing semaphores configured inside Buffer structure, 'empty' to wait if queue is full, 'full' to notify consumer that queue is not empty, and increment 'in' index in circular way.

4. The 'get' function returns one item from the queue: it tries to wait for urgent queue and if it is empty, it waits for normal queue; it then get the item and notify producer that queue is not full using 'empty' semaphore.

5. The producer routine decides which queue to use to store item via random function and user entered probability value
