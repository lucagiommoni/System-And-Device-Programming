/**
  * NOTE:
  * Compilazione:
  * $ gcc -Wall -g -lpthread -lm -o esercizio02 esercizio02.c
  */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

struct LinkedList {
  int len;
  pthread_t tid;
  struct LinkedList *ll;
};
sem_t *sem;
int counter;

void printTree(struct LinkedList ll, char *nodes) {
  int tmpLen = 11 + (nodes == NULL ? 0 : strlen(nodes));
  char *tmp = (char*) malloc(tmpLen);
  memset(tmp, '\0', tmpLen);

  sprintf(tmp, " %u", (unsigned int) ll.tid);

  if (nodes != NULL) {
    strcat(tmp, nodes);
  }

  if (ll.len == 0) {
    printf("Thread tree:%s\n", tmp);
  } else {
    printTree(*ll.ll, tmp);
  }
  return;
}

void *func1(void *ll) {
  struct LinkedList *newlist = (struct LinkedList*) malloc(sizeof(struct LinkedList));
  newlist->len = ll == NULL ? 0 : ((struct LinkedList*) ll)->len + 1;
  newlist->tid = pthread_self();
  newlist->ll = (struct LinkedList*) ll;

  sem_wait(sem);
  counter--;
  sem_post(sem);

  if (counter <= 0) {
    printTree(*newlist, NULL);
    pthread_exit(NULL);
  }

  pthread_t pt;
  if (pthread_create(&pt, NULL, func1, newlist) != 0 ||
      pthread_create(&pt, NULL, func1, newlist) != 0) {
    perror("Error: ");
  }

  pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {

  if (argc != 2 || atoi(argv[1]) <= 0) {
    printf("USAGE: %s <number>\n", argv[0]);
    exit(1);
  }

  counter = pow(2,atoi(argv[1]));
  printf("Leaves: %d\n", counter);

  sem = (sem_t *) malloc(sizeof(sem_t));

  if (sem_init(sem, 0, 1) != 0) {
    perror("Error: ");
    exit(1);
  }

  func1(NULL);

  return 0;
}
