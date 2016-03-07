#define HEADER_MAX_SIZE 1024

enum http_code { _200= 0, _404, _501}; // change status_dic[]
enum http_content_type { none = 0, html, jpg }; // change content_type_dic[]

struct http_response {
  enum http_code code;
  size_t content_length;
  enum http_content_type content_type;
  char header[HEADER_MAX_SIZE];
};

void http_response_init(struct http_response *res);
int render_header(struct http_response *res);
