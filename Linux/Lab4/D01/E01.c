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

int filesize,size;
int *v;
struct timeval stop, start;

typedef struct Vparam {
  int left,right;
} Vparam;

void quicksort (int left, int right);

void *ptRoutine(void *vp) {
  quicksort(((Vparam*)vp)->left, ((Vparam*)vp)->right);
  pthread_exit(NULL);
}

void quicksort (int left, int right) {
  int i, j, x, tmp;
  pthread_t tid;
  if (left >= right) return;
  x = v[left];
  i = left - 1;
  j = right + 1;
  while (i < j) {
    while (v[--j] > x);
    while (v[++i] < x);
    if (i < j) {
      tmp = v[i];
      v[i] = v[j];
      v[j] = tmp;
    }
  }
  if ((left-j) < size) {
    quicksort (left, j);
  } else {
    Vparam *vp = (Vparam*)malloc(sizeof(Vparam));
    vp->left=left;
    vp->right=j;
    if (pthread_create (&tid, NULL, ptRoutine, vp)) {
      perror("pthread_create");
      exit(-1);
    }
  }

  if ((j+1-right) < size) {
    quicksort (j + 1, right);
  } else {
    Vparam *vp = (Vparam*)malloc(sizeof(Vparam));
    vp->left=j+1;
    vp->right=right;
    if (pthread_create (&tid, NULL, ptRoutine, vp)) {
      perror("pthread_create");
      exit(-1);
    }
  }
}

int main(int argc, char const *argv[]) {

  if (argc != 3 || atoi(argv[2]) <= 0) {
    fprintf(stderr, "Usage: %s <filepath> <size>\n", argv[0]);
    exit(1);
  }

  struct stat sb;

  if (stat(argv[1], &sb) == -1) {
    perror("stat error");
    exit(1);
  }

  gettimeofday(&start, NULL);

  // check if entered path corresponds to an existing file
  if (!S_ISREG(sb.st_mode)) {
    printf("Error: Not a regular file!\n");
    exit(1);
  }

  size = atoi(argv[2]);
  filesize = sb.st_size/sizeof(int);

  int fd;

  if ((fd = open (argv[1], O_RDONLY)) == -1)
    perror ("open");

  /* mmap the input file */
  if ((v = (int*) mmap ((caddr_t) 0, filesize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0))  == MAP_FAILED){
    close(fd);
    perror("mmap error for input");
    exit(-1);
  }

  close(fd);

  quicksort(0, filesize-1);

  gettimeofday(&stop, NULL);
  printf("tot milliseconds:\t%lu\n", stop.tv_usec - start.tv_usec);

  return 0;
}
