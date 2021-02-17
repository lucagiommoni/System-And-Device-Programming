# Structure of the programs:

1. Check of arguments passed by user
  - the arguments must be 2
  - the first argument must be a positive non-zero integer
  - the second argument must be a valid path that corresponds to an existing directory

2. Set a signal handler to manage number of concurrent process:
  - when main process makes a fork, it increments the counter of the number of children
  - when the counter of the number of children reaches the number entered by user, main process stops, waiting for some child to finish
  - when child terminates, it sends to father SIGCHLD
  - main process manages SIGCHLD and decrements the children number's counter

3. 'cmd' variable is used to make the command useful to save the list of files contained into directory passed as parameter.

4. Every time a line, corresponding to a file name, is read by main process, a child process is created to sort the numbers contained into that file. To do so, child process change its working directory first, then it executes the 'sort' command by means of 'execlp' function.

5. If some errors occur during the fork operation, the main process try to kill all child processes, then it waits for them and terminates

6. Main process waits for all child processes to finish, then creates a new file 'all_sorted.txt' where to put the result of sort operation upon all numbers coming from files contained into the directory entered as parameter.

### NOTE:
Since 'execlp' function replace the image of calling process with the called process one, it is not necessary to close file handler and to free dynamically allocated memory. Furthermore, once 'execlp' is executed, the control pass to the called process and the only possible way for father and son to communicate is during termination of child because this sends to father SIGCHLD signal. Managing this situation in useful for processes 'birth control'.
