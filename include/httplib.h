#ifndef HTTPLIB_H_
#define HTTPLIB_H_
#include <stdbool.h>

static const int HTTP_METHOD_GET = 1;
static const int HTTP_METHOD_POST = 2;
static const int HTTP_METHOD_PUT = 3;
static const int HTTP_METHOD_DELETE = 4;
static const char *HTTP_CT_FORM = "application/x-www-form-urlencoded";
static const char *HTTP_CT_JSON = "application/json";

typedef struct HTTPReq {
    char *url;
    int method;
    char header[512];
    char raw_body[512];
} HTTPReq_t;
typedef struct {
    bool success;
    bool finish;
    int status;
    char *body;
    int content_length;
} HTTPRes_t;

HTTPReq_t *http_create_req(char *url, const int method, const char *content_type);
void http_req_add_header(HTTPReq_t *req, char *name, char *value);

HTTPRes_t *http_send(HTTPReq_t *req);
void http_free_res(HTTPRes_t *res);

void http_add_form_int(HTTPReq_t *req, char *name, int val);
void http_add_form_str(HTTPReq_t *req, char *name, char *val);
void http_add_form_long(HTTPReq_t *req, char *name, long val);
void http_add_form_float(HTTPReq_t *req, char *name, float val);
void http_set_request_body(HTTPReq_t *req, char *body);

char *HTTPReq_getURL(HTTPReq_t *req);
int HTTPReq_getMethod(HTTPReq_t *req);
char *HTTPReq_getHeader(HTTPReq_t *req);
char *HTTPReq_getRawBody(HTTPReq_t *req);

int HTTPRes_getStatus(HTTPRes_t *res);
#endif