# HTTP client library for Mongoose OS app

## Overview

## build

依存ライブラリのヘッダなどを取得する必要があるため、一度ローカルでビルドをして deps ディレクトリを取得する必要がある。

$ mos build --local

vscode の拡張では include パスの構築がリアルタイムで反映されないときがある。
`#include <mgos.h>` などが取得できない場合は、一度 vscode を再起動すると良い。
