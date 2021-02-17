# Structure of the programs:

1. Check of arguments passed by user

  - the argument must be 1

  - the argument must be a positive non-zero number

2. Creation of a semaphore (mutex) useful to manage shared variable (the one that counts the number of thread created)

3. Start of recursion that creates threads

4. The print of the thread tree.

### NOTE:
To keep track of every thread a linkedlist is used. Every node of the list contains the actual length of the list, the thread ID of the thread that creates the node, a reference to preceding node.
When last threads (leaves) have been created, these call the recursive function printTree that loop along the linkedlist and print every thread ID.
