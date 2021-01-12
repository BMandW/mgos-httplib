
#include "linechar.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    } else {
        buff[len] = '\0';
    }
    return len + 1;
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
    char *data = (char *)malloc(l * sizeof(char) + 1);
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