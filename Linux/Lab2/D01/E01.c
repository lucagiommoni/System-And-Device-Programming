/**
  * EXERCISE 1 - WEEK 3
  * Name: Luca
  * Surname: Giommoni
  * Student ID: 225049
  * Year: 2018
  * Email: s225049@studenti.polito.it
  */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

int flag, end;

void fatherRoutine(int signo) {
  switch (signo) {
    case SIGUSR1:
      flag = !flag;
      break;
    case SIGCHLD:
    case SIGUSR2:
      end = 1;
  }
  return;
}

void childEndRoutine() {
  kill(getppid(), SIGUSR2);
  while(wait(NULL) != -1);
  exit(0);
}

void childRoutine() {
  switch (fork()) {
    case 0:
      sleep(60); // sleep 60 seconds
      kill(getppid(), SIGUSR2); // notify parents to finish
      exit(0);
    case -1:
      perror("Error while forking: ");
      exit(1);
  }
  signal(SIGUSR2, childEndRoutine);
  time_t t;
  srand((unsigned) time(&t)); // init random function
  while(1) {
    sleep((rand() % 10) + 1);
    kill(getppid(), SIGUSR1); // toggle parent's printing function
  }
  return;
}

int main(int argc, char const *argv[]) {

  if (argc != 2) {
    printf("USAGE: %s <filepath>\n", argv[0]);
    exit(1);
  }

  struct stat sb;

  if (stat(argv[1], &sb) == -1) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(1);
  }

  // check if entered path corresponds to an existing directory
  if (!S_ISREG(sb.st_mode)) {
    fprintf(stderr, "Error: Not a regular file!\n");
    exit(1);
  }

  signal(SIGCHLD, fatherRoutine);
  signal(SIGUSR1, fatherRoutine);
  signal(SIGUSR2, fatherRoutine);

  flag = 1;
  end = 0;

  switch (fork()) {
    case 0:
      childRoutine();
      exit(0);
    case -1:
      perror("Error while forking: ");
      exit(1);
  }

  int line = 0;
  char c[2] = {'\0', '\0'};
  int strLen = 50;
  char *str = (char*) malloc(strLen);
  memset(str, '\0', strLen);
  FILE *file = fopen(argv[1], "r");

  while (!end) {
    if (feof(file)) {
      rewind(file);
      line = 0;
    }

    c[0] = fgetc(file);

    if (c[0] == '\n' || c[0] == '\r') {
      if (flag) {
        fprintf(stdout, "%d - %s\n", line, str);
        fflush(stdout);
        sleep(1);
      }
      line++;
      memset(str, '\0', strLen);
      continue;
    }

    if (strlen(str) == (strLen - 1)) {
      strLen += 10;
      str = realloc(str, strLen);
      for (int i = (strLen-10); i <= strLen; i++) {
        str[i] = '\0';
      }
    }

    strcat(str, c);
  }

  fclose(file);
  free(str);
  while(wait(NULL) != -1);
  return 0;
}
