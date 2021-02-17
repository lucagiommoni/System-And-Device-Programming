#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

int res, child;

void childCounter() {
  child--;
  signal(SIGCHLD, childCounter);
  return;
}

int main(int argc, char const *argv[]) {

  if (argc < 3) {
    printf("USAGE: %s  <int c> <directory absolute path>\n", argv[0]);
    exit(1);
  }

  int c = atoi(argv[1]);

  if (c <= 0) {
    fprintf(stderr, "Error: Enter a positive non-zero integer!");
    exit(1);
  }

  struct stat sb;

  if (stat(argv[2], &sb) == -1) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(1);
  }

  // check if entered path corresponds to an existing directory
  if (!S_ISDIR(sb.st_mode)) {
    fprintf(stderr, "Error: No such file or directory!\n");
    exit(1);
  }

  signal(SIGCHLD, childCounter);

  int cmdLen = strlen(argv[2]) + 30;
  char cmd[cmdLen];
  memset(cmd, '\0', cmdLen); // init cmd
  sprintf(cmd, "ls -1 %s > list.txt", argv[2]); // create cmd
  system(cmd); // execute cmd

  child = 0;
  FILE *file = fopen("list.txt", "r");
  char tmp;
  int fnLen = 50;
  char *filename = (char*) malloc(fnLen);
  memset(filename, '\0', fnLen);

  while (!feof(file)) {

    tmp = fgetc(file); // read 1 character at a time

    if (tmp == '\r' || tmp == '\n') {

      if (child >= c) {
        printf("Num child (%d) >= c(%d) \n", child, c);
        wait(NULL);
      }

      switch (fork()) {
        case 0: // child
          if (chdir(argv[2]) < 0) { // change working directory
            perror("Error in changing working directory: ");
          }
          char *arg[] = { "sort", "-n", "-o", filename, filename, (char*) 0};
          if (execv("/usr/bin/sort", arg) < 0) { // execute sort
            perror("Error in sorting file: ");
          }
          exit(1);
        case -1:// error
          perror("Error: ");
          kill(0, SIGKILL);
          while(wait(NULL) != -1);
          exit(1);
        default: //father
          memset(filename, '\0', fnLen);
          child++;
      }
      continue;
    }

    if (strlen(filename) == (fnLen - 1)) {
      fnLen +=50;
      filename = realloc(filename, fnLen);
    }

    filename[strlen(filename)] = tmp;
  }

  fclose(file);
  free(filename);
  while(wait(NULL) != -1);

  if (chdir(argv[2]) < 0) {
    perror("Error in changing working directory: ");
  }

  if ((res = system("sort -n * > ../all_sorted.txt")) == -1) {
    fprintf(stderr, "Error %d in sorting all files", res);
  }

  return 0;
}
