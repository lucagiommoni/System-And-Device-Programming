#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE 16
#define LOOP 100

int prob;
pthread_t producer_thread, consumer_thread;
void *retval;

typedef struct Buffer {
  sem_t *empty, *full;
  int in, out;
  long long *buffer;
  int size;
} Buffer;

Buffer * buffer_init(int size);

Buffer *normal, *urgent;

Buffer *buffer_init(int size){
  Buffer *b = (Buffer *) malloc(sizeof(Buffer));
  b->size = size;
  b->buffer = (long long *) malloc(b->size * sizeof(long long));
  b->in = 0;
  b->out = 0;
  b->empty = (sem_t *) malloc(sizeof(sem_t)); sem_init (b->empty, 0, b->size);
  b->full = (sem_t *) malloc(sizeof(sem_t));  sem_init (b->full, 0, 0);
  return b;
}

/* Store an integer in the buffer */
void put (Buffer *b, long long data){
  sem_wait(b->empty);
  b->buffer[b->in] = data;
  b->in = (b->in + 1) % b->size;
  sem_post(b->full);
}

/* Read and remove an integer from the buffer */
long long get (void){
  long long data;
  if(sem_trywait(urgent->full) == -1 && errno == EAGAIN) {
    sem_wait(normal->full);
    data = normal->buffer[normal->out];
    normal->out = (normal->out + 1) % normal->size;
    sem_post(normal->empty);
  } else {
    data = urgent->buffer[urgent->out];
    urgent->out = (urgent->out + 1) % urgent->size;
    sem_post(urgent->empty);
  }
  return data;
}

long long current_timestamp() {
  struct timeval te;
  gettimeofday(&te, NULL); // get current time
  long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
  return milliseconds;
}

void *producer() {
  for (int i = 0; i < LOOP; i++) {
    usleep((useconds_t)((rand() % 10) + 1));
    long long ms = current_timestamp();
    if (((rand() % 10) + 1) <= prob) {
      printf("producer: buffer 0 - %lld\n", ms);
      put(normal, ms);
    } else {
      printf("producer: buffer 1 - %lld\n", ms);
      put(normal, ms);
    }
  }
  pthread_exit((void*) 0);
}

void *consumer() {
  for (int i = 0; i < LOOP; i++) {
    usleep((useconds_t) 10);
    printf("consumer: %lld\n", get());
  }
  pthread_exit((void*) 0);
}

int main(int argc, char const *argv[]) {

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <1-10>\n", argv[0]);
    exit(1);
  }
  // init prob
  prob = atoi(argv[1]);

  if (prob <= 0 || prob > 10) {
    fprintf(stderr, "Usage: %s <1-10>\n", argv[0]);
    exit(1);
  }

  // init buffers
  normal = buffer_init(BUFFER_SIZE);
  urgent = buffer_init(BUFFER_SIZE);
  // init random function
  time_t t;
  srand((unsigned) time(&t));

  if (pthread_create(&producer_thread, NULL, producer, NULL) != 0 ||
      pthread_create(&consumer_thread, NULL, consumer, NULL) != 0) {
    perror("Error: ");
    pthread_cancel(producer_thread);
    pthread_cancel(consumer_thread);
  }

  if (pthread_join(producer_thread, retval) != 0 ||
      pthread_join(consumer_thread, retval) != 0 ) {
    fprintf(stderr, "Error join thread!\n");
  }

  return 0;
}
