#include <pthread.h>

struct thread_info {
  pthread_t thread_id;  /* ID returned by pthread_create() */
  int thread_num;       /* Application-defined thread # */
  int pipe_socket;
};
