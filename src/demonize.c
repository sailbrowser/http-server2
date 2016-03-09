#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

void demonize(char *dir) {
  pid_t pid, sid;

  /* Fork off the parent process */
  pid = fork();
  if (pid < 0) {
    perror("daemon fork");
    exit(EXIT_FAILURE);
  }
  /* If we got a good PID, then
     we can exit the parent process. */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /* Change the file mode mask */
  umask(0);

  /* Open any logs here */
  // int fd_log = open(LOG_FILENAME, O_WRONLY | O_CREAT | O_APPEND, 0666);
  // if(fd_log == -1) {
  //   perror("open log");
  //   exit(EXIT_FAILURE);
  // }
  // if(dup2(fd_log, STDOUT_FILENO) == -1) {
  //   perror("dup2 stdout");
  //   exit(EXIT_FAILURE);
  // };
  // if(dup2(fd_log, STDERR_FILENO) == -1) {
  //   perror("dup2 stderr");
  //   exit(EXIT_FAILURE);
  // }

  /* Create a new SID for the child process */
  sid = setsid();
  if (sid < 0) {
    /* Log any failures here */
    exit(EXIT_FAILURE);
  }


  /* Change the current working directory */
  if ((chdir(dir)) < 0) {
    /* Log any failures here */
    exit(EXIT_FAILURE);
  }

  /* Close out the standard file descriptors */
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  // close(fd_log);
}
