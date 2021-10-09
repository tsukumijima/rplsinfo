
# rplsinfo

## このリポジトリについて

このリポジトリは、2018年8月頃まで Vesti La Giubba (http://saysaysay.net/rplstool/rplsinfo) にて公開されていた、  
rplsinfo v1.5.1 ([Axfc のミラー](http://www.axfc.net/u/3933238)) に同梱されているソースコードのミラーです。  

番組の映像情報・音声情報の出力に対応したこと、ビルド環境を Visual Studio 2019 (VS2019) に更新したこと、rplsinfo.sln（ソリューションファイル）を作成したこと、rplsinfo.txt を現状に合わせて改変したこと、この Readme.md を作成したこと以外はオリジナルのままとなっています。  
このリポジトリは VS2019 に更新されていますが、Release の [v1.5.1](https://github.com/tsukumijima/rplsinfo/releases/tag/v1.5.1) (rplsinfo151.zip) に関してはオリジナルのもの（再配布）です。  
Release の [v1.5.2](https://github.com/tsukumijima/rplsinfo/releases/tag/v1.5.2) (rplsinfo152.zip) は [toisme 氏のフォーク](https://github.com/toisme/rplsinfo/tree/develop) の変更を取り込んだものです。番組の映像情報・音声情報の出力に対応しています。

以下のドキュメントは [http://saysaysay.net/rplstool/rplsinfo](https://web.archive.org/web/20161126173554/http://saysaysay.net/rplstool/rplsinfo) に記載されていた情報を現状に合わせて一部改変し、Markdown 形式に書き換えたものです。  
[rplsinfo.txt](rplsinfo.txt)・[ソースファイルについて.txt](ソースファイルについて.txt) も参照してください。

TS ファイルから番組情報を取得できるおそらく唯一のツールで、[TVRemotePlus](https://github.com/tsukumijima/TVRemotePlus) に組み込んで使わせて頂いています。  
rplsinfo の作者の方に感謝します。

----

rplsinfo は、rpls ファイル, TS ファイルの番組情報をテキスト出力するツールです。

このツールの使用は使用者の自己責任において行って下さい。  
このツールの使用によってもたらされた如何なる結果にも作者は一切責任を負いません。

- [rplsinfo](#rplsinfo)
  - [このリポジトリについて](#このリポジトリについて)
  - [ダウンロード](#ダウンロード)
    - [アーカイブの内容](#アーカイブの内容)
  - [動作環境](#動作環境)
  - [アプリケーションの使用方法](#アプリケーションの使用方法)
    - [出力形式を指定するオプションの一覧](#出力形式を指定するオプションの一覧)
  - [注意点](#注意点)
  - [再配布について](#再配布について)
  - [更新履歴](#更新履歴)

## ダウンロード
[rplsinfo Version 1.5.1 for Windows](https://github.com/tsukumijima/rplsinfo/releases/download/v1.5.1/rplsinfo151.zip)

### アーカイブの内容
* rplsinfo.txt … 説明ファイル
* rplsinfo.exe … アプリケーション本体
* x64 フォルダ … 64bit版アプリケーション本体
* src フォルダ … アプリケーションのソースファイル

## 動作環境
本ツールは主に Windows 10 で動作を確認しています。
開発環境は Visual Studio 2019 です。

## アプリケーションの使用方法
コマンドプロンプトから以下のように実行して下さい。

    rplsinfo.exe 読み込みファイル名 出力ファイル名 [-オプションスイッチ] [enter]

読み込みファイル名には、rpls ファイル、もしくは TS ファイルを指定します。  
指定したファイルから番組情報を取得して、テキスト情報として出力ファイルに出力します。

    rplsinfo.exe 00001.rpls test1.txt -T -cb [enter]
    出力例1：rpls ファイル "00001.rpls" から、番組情報(放送局名, 番組名)を、テキストファイル "test1.txt" にタブ文字区切りで出力する

    rplsinfo.exe 00002.m2ts test2.txt -C -bd [enter]
    出力例2：TS ファイル "00002.m2ts" から、番組情報(番組名, 録画日付)を、テキストファイル" test2.txt" に CSV 形式で出力する

出力ファイル名を省略した場合は、情報をコマンドプロンプト画面に出力します。

    rplsinfo.exe 00003.rpls -T -bi [enter]
    出力例3：rpls ファイル "00003.rpls" から、番組情報(番組名, 番組内容)を、コマンドプロンプトにタブ文字区切りで出力する

出力形式、出力内容はオプションスイッチで指定します。出力形式を指定するスイッチ [-T] [-S] [-C] [-N] [-I] は以下の通りです。

### 出力形式を指定するオプションの一覧

* 指定無し
  - デフォルトで簡易コンマ区切りで出力します。各項目はコンマ文字で区切られます。
  - 各項目中の改行などの制御文字、及びコンマ文字は省略され出力されません。
* -T
  - 各項目をタブ文字区切りで出力します。
  - 各項目中の改行などの制御文字は省略され出力されません。
* -S
  - 各項目を半角スペース文字区切りで出力します。
  - 各項目中の改行などの制御文字は省略され出力されません。
* -C
  - 正式な CSV 形式で出力します。各項目の前後は「"」で括られ、項目中の「"」は「"」でエスケープされます。区切り文字はコンマです。
  - 改行などの制御文字もそのまま出力します。
* -N
  - 各項目を2回の改行で区切って出力します。
  - 改行などの制御文字もそのまま出力します。
* -I
  - 各項目を[項目名]付で出力します。各項目は2回の改行で区切られます。
  - 改行などの制御文字もそのまま出力します。

出力したい番組情報を指定するスイッチ [-fukdtpzaoscnbiegvm] は以下の通りです。情報を出力したい順番に、まとめて指定して下さい。

* -f … ファイル名を出力します。
* -u … フルパスファイル名を出力します。
* -k … ファイルサイズ情報を出力します。
* -d … 録画した日付を YY/MM/DD 形式で出力します。
* -t … 録画開始時刻を HH:MM:SS 形式で出力します。
* -p … 録画期間(録画時間)を HH:MM:SS 形式で出力します。
* -z … タイムゾーン設定値を出力します。TSファイルが元ファイルの場合、内容に関係なくタイムゾーンは 18 になります。
* -a … メーカーIDを出力します。
* -o … メーカー機種コードを出力します。
* -s … 放送種別情報を出力します。
* -c … 放送局名を出力します。
* -n … チャンネル番号を 000ch のように 3桁の数字+ch で出力します。番号が999を超える場合はそのまま出力します。
* -b … 番組名を出力します。
* -i … 番組内容情報を出力します。
* -e … 番組内容詳細情報を出力します。
* -g … 番組ジャンル情報を出力します。
* -v … 番組の映像情報を出力します。
* -m … 番組の音声情報を出力します。

その他のスイッチ一覧です。必要に応じて指定して下さい。

* -y … 番組情報読み込み時に、英数文字に対して文字サイズ指定(NSZ, MSZ)を反映させます。
* -j … 複数の異なる字体が存在する文字(葛, 辻, 祇など)の出力時、その区別に異体字セレクタを使用する。
* -q … ファイル出力時、BOM (バイトオーダーマーク)を付与しない。
* -F … TS ファイルから番組情報を取得する際のファイル位置を指定します。
  - "-F 0" がファイルの先頭、"-F 99" がファイルの終端付近です。
  - デフォルトは "-F 50" 相当です。
* -l … 番組情報パケットの探索リミットを整数値で指定します。
  - 番組情報元TSファイルから番組情報を取得する際、情報の探索は開始位置からファイルの終端まで行います。
  - このため必要な情報を見つけられない場合、大きなファイルでは長時間を要してエラー終了することになります。
  - -l オプションを指定すると、必要な番組情報がまだ見つからない場合でも "指定した数値MByte" 分まで進行した所で探索を終了します。
  - 指定例： -l 50

## 注意点

出力ファイルデータは Unicode 形式 (UTF-16 LE) です。  
出力ファイルとして指定したファイルが既に存在していた場合、そのファイルに追記します。  
この際、既存ファイルのテキストエンコーディング設定に関わらず Unicode 形式で追記しますので注意して下さい。  

シフト JIS 形式で出力を受けたい場合は、出力ファイル名を指定せず、コマンドプロンプト出力としてそれをリダイレクト(>, >>)でファイルに受けて下さい。  
但しこの場合は一部の Unicode 文字が失われます。

    rplsinfo.exe 00001.rpls -T -cb > test3.txt [enter]
    出力例4：rplsファイル "00001.rpls" から、番組情報(放送局名, 番組名)を、テキストファイル "test3.txt" にタブ文字区切り、シフト JIS 形式で出力する

-s オプションによる放送種別情報の出力は、パナ製レコーダ作成 rpls ファイルの場合のみ可能です。

## 再配布について

本ツールの再配布に制限はありませんので、自由に再配布してくださって構いません。  
ただしツールの再配布によってもたらされた如何なる結果にも作者は一切責任を負いません。再配布に際して作者への連絡・許可は必要ありません。

ソースを改変したオリジナルバージョン等を公開される場合にも、作者への連絡･許可は必要ありません。

## 更新履歴

Version | Windows版 更新内容
---- | ----
1.5.2 | 番組の映像情報・音声情報の出力に対応しました。開発環境を Visual Studio 2019 に変更しました。
1.5.1 | ファイルを開く際、FILE_SHARE_READ を指定するようにしました。開発環境を Visual Studio 2017 に変更しました。
1.5 | 文字コード変換ルーチンを更新しました。
1.4 | 文字コード変換ルーチンを更新しました。ファイルサイズ情報出力オプション -k を追加しました。-l, -j, -q オプションを追加しました。
1.3 | 文字コード変換ルーチンの不具合を修正しました。
1.2 | パナ製レコーダで作成したrplsファイルでの放送種別情報出力に対応しました。
1.1 | パナ製レコーダで作成したrplsファイルでのジャンル情報出力に対応しました。
1.0 | 最初のバージョン。
