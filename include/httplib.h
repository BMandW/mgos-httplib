#ifndef HTTPLIB_H_
#define HTTPLIB_H_
#include <stdbool.h>

#define HTTP_CT_FORM "application/x-www-form-urlencoded"
#define HTTP_CT_JSON "application/json"

enum HTTP_METHOD { M_GET, M_POST };

/**
 * HTTPリクエスト
 */
typedef struct HTTPReq {
    /** リクエストURL */
    char *url;
    /** HTTPメソッド(M_GET|M_POST) */
    int method;
    /** リクエストヘッダ 512バイト固定 */
    char header[512];
    /** リクエストボディ 512バイト固定 */
    char raw_body[512];
} HTTPReq_t;
typedef struct {
    /** リクエストの成功かどうか */
    bool success;
    /** リクエストの送受信が完了したかどうか */
    bool finish;
    /** HTTPステータスコード */
    int status;
    /** HTTP送受信イベントハンドラ用フラグ */
    bool recv;
    /** HTTPレスポンスボディ */
    char *body;
    /** HTTPヘッダ */
    char *header;
    int content_length;
    /** http_res_header_valueで使用するヘッダ値用バッファ */
    char header_val[256];
} HTTPRes_t;

/**!
 * HTTP Requestオブジェクトの作成。
 * リクエストボディは固定で512バイトのヘッダとボディを持ち開放の必要はない。
 * @param url 送信先URL
 * @param method HTTPメソッド(M_GET|M_POST)
 * @param content_type HTTP ContentType (HTTP_CT_FORM|HTTP_CT_JSON)
 * @return HTTPリクエストオブジェクト
 */
HTTPReq_t *http_create_req(char *url, const int method, const char *content_type);
/**!
 * リクエストヘッダ情報を追加する。ヘッダ全体で512バイトのバッファを持つのでそれを超えないように
 * @param req HTTP Requestオブジェクト
 * @param name ヘッダ名
 * @param value ヘッダ値
 */
void http_req_add_header(HTTPReq_t *req, char *name, char *value);

/**!
 * HTTPリクエスト送信
 * @param req HTTP リクエストオブジェクト
 * @return res HTTP レスポンスオブジェクト(要 http_res_free)
 */
HTTPRes_t *http_send(HTTPReq_t *req);

/**!
 * リクエストに文字列のフォーム値を追加する
 * @param req HTTP Requestオブジェクト
 * @param name フォームパラメータ名
 * @param val フォームパラメーダ値
 */
void http_add_form_val(HTTPReq_t *req, char *name, char *val);
/**!
 * リクエストボディに直接データをセットする。
 * josnリクエストなどで使用。
 * @param req HTTP Requestオブジェクト
 * @param body リクエストボディ文字列
 */
void http_set_request_body(HTTPReq_t *req, char *body);

/**
 * レスポンスヘッダ値取得(バッファ値指定)
 * @param res HTTPレスポンスオブジェクト
 * @param name ヘッダ名
 * @param buff ヘッダ値格納用バッファ
 * @param bufflen バッファ長
 * @return 取得した値。エラー発生時はNULL
 */
char *http_res_hval_buff(HTTPRes_t *res, char *name, char *buff, int bufflen);

/**
 * レスポンスヘッダ値取得(内部バッファ利用)
 * 取得された値はレスポンスオブジェクトの内部バッファで保持しているので、
 * 速やかに別領域にコピーすること。
 * @param res HTTPレスポンスオブジェクト
 * @param name ヘッダ名
 * @return 取得した値。エラー発生時はNULL
 */
char *http_res_header_value(HTTPRes_t *res, char *name);
/**!
 * 生成されたHTTP Response オブジェクトの開放
 */
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