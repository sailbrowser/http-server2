#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#include "request.h"
#include "response.h"
#include "thread_info.h"

int MAX_EVENTS=32;

int set_nonblock(int fd)
{
  int flags;
#if defined(O_NONBLOCK)
  if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
    flags = 0;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
  flags = 1;
  return ioctl(fd, FIOBIO, &flags);
#endif
}

int send_socket_to_pipeline(int fd, int num) {
  // printf("send to fd=%d socket=%d\n", fd, num);
  char *buf = (char *)&num;
  return write(fd, buf, sizeof(int));
}

int recv_socket_from_pipeline(int fd, int *num) {
  char data[sizeof(int)];
  int ret = read(fd, data, sizeof(int));
  *num = *(int *)data;
  // printf("recv from fd=%d socket=%d\n", fd, *num);
  return ret;
}

void *thread_worker(void *arg)
{
  struct thread_info *tinfo = arg;
  static char buffer[4096];
  int pipe_socket = tinfo->pipe_socket;
  // printf("thread %d pipe %d\n", tinfo->thread_num, pipe_socket);

  int epoll = epoll_create1(0);

  struct epoll_event pipe_event;
  pipe_event.data.fd = pipe_socket;
  pipe_event.events = EPOLLIN;
  if(epoll_ctl(epoll, EPOLL_CTL_ADD, pipe_socket, &pipe_event) == -1) {
    perror("Error epoll_ctl connection sock\n");
    exit(1);
  }

  while (1) {
    struct epoll_event events[MAX_EVENTS];
    int n_events = epoll_wait(epoll, events, MAX_EVENTS, -1);
    if(n_events == -1) {
      perror("Error epoll_wait\n");
      exit(1);
    }

    for (int i = 0; i < n_events; ++i)
    {
      if(events[i].data.fd == pipe_socket) {
        int accept_socket;
        recv_socket_from_pipeline(pipe_socket, &accept_socket);

        struct epoll_event new_event;
        new_event.data.fd = accept_socket;
        new_event.events = EPOLLIN | EPOLLET;
        if(epoll_ctl(epoll, EPOLL_CTL_ADD, accept_socket, &new_event) == -1) {
          perror("Error epoll_ctl connection sock\n");
          exit(1);
        }
      } else {
        int cur_socket = events[i].data.fd;
        // printf("socket = %d\n", cur_socket);
        ssize_t size = recv(cur_socket, buffer, sizeof(buffer), MSG_NOSIGNAL);
        // printf("size = %lu\n", size);
        if (size > 0) {
          buffer[size] = 0;
          // printf("%s", buffer);
          struct http_request req;
          http_request_init(&req);
          struct http_response res;
          http_response_init(&res);
          int fd;
            // struct http_io *w_read = (struct http_io *)watcher;
          if(http_request_parse(&req, buffer, size) == -1) {
            res.code = _501;
          } else if(req.method != GET) {
            res.code = _501;
          } else {
            fd = open(req.path, O_RDONLY);
            if(fd == -1) {
              res.code = _404;
            } else {
              res.code = _200;
                /* get the size of the file to be sent */
              struct stat stat_buf;
              fstat(fd, &stat_buf);
              res.content_length = stat_buf.st_size;
              res.content_type = html;
            }
            render_header(&res);
            send(cur_socket, res.header, strlen(res.header), MSG_NOSIGNAL);
            if(res.code == _200) {
              ssize_t l;
              while((l = read(fd, buffer, sizeof(buffer))) > 0) {
                send(cur_socket, buffer, l, MSG_NOSIGNAL);
              }
              close(fd);
            }
          }
        }
        shutdown(cur_socket, SHUT_RDWR);
        close(cur_socket);
      }
    }
  }

  // sem_post(&semaphore);
  return NULL;
}

void server(int port, char *ip_address, int *sv, int cores) {
 int main_socket = socket(PF_INET, SOCK_STREAM, 0);
 if(main_socket == -1) {
   perror("Error initializing socket\n");
   exit(1);
 }
 set_nonblock(main_socket);

 struct hostent *he;
 if((he = gethostbyname(ip_address)) == NULL) {
   perror("gethostbyname:");
   exit(1);
 }

 struct sockaddr_in sa;
 sa.sin_family = AF_INET;
 sa.sin_port = htons(port);
 memcpy(&sa.sin_addr, he->h_addr_list[0], he->h_length);

 int enable = 1;
 if (setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
   perror("setsockopt");
 }
 int res_bind = bind(main_socket, (struct sockaddr *) &sa, sizeof(sa));
 if(res_bind == -1) {
   perror("Error binding to socket\n");
   exit(1);
 }

 int res_listen = listen(main_socket, SOMAXCONN);
 if(res_listen == -1) {
   perror("Error listening\n");
   exit(1);
 }

 int epoll = epoll_create1(0);
 struct epoll_event event;
 event.data.fd = main_socket;
 event.events = EPOLLIN;
 if(epoll_ctl(epoll, EPOLL_CTL_ADD, main_socket, &event) == -1) {
   perror("Error epoll_ctl listening sock\n");
   exit(1);
 }

 while (1) {
   struct epoll_event events[MAX_EVENTS];
   int n_events = epoll_wait(epoll, events, MAX_EVENTS, -1);
   if(n_events == -1) {
     perror("Error epoll_wait\n");
     exit(1);
   }

   for (int i = 0; i < n_events; ++i)
   {
     if(events[i].data.fd == main_socket) {
       struct sockaddr from_address;
       socklen_t len_address = sizeof(from_address);
       memset(&from_address, 0, sizeof(from_address));
       int new_socket = accept(main_socket, &from_address, &len_address);
       if(new_socket == -1) {
         perror("Error accept connection\n");
         exit(1);
       }
       set_nonblock(new_socket);
       static int index = 0;
       int pipe = sv[index++];
       index %= cores;
       // send(sv[index],&)
       // printf("send to %d socket %d\n", pipe, new_socket);
       send_socket_to_pipeline(pipe, new_socket);
     }
   }
 }
}
