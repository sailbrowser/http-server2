#define BUFF_SIZE 4096

enum http_method {GET, HEAD, POST, UNKNOWN};

struct http_request {
  enum http_method method;
  char path[BUFF_SIZE];
};

void http_request_init(struct http_request *req);
int http_request_parse(struct http_request *req, char *buf, size_t len);
