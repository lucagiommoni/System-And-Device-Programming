#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread.h"
#include "semaphore.h"

sem_t *readers_ME, *writers_ME, *w;
int nr = 0;

static void *writer (void *a) {
  int *i = (int *) a;
  usleep(rand() % 500);
  printf("Writer %d trying to write\n", *i);
  sem_wait(writers_ME);
  sem_wait(w);
  printf("Thread n. %d writing\n", *i);
  usleep(500);
  sem_post(w);
  sem_post(writers_ME);
  return NULL;
}

static void *reader (void *a) {
  int *i = (int *) a;
  usleep(rand() % 500);
  printf("Reader %d trying to read\n", *i);
  sem_wait(readers_ME);
  nr++;
  if (nr == 1) {
    sem_wait(w);
  }
  sem_post(readers_ME);
  printf("Thread n. %d reading\n", *i);
  usleep(500);
  sem_wait(readers_ME);
  nr--;
  if (nr == 0)
    sem_post(w);
  sem_post(readers_ME);
  return NULL;
}

int main(int argc, char const *argv[]) {
  if (argc != 2 || atoi(argv[1]) <= 0) {
    printf("USAGE: %s <positive non-zero integer>\n", argv[0]);
    exit(1);
  }
  int num_th = atoi(argv[1]);
  pthread_t readers_TH[num_th], writers_TH[num_th];

  w = (sem_t *) malloc (sizeof (sem_t));
  readers_ME = (sem_t *) malloc (sizeof (sem_t));
  writers_ME = (sem_t *) malloc (sizeof (sem_t));
  sem_init (w, 0, 1);
  sem_init (readers_ME, 0, 1);
  sem_init (writers_ME, 0, 1);

  setbuf(stdout,0);

  /* Create the threads */
  for (int i = 0; i < num_th; i++) {
    int *v = (int *) malloc (sizeof (int));
    *v = i;
    if(pthread_create(&readers_TH[i], NULL, reader, v) != 0 ||
       pthread_create(&writers_TH[i], NULL, writer, v) != 0) {
      printf("Error to create thread!\n");
      exit(1);
    }
  }

  for (int i = 0; i < num_th; i++) {
    if (pthread_join(readers_TH[i], NULL) != 0 ||
        pthread_join(writers_TH[i], NULL) != 0) {
      fprintf(stderr, "Error join thread!\n");
    }
  }

  printf("The End.\n");

  pthread_exit (0);

}
