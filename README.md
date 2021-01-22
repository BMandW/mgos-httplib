# HTTP client library for Mongoose OS app

## Overview

HTTP送受信をするためのHTTP Clientライブラリ。
ヘッダの設定や、レスポンスの取得などを `mg_connect_http` よりも簡単に使いやすくしたもの。


1. http_create_reqでリクエストオブジェクトを作成し
2. http_sendでリクエストを送信。
3. 取得したレスポンスオブジェクトからデータを得る(res->body)
4. http_res_freeでレスポンスオブジェクトを開放する

[C言語実装のexample](https://github.com/BMandW/example-clang-mgos-httplib)

[mJS実装のexample](https://github.com/BMandW/example-mjs-mgos-httplib)

## build

依存ライブラリのヘッダなどを取得する必要があるため、一度ローカルでビルドをして deps ディレクトリを取得する必要がある。

$ mos build --local

vscode の拡張では include パスの構築がリアルタイムで反映されないときがある。
`#include <mgos.h>` などが取得できない場合は、一度 vscode を再起動すると良い。
