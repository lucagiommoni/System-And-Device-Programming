#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>

int main(int argc, char const *argv[]) {
  if (argc != 3 || atoi(argv[1]) <= 0 || atoi(argv[2]) <= 0) {
    fprintf (stderr, "USAGE: %s <n> <k>\n", argv[0]);
    exit(1);
  }

  int n = atoi(argv[1]);
  int k = atoi(argv[2]);

  // Remove directory if already exists
  system("rm -rf data");

  // Create directory
  system("mkdir data");

  char fnum[3];
  memset(fnum, '\0', 3); // init fnum

  // Contains command
  // size of 50 should be enough (echo 1234567890 >> data/f00)
  char cmd[50];
  memset(cmd, '\0', 50);

  time_t t;
  srand((unsigned)time(&t)); // init random func

  int counter; // how many integer into file

  // Create n files inside 'data'
  for (int i = 0; i < n; i++) {

    fnum[0] = '\0'; // init string

    // name format = fXX
    if (i < 10) {
      sprintf(fnum, "0%d", i);
    } else {
      sprintf(fnum, "%d", i);
    }

    counter = (rand() % k) + 1;

    for (int j = 0; j < counter; j++) {
      sprintf(cmd, "echo %d >> data/f%s", rand(), fnum);
      system(cmd); // append integer into file
      memset(cmd, '\0', 50);
    }

  }

  return 0;
}
