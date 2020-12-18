#define MG_ENABLE_CALLBACK_USERDATA 1
#include "httplib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mgos.h"

static const char *USER_AGENT = "ESP32";
static HTTPReq_t _req;

HTTPReq_t *http_create_req(char *url, const int method, const char *content_type) {
    _req.url = url;
    _req.method = method;
    memset(_req.raw_body, '\0', sizeof(_req.raw_body));
    memset(_req.header, '\0', sizeof(_req.header));
    strcat(_req.header, "Content-Type:");
    strcat(_req.header, content_type);
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
HTTPRes_t *http_create_res() {
    HTTPRes_t *res = malloc(sizeof(HTTPRes_t));
    res->success = false;
    res->status = -1;
    res->content_length = 0;
    res->body = NULL;
    res->finish = false;
    return res;
}

bool mgos_mgos_httplib_init() { return true; }
void http_free_res(HTTPRes_t *res) {
    free(res->body);
    free(res);
}
void http_req_add_header(HTTPReq_t *req, char *name, char *value) {
    sprintf(req->header, "%s%s: %s\r\n", req->header, name, value);
}
void http_add_form_int(HTTPReq_t *req, char *name, int val) {
    const char *prefix = (strlen(req->raw_body) == 0) ? "" : "&";

    sprintf(req->raw_body, "%s%s%s=%d", req->raw_body, prefix, name, val);
}
void http_add_form_str(HTTPReq_t *req, char *name, char *val) {
    const char *prefix = (strlen(req->raw_body) == 0) ? "" : "&";
    sprintf(req->raw_body, "%s%s%s=%s", req->raw_body, prefix, name, val);
}
void http_add_form_long(HTTPReq_t *req, char *name, long val) {
    const char *prefix = (strlen(req->raw_body) == 0) ? "" : "&";
    sprintf(req->raw_body, "%s%s%s=%ld", req->raw_body, prefix, name, val);
}
void http_add_form_float(HTTPReq_t *req, char *name, float val) {
    const char *prefix = (strlen(req->raw_body) == 0) ? "" : "&";
    sprintf(req->raw_body, "%s%s%s=%.3f", req->raw_body, prefix, name, val);
}
void http_set_request_body(HTTPReq_t *req, char *body) { strcpy(req->raw_body, body); }

/**!
 * @brief １行読み込み
 * @param data 読み込み元文字列
 * @param buff 1行データの格納用文字列バッファ
 * @param bufflen 格納文字列のバッファサイズ
 * @param next 継続行を読み込むための次の配列用ポインタ(NULLのとき終了)
 * @return 0以上のとき取得成功。マイナスのときエラー
 *
    char *next = data;
    char buff[512];
    do {
        int len = readline(next, buff, 512, &next);
        cout << len << "[" << buff << "]" << endl;
    } while (next != NULL);
 */
int readline(char *data, int datalen, char *buff, int bufflen, char **next) {
    char *p = memchr(data, '\n', datalen);
    int len;
    //改行検索
    if (p == NULL) {
        len = bufflen - 1 > datalen ? datalen : bufflen - 1;
        strncpy(buff, data, len);
        buff[len] = '\0';
        *next = NULL;
        return len;
    }

    //次の改行後文字列開始位置nextの設定
    len = p - data;
    *next = p + 1;
    if (len > bufflen - 1) {
        *next = NULL;
        return -1;
    }
    // buffer内のNULLターミネイト
    strncpy(buff, data, len);
    if (buff[len - 1] == '\r') {
        buff[len - 1] = '\0';
        return len + 1;
    } else {
        buff[len] = '\0';
        return len;
    }
}
/**!
 * @brief 文字列分割して指定インデックスの要素を抜き出し
 * @param src 元文字列
 * @param delim 区切り文字の集合(それぞれのcharで区切られる)
 * @param index 取得要素インデックス(0~)
 * @param buff 取得文字列格納バッファ
 * @param bufflen バッファ長
 * @return 抽出された要素文字列の先頭charポインタ。見つからない場合はNULL
 */
char *split(char *src, char *delim, int index, char *buff, int bufflen) {
    int l = strlen(src);
    // strtok用にsrcのコピー文字列生成
    char *data = (char *)malloc(l * sizeof(char));
    strcpy(data, src);

    char *p = strtok(data, delim);
    int idx = 0;

    while (p != NULL) {
        if (idx == index) {
            if (strlen(p) > bufflen - 1) {
                //要素文字列がバッファ長を超えている場合はバッファまで一部格納
                strncpy(buff, p, bufflen - 1);
                buff[bufflen - 1] = '\0';
            } else {
                //超えていない場合は全部コピー
                strcpy(buff, p);
            }
            free(data);
            return buff;
        }
        idx++;
        p = strtok(NULL, delim);
    }
    //見つからない場合
    free(data);
    buff[0] = '\0';
    return NULL;
}
static int read_response(char *resdata, int message_len, HTTPRes_t *res) {
    int content_length = -1;
    char buff[512];
    char elmbuf[64];
    int line_no = 0;
    char *next = resdata;
    int read_len = 0;

    do {
        int l = readline(next, (message_len - read_len), buff, sizeof(buff), &next);
        read_len += l;
        LOG(LL_INFO, ("%d\t[%s]", line_no, buff));

        if (line_no == 0) {
            // STATUS 取得
            char *status = split(buff, (char *)" ", 1, elmbuf, sizeof(elmbuf));
            if (status == NULL) {
                return -1;
            }
            res->status = atoi(status);
        } else {
            if (strlen(buff) == 0) {
                //ヘッダ終了
                int body_len = message_len - read_len;
                LOG(LL_INFO, ("content_length=%d, body_len=%d", res->content_length, body_len));
                res->body = (char *)calloc(sizeof(char), body_len + 1);
                strncpy(res->body, next, res->content_length);
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
static void ev_handler(struct mg_connection *c, int ev, void *p, void *ud) {
    HTTPRes_t *res = (HTTPRes_t *)ud;
    if (ev == MG_EV_HTTP_REPLY) {
        struct http_message *hm = (struct http_message *)p;
        LOG(LL_INFO, ("Response Receive, %d", (int)hm->message.len));
        c->flags |= MG_F_CLOSE_IMMEDIATELY;
        // fwrite(hm->message.p, 1, (int)hm->message.len, stdout);
        // putchar('\n');

        read_response((char *)hm->message.p, (int)hm->message.len, res);
    } else if (ev == MG_EV_CLOSE) {
        res->finish = true;
        res->success = (res->status == 200);
    }
}
HTTPRes_t *http_send(HTTPReq_t *req) {
    LOG(LL_INFO, ("HTTP SEND REQUEST"));
    struct mg_mgr *mgr = mgos_get_mgr();
    struct mg_connection *conn;
    HTTPRes_t *res = http_create_res();
    LOG(LL_INFO, ("REQUEST: %s", req->url));
    LOG(LL_INFO, ("REQUEST: %s", req->header));
    LOG(LL_INFO, ("REQUEST: %s", req->raw_body));
    conn = mg_connect_http(mgr, (mg_event_handler_t)ev_handler, res, req->url, req->header, req->raw_body);

    while (!res->finish) {
        mg_mgr_poll(mgr, 100);
    }
    LOG(LL_INFO, ("Finish Send"));
    return res;
}
char *HTTPReq_getURL(HTTPReq_t *req) { return req->header; }
int HTTPReq_getMethod(HTTPReq_t *req) { return req->method; }
char *HTTPReq_getHeader(HTTPReq_t *req) { return req->header; }
char *HTTPReq_getRawBody(HTTPReq_t *req) { return req->raw_body; }

int HTTPRes_getStatus(HTTPRes_t *res) { return res->status; }