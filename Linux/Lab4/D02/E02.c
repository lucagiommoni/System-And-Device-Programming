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

int main(int argc, char const *argv[]) {

  if (argc != 2 || atoi(argv[1]) <= 0) {
    fprintf(stderr, "Usage: %s <int gt 0>\n", argv[0]);
    exit(1);
  }

  int k = atoi(argv[1]);
  float v1[k],v2[k],m[k][k];

  srand((unsigned)10);

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

  float middle[k];

  for (int i = 0; i < k; i++) {
    middle[i] = 0;
    for (int j = 0; j < k; j++) {
      middle[i] += m[i][j]*v2[j];
    }
  }

  float res = 0;

  for (int i = 0; i < k; i++) {
    res += v1[i]*middle[i];
  }

  printf("Result: %f\n", res);

  return 0;
}
