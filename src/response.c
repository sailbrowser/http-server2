#include <stdio.h>

#include "response.h"

const char *version = "HTTP/1.0";

// enum http_code { _200= 0, _404, _501}
const char *status_dic[] = {
  "200 OK",
  "404 Not Found",
  "501 Not Implemented"
};

//enum http_content_type { none = 0, html, jpg }
const char *content_type_dic[] = {
  "",
  "Content-Type: text/html\r\n",
  "Content-Type: image/jpeg\r\n"
};

const char *content_length = "Content-Length:";

void http_response_init(struct http_response *res) {
  res->code = _501;
  res->content_length = 0;
  res->content_type = none;
}

int render_header(struct http_response *res) {
  return snprintf(res->header, sizeof(res->header), "%s %s\r\n%s %lu\r\n%s\r\n",
    version,
    status_dic[res->code],
    content_length,
    res->content_length,
    content_type_dic[res->content_type]);
}
