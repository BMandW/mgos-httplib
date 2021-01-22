#define MG_ENABLE_CALLBACK_USERDATA 1
#include "httplib.h"

#include <mgos.h>

#include "linechar.h"

static const char *USER_AGENT = "ESP32";
static HTTPReq_t _req;

HTTPReq_t *http_create_req(char *url, const int method, const char *content_type) {
    _req.url = (char *)url;
    _req.method = method;
    memset(_req.raw_body, '\0', sizeof(_req.raw_body));
    memset(_req.header, '\0', sizeof(_req.header));
    strcat(_req.header, "Content-Type:");
    if (content_type == NULL) {
        strcat(_req.header, HTTP_CT_FORM);
    } else {
        strcat(_req.header, content_type);
    }
    strcat(_req.header, "\r\n");

    char *user_agent = (char *)mgos_sys_config_get_httplib_user_agent();
    if (user_agent == NULL) {
        user_agent = (char *)USER_AGENT;
    }
    strcat(_req.header, "User-Agent:");
    strcat(_req.header, user_agent);
    strcat(_req.header, "\r\n");
    return &_req;
}
/**!
 * HTTP Responseオブジェクトの生成。
 * (http_send関数内で生成されるため外部からは使用しない)
 * 生成されたHTTPRes_tオブジェクトは http_res_free関数で開放する必要がある。
 * @return HTTPレスポンスオブジェクト
 */
HTTPRes_t *http_create_res() {
    HTTPRes_t *res = malloc(sizeof(HTTPRes_t));
    res->success = false;
    res->recv = false;
    res->status = -1;
    res->content_length = 0;
    res->header = NULL;
    res->body = NULL;
    res->finish = false;
    return res;
}

/**!
 * ライブラリ初期関数
 */
bool mgos_mgos_httplib_init() { return true; }

void http_res_free(HTTPRes_t *res) {
    if (res->body != NULL) {
        free(res->body);
    }
    if (res->header_val != NULL) {
        free(res->header);
    }
    free(res);
}

void http_req_add_header(HTTPReq_t *req, char *name, char *value) {
    char buff[512];
    sprintf(buff, "%s: %s\r\n", name, value);
    strcat(req->header, buff);
}
void http_add_form_val(HTTPReq_t *req, char *name, char *val) {
    const char *prefix = (strlen(req->raw_body) == 0) ? "" : "&";
    char buff[1024];
    sprintf(buff, "%s%s=%s", prefix, name, val);
    strcat(req->raw_body, buff);
}

void http_set_request_body(HTTPReq_t *req, char *body) { strcpy(req->raw_body, body); }

/**!
 * レスポンスデータの読み取り
 * @param resdata レスポンスデータ文字列
 * @param message_len レスポンスデータ文字列長
 * @param res レスポンスオブジェクトポインタ
 * @return 成功のとき0。
 */
static int _read_response(char *resdata, int message_len, HTTPRes_t *res) {
    char buff[512];
    char elmbuf[64];
    char *header_start = NULL;
    int line_no = 0;
    char *next = resdata;
    int read_len = 0;
    bool is_header = true;

    do {
        int l = readline(next, (message_len - read_len), buff, sizeof(buff), &next);
        read_len += l;
        LOG(LL_INFO, ("%d\t%d\t[%s]", line_no, read_len, buff));

        if (line_no == 0) {
            // STATUS 取得
            char *status = split(buff, (char *)" ", 1, elmbuf, sizeof(elmbuf));
            if (status == NULL) {
                return -1;
            }
            res->status = atoi(status);
            header_start = next;
        } else {
            if (strlen(buff) == 0) {
                if (is_header == true) {
                    //ヘッダ終了
                    is_header = false;
                    int body_len = message_len - read_len;

                    LOG(LL_INFO, ("content_length=%d, body_len=%d", res->content_length, body_len));
                    if (body_len > 0) {
                        res->body = (char *)calloc(sizeof(char), body_len + 1);
                        strncpy(res->body, next, res->content_length);
                    }
                    int header_len = next - header_start;
                    LOG(LL_INFO, ("header_len=%d", header_len));
                    res->header = (char *)calloc(sizeof(char), header_len + 1);

                    strncpy(res->header, header_start, header_len);
                    //ヘッダの末尾の空行除去
                    for (int i = header_len; i >= 0; i--) {
                        char c = res->header[i];
                        if (c == '\n' || c == '\r' || c == '\0') {
                            res->header[i] = '\0';
                        } else {
                            break;
                        }
                    }
                }
            } else {
                char *header_name = split(buff, (char *)":", 0, elmbuf, sizeof(elmbuf));
                int i = 0;
                while (header_name[i] != '\0') {
                    header_name[i] = (char)tolower((int)header_name[i]);
                    i++;
                }
                if (strcmp(header_name, "content-length") == 0) {
                    char *header_value = split(buff, (char *)":", 1, elmbuf, sizeof(elmbuf));
                    res->content_length = atoi(header_value);
                }
            }
        }
        line_no++;
    } while (next != NULL);

    return 0;
}

