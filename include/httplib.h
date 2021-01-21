#ifndef HTTPLIB_H_
#define HTTPLIB_H_
#include <stdbool.h>

#define HTTP_CT_FORM "application/x-www-form-urlencoded"
#define HTTP_CT_JSON "application/json"

enum HTTP_METHOD { M_GET, M_POST };

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
    bool recv;
    char *body;
    char *header;
    int content_length;
    char header_val[256];
} HTTPRes_t;

HTTPReq_t *http_create_req(char *url, const int method, const char *content_type);
void http_req_add_header(HTTPReq_t *req, char *name, char *value);

HTTPRes_t *http_send(HTTPReq_t *req);

void http_add_form_val(HTTPReq_t *req, char *name, char *val);
void http_set_request_body(HTTPReq_t *req, char *body);

char *http_res_hval_buff(HTTPRes_t *res, char *name, char *buff, int bufflen);
char *http_res_header_value(HTTPRes_t *res, char *name);
void http_res_free(HTTPRes_t *res);

char *HTTPReq_getURL(HTTPReq_t *req);
int HTTPReq_getMethod(HTTPReq_t *req);
char *HTTPReq_getHeader(HTTPReq_t *req);
char *HTTPReq_getRawBody(HTTPReq_t *req);

int HTTPRes_getStatus(HTTPRes_t *res);
char *HTTPRes_getBody(HTTPRes_t *res);
bool HTTPRes_isSuccess(HTTPRes_t *res);
int HTTPRes_getContentLength(HTTPRes_t *res);
char *HTTPRes_getHeader(HTTPRes_t *res);

#endif