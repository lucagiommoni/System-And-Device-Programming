Exercise 03
------------

A one-way bridge is approached by cars in both directions, but being
one-way it must follow three rules:

- Only cars from one direction can traverse the bridge in each moment,
  either the ones moving from left-to-right or the one running from
  right-to-left.

- There can be multiple cars going in the same direction.

- If cars from the opposite direction arrive while there are cars on
  the bridge, those cars in the opposite direction must wait till the
  bridge is cleared.

Write a Windows program to synchronize all cars traversing the bridge.
More specifically, the program must run two threads.
The first thread is in charge of all cars moving from left-to-right.
The second one is in charge of all cars moving from right-to-left.

Notice that:

- All cars need timeL2R (integer value) seconds to traverse the bridge
  from left-to-right

- All cars need timeR2L (integer value) seconds to traverse it from
  right-to-left.

- Car in each direction arrive at random time interval, varying from 0
  to timeARRIVAL (integer value) seconds.

- The program has to terminate only once exactly

  - nL2R (integer value) cars have traversed the bridge from
    left-to-right
    
  - nR2L (integer value) cars have traversed the bridge from
    right-to-left.

The program must accepts 5 parameters on the command line, namely 5
integers: timeL2R, timeR2L, timeARRIVAL, nL2R, nR2L.

Suggestion
-----------

Referring to the classical Reader and Writer logical scheme modify it
to manipulate two sets of Readers.
