#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "request.h"

bool is_not_stop_char(char c) {
  if(c == '?' || isspace(c)) {
    return false;
  }
  return true;
}

void http_request_init(struct http_request *req) {
  req->method = UNKNOWN;
  req->path[0]=0;
}

int http_request_parse(struct http_request *req, char *buf, size_t len) {
  size_t index = 0;
  if(len < 5) {
    return -1;
  }
  if(strncmp("GET ", buf, 4) == 0) {
    req->method = GET;
    index = 4;
  } else {
    return -1;
  }
  if(buf[index++] != '/') {
    return -1;
  }
  int i = 0;
  while(is_not_stop_char(buf[index]) && index < len)
  {
    req->path[i++] = buf[index++];
  }
  req->path[i] = 0;
  if(i == 0) {
   strncpy(req->path, "index.html", 11);
  }
  return index;
}