/**!
 * HTTP リクエスト後のイベントハンドラ
 */
static void _event_handler(struct mg_connection *c, int ev, void *p, void *ud) {
    HTTPRes_t *res = (HTTPRes_t *)ud;
    // LOG(LL_DEBUG, ("EV %d", ev));
    if (ev == MG_EV_HTTP_REPLY) {
        res->recv = true;
        struct http_message *hm = (struct http_message *)p;
        LOG(LL_DEBUG, ("Response Receive, %d", (int)hm->message.len));
        c->flags |= MG_F_CLOSE_IMMEDIATELY;
        _read_response((char *)hm->message.p, (int)hm->message.len, res);
    } else if (ev == MG_EV_CLOSE) {
        if (res->recv) {
            res->success = ((int)(res->status / 100) == 2);  // statusが200番台
        } else {
            res->success = false;
        }
        res->finish = true;
        LOG(LL_DEBUG, ("CLOSE %d, %d %d", res->finish, res->success, res->recv));
    }
}
HTTPRes_t *http_send(HTTPReq_t *req) {
    LOG(LL_INFO, ("HTTP SEND REQUEST"));
    struct mg_mgr *mgr = mgos_get_mgr();
    HTTPRes_t *res = http_create_res();
    LOG(LL_INFO, ("REQUEST URL: [%s]", req->url));
    LOG(LL_DEBUG, ("REQUEST HEADER: [%s]", req->header));
    LOG(LL_DEBUG, ("REQUEST BODY: [%s]", req->raw_body));
    mg_connect_http(mgr, (mg_event_handler_t)_event_handler, res, req->url, req->header, req->raw_body);

    //レスポンス受信を待つ
    int t = 0;
    for (t = 0; t < 30 && res->finish == false; t++) {
        LOG(LL_DEBUG, ("poll %d, %d %d", res->finish, res->success, res->recv));
        mg_mgr_poll(mgr, 100);
        sleep(1);
    }
    if (t >= 30) {
        LOG(LL_INFO, ("Timeout %d", res->success));
    } else {
        LOG(LL_INFO, ("Finish Send"));
    }
    return res;
}

char *http_res_hval_buff(HTTPRes_t *res, char *name, char *buff, int bufflen) {
    char linebuff[256];
    char namebuff[64];
    char *next = res->header;
    int header_len = strlen(res->header);

    do {
        readline(next, header_len, linebuff, sizeof(linebuff), &next);
        char *header_name = split(linebuff, (char *)":", 0, namebuff, sizeof(namebuff));
        int i = 0;
        while (header_name[i] != '\0') {
            header_name[i] = (char)tolower((int)header_name[i]);
            i++;
        }
        if (strcmp(header_name, name) == 0) {
            char *header_value = linebuff + strlen(header_name) + 1;  // name長さ+1(:)がvalue開始位置
            if (header_value != NULL) {
                //:のあとのスペースを読み飛ばす
                while (header_value[0] == ' ') {
                    header_value++;
                }
                if (strlen(header_value) > bufflen - 1) {
                    strncpy(buff, header_value, bufflen - 1);
                } else {
                    strcpy(buff, header_value);
                }
                return buff;
            }
            return NULL;
        }
    } while (next != NULL);
    return NULL;
}
char *http_res_header_value(HTTPRes_t *res, char *name) {
    return http_res_hval_buff(res, name, res->header_val, sizeof(res->header_val));
}

// mJS インターフェース用関数
char *HTTPReq_getURL(HTTPReq_t *req) { return req->header; }
int HTTPReq_getMethod(HTTPReq_t *req) { return req->method; }
char *HTTPReq_getHeader(HTTPReq_t *req) { return req->header; }
char *HTTPReq_getRawBody(HTTPReq_t *req) { return req->raw_body; }

int HTTPRes_getStatus(HTTPRes_t *res) { return res->status; }
char *HTTPRes_getBody(HTTPRes_t *res) { return res->body; }
bool HTTPRes_isSuccess(HTTPRes_t *res) { return res->success; }
int HTTPRes_getContentLength(HTTPRes_t *res) { return res->content_length; }
char *HTTPRes_getHeader(HTTPRes_t *res) { return res->header; }