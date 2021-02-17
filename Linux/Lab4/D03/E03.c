#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>

int k, counter;
float *v1,*v2,**m,*v;
sem_t *mutex;

void *routine(void *index) {
  int idx = *((int*)index);
  v[idx] = 0;

  for (size_t i = 0; i < k; i++) {
    v[idx] += m[idx][i]*v2[i];
  }

  sem_wait(mutex);
  counter--;
  if (!counter) {
    // do last operation
    float res=0;
    for (size_t i = 0; i < k; i++) {
      res += v1[i]*v[i];
    }
    printf("Result: %f\n", res);
  }
  sem_post(mutex);
  pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {

  if (argc != 2 || atoi(argv[1]) <= 0) {
    fprintf(stderr, "Usage: %s <int gt 0>\n", argv[0]);
    exit(1);
  }

  k = atoi(argv[1]);

  v1 = (float*) malloc(k*sizeof(float));
  v2 = (float*) malloc(k*sizeof(float));
  v = (float*) malloc(k*sizeof(float));
  m = (float**) malloc(k*sizeof(float*));
  for (size_t i = 0; i < k; i++) {
    m[i] = (float*) malloc(k*sizeof(float));
  }
  counter = k;
  pthread_t tid[k];

  // init semaphore
  mutex = (sem_t*) malloc(sizeof(sem_t));
  if (sem_init(mutex, 0, 1) == -1) {
    perror("sem_init");
    exit(1);
  }

  // srand((unsigned)time(NULL));
  srand((unsigned)10);

  // fill array and matrix
  for (int i = 0; i < k; i++) {
    v1[i] = (float)(rand()%11-5)/10;
    v2[i] = (float)(rand()%11-5)/10;
    printf("v1[%d] = %f\n", i, v1[i]);
    printf("v2[%d] = %f\n", i, v2[i]);
  }

  for (int i = 0; i < k; i++) {
    for (int j = 0; j < k; j++) {
      m[i][j] = (float)(rand()%11-5)/10;
      printf("m[%d][%d] = %f\n", i, j, m[i][j]);
    }
  }

  // create pthreads
  for (int i = 0; i < k; i++) {
    int *index = (int*) malloc(sizeof(int));
    *index = i;
    if (pthread_create(&tid[i], NULL, routine, index) != 0) {
      perror("pthread_create");
      exit(1);
    }
  }

  // wait for pthreads
  for (int i = 0; i < k; i++) {
    if (pthread_join(tid[i], NULL)) {
      perror("pthread_join");
      exit(1);
    }
  }

  printf("The end\n");

  return 0;
}
