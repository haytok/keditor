# keditor

## 概要
- [kilo](https://github.com/antirez/kilo) と [Build Your Own Text Editor](https://viewsourcecode.org/snaptoken/kilo/index.html) を参考に C 言語を用いて、自作エディタを実装する。
  - [Setup](https://viewsourcecode.org/snaptoken/kilo/01.setup.html)
  - [Entering raw mode](https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html)
  - [Raw input and output](https://viewsourcecode.org/snaptoken/kilo/03.rawInputAndOutput.html)
  - [A text viewer](https://viewsourcecode.org/snaptoken/kilo/04.aTextViewer.html)
  - [A text editor](https://viewsourcecode.org/snaptoken/kilo/05.aTextEditor.html)

## コマンド

- ビルドと実行 (引数にファイルがあり)

```bash
make
```

- ビルドと実行 (引数にファイルがなし)

```bash
make no
```

- ビルド

```bash
make build
```

- デバッグ
  - 入力キーとプログラムが受け取った値を確認できる。

```bash
make debug
```
