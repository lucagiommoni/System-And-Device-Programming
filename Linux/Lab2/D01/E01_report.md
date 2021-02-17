# Structure of the programs:

1. Check of arguments passed by user

  - the argument must be 1

  - the argument must be a valid path that corresponds to a regular file

2. Setting signal handlers: if parent process receives

  - SIGUSR1: toggles printing function

  - SIGUSR2|SIGCHLD: exits

3. Main process fork a child and starts reading file and printing each line

4. Child process forks another process that will sleep 60 seconds and then sends to parent a SIGUSR2 signal

5. The first child loop forever and sleep randomly from 1 to 10 seconds and sends parent a SIGUSR1 signal. When it receives from his child a SIGUSR2 signal, it forwards the signal to its parent and terminates.

6. Main process loop continuously until it receives SIGUSR2 or SIGCHLD signal.


### NOTE:
One signal handler only is used by main process because it can discriminate signals through switch and do simple actions, like to toggle flags;
For the first child a dedicated function is used just to keep separate father and son actions.
