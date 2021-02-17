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

sem_t *cl, *sr;
int counter, g;
void *retval;
pthread_t client, server;

void readInt(const char* filename) {
  FILE *file;
  if ((file = fopen(filename,"rb")) == NULL) {
    perror("Error: ");
    pthread_exit((void*) 1);
  }

  clearerr(file);

  while (fread(&g, sizeof(int), 1, file) == 1) {
    sem_post(cl); // unlock server
    sem_wait(sr); // wait for server
    printf("g = %d\n", g);
    counter++;
  }

  if (ferror(file) != 0) {
    fclose(file);
    perror("Error in reading file: ");
    pthread_exit((void*) 1);
  }

  if (fclose(file) != 0) {
    perror("Error closing file: ");
    pthread_exit((void*) 1);
  }
}

void *clientRoutine() {
  readInt("fv1.b");
  readInt("fv2.b");
  pthread_exit((void*) 0);
}

void *serverRoutine() {
  while (1) {
    sem_wait(cl);
    g *= 3;
    sem_post(sr);
  }
  pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {

  counter = 0;
  cl = (sem_t *) malloc(sizeof(sem_t));
  sr = (sem_t *) malloc(sizeof(sem_t));

  if (sem_init(cl, 0, 0) != 0 ||
      sem_init(sr, 0, 0) != 0 ) {
    perror("Error: ");
    exit(1);
  }

  if (pthread_create(&client, NULL, clientRoutine, NULL) != 0 ||
      pthread_create(&server, NULL, serverRoutine, NULL) != 0) {
    perror("Error: ");
    pthread_cancel(client);
    pthread_cancel(server);
  }

  if (pthread_join(client, retval) != 0) {
    fprintf(stderr, "Error join thread!\n");
  }
  if (retval != 0) {
    exit(1);
  }
  if (pthread_cancel(server) != 0) {
    fprintf(stderr, "Error stooping thread!\n");
  }

  printf("Tot Requests: %d\n", counter);
  return 0;
}
