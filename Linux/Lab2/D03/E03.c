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

int main(int argc, char const *argv[]) {

  if (argc != 3) {
    printf("USAGE: %s <n1> <n2>\n", argv[0]);
    exit(1);
  }

  int n1 = atoi(argv[1]);
  int n2 = atoi(argv[2]);

  if (n1 <= 0 || n2 <= 0) {
    fprintf(stderr, "Arguments error: please enter 2 positive non-zero numbers!\n");
    exit(1);
  }

  int v1[n1];
  int v2[n2];
  FILE *file;

  time_t t;
  srand((unsigned) time(&t));

  if((file = fopen("fv1.b", "wb")) == NULL) {
    perror("Error: ");
    exit(1);
  }

  for (int i = 0; i < n1; i++) {
    while((v1[i] = 10 + (rand() % 91)) % 2 != 0);
    printf("v1[%d] = %d\n", i, v1[i]);
    fwrite(&v1[i], sizeof(int), 1, file);
  }

  fclose(file);

  if((file = fopen("fv2.b", "wb")) == NULL) {
    perror("Error: ");
    exit(1);
  }

  for (int i = 0; i < n2; i++) {
    while((v2[i] = 21 + (rand() % 81)) % 2 != 1);
    printf("v2[%d] = %d\n", i, v2[i]);
    fwrite(&v2[i], sizeof(int), 1, file);
  }

  fclose(file);

  return 0;
}
