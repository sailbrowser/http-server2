#define _POSIX_C_SOURCE 2
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>

#include "server.h"

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

  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sigfillset(&sa.sa_mask);
  sigaction(SIGHUP, &sa, NULL);

  pid_t pid, sid;

  /* Fork off the parent process */
  pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }
  umask(0);
  sid = setsid();
  if (sid < 0) {
    exit(EXIT_FAILURE);
  }
  /* Change the current working directory */
  if ((chdir(dir)) < 0) {
    /* Log any failures here */
    exit(EXIT_FAILURE);
  }

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  server(port, ip);
  return EXIT_SUCCESS;
}
