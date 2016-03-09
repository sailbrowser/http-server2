#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

#include "server.h"
#include "thread_info.h"

int main(int argc, char *argv[])
{
  int opt;
  char *ip = (char *)"127.0.0.1";
  unsigned port = 8080;
  char * dir;
  while((opt = getopt(argc, argv, "h:p:d:")) != -1){
      switch (opt){
      case 'h':
        ip = optarg;
        break;
      case 'p':
        port = strtol(optarg, NULL, 10);
        if(port == 0) {
          exit(EXIT_FAILURE);
        }
        break;
      case 'd':
        dir = optarg;
        break;
      default:
        exit(EXIT_FAILURE);
      };
    };
  if(optind < 7) {
    printf("Usage: %s -h <ip> -p <port> -d <directory>", argv[0]);
    exit(EXIT_FAILURE);
  }

  // disable SIGHUP for online test
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = SA_RESTART;
  sigfillset(&sa.sa_mask);
  sigaction(SIGHUP, &sa, NULL);

  demonize(dir);

  int num_cores = get_num_cores();
  int worker_sv[num_cores];
  struct thread_info *tinfo = calloc(num_cores, sizeof(struct thread_info));
  if (tinfo == NULL) {
    perror("calloc");
  }

  for (int i = 0; i < num_cores; ++i)
  {
    int sv[2];
    if(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0){
      perror("socketpair");
      exit(1);
    }
    // if (pipe(sv) == -1) {
    //     perror("pipe");
    //     exit(EXIT_FAILURE);
    // }
    tinfo[i].thread_num = i;
    tinfo[i].pipe_socket = sv[0];
    // printf("create thread %d pipe %d\n", tinfo[i].thread_num, tinfo[i].pipe_socket);
    int result=pthread_create(&tinfo[i].thread_id, NULL, thread_worker, &tinfo[i]);
    if(result != 0) {
      perror("pthread_create");
      exit(1);
    }
    worker_sv[i] = sv[1];
  }

  server(port, ip, worker_sv, num_cores);

  for (int i = 0; i < num_cores; ++i)
  {
    int status_addr;
    pthread_join(tinfo[i].thread_id, (void**)&status_addr);
  }
  free(tinfo);
  return EXIT_SUCCESS;
}
