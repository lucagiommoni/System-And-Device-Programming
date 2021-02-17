# Exercise 01 (versions A, B, C and D)


A BINARY file defines the balance (i.e., the amount on money) in all
bank accounts held in a small bank branch (this kind of file is called
"ACCOUNT" file).

The file stores each bank account data on a single line, with
the following format:

- An identifier, i.e., an **integer** value varying from 1 to the number
  of rows in the file (e.g., 1, 2, etc.)

- The bank account number, i.e., a **long integer** (e.g., 164678)

- The surname account holder, i.e., a **string of maximum 30 characters**
  (e.g., Rossi)

- The name account holder, i.e., a **string of maximum 30 characters**
  (e.g., Mario)

- The bank account balance amount, i.e., an **integer** value (of a sum in
  euro).

The following is a correct example of such a file (in ASCII/text
format):

    1 100000 Romano Antonio 1250
    2 150000 Fabrizi Aldo 2245
    3 200000 Verdi Giacomo 11115
    4 250000 Rossi Luigi 13630

Another BINARY file specifies operations done on the bank accounts.
This file (called "OPERATION" file) has the same format of the
ACCOUNT file, i.e.:

    1 100000 Romano Antonio +50
    3 200000 Verdi Giacomo +115
    1 100000 Romano Antonio +250
    1 100000 Romano Antonio -55
    3 200000 Verdi Giacomo -1015

(in ASCII/text format) where each positive amount describe a deposit
in the bank account, and each negative number a withdrawal.

Write a C program in the MS Visual Studio environment satisfying the
following specifications:

- The program receives N parameters on the command line.
  The first parameter specify the name of an ACCOUNT file.
  All other parameters indicate the name of OPERATION files.

- The program has to open the ACCOUNT file, and then run N-1 threads
  (one for each OPERATION file).

- Each thread is in charge of reading and performing on the ACCOUNT
  file the set of operations specified on the related OPERATION file.
  The target of the program is to compute the final balance for all
  bank accounts in the ACCOUNT file.
  Obviously, threads have to be properly synchronized to avoid
  contemporary operations on the same bank account (i.e., OPERATION
  file 1 specifies a deposit on bank account 10, whereas OPERATION file
  3 specifies a withdrawal).

- When all threads have done their job, i.e., they have read their
  OPERATION files till the end, the resulting ACCOUNT file has to be
  printed-out on standard output by the main process, and all files
  have to be closed.

Write 4 versions of the program:

- **Version A:**

  Mutual exclusion is guaranteed adopting file locking
  applied on a line-by-line basis (i.e., each thread
  locks just the record it must modify).

- **Version B:**

  Use _critical sections_ to lock the entire ACCOUNT file

- **Version C:**

  Same as version B, but using mutexes

- **Version D:**

  Same as version B, but using semaphores
