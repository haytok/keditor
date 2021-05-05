# 概要
- kilo の解説記事を読む際に、検証したことやメモなどを書いていく。

## メモ 1
- 何も挿入されていない行の先頭にある ~ を削除するために、空文字を挿入する。

### プログラム

```
void editorInsertChar(int c) {
  if (E.cy == E.numrows) {
    editorAppendRow("", 0);
  }
  editorRowInsertChar(&E.row[E.cy], E.cx, c);
  E.cx++;
}
```

### 参考

```text
If E.cy == E.numrows, then the cursor is on the tilde line after the end of the file, so we need to append a new row to the file before inserting a character there. After inserting a character, we move the cursor forward so that the next character the user inserts will go after the character just inserted.
```

## メモ 2
- `editorRowInsertChar` と `editorInsertChar` の責務の切り分けは以下のようになる。
    - `editorInsertChar` は、`erow` オブジェクトの詳細に関心が無い設計になっている。(editor operations)
    - `editorRowInsertChar` は、カーソルの位置に関心が無い設計になっている。(row operations)

- `editorDelChar` と `editorRowDelChar` の関係性も同様である。

### プログラム

```c
void editorRowInsertChar(erow *row, int at, int c) {
  if (at < 0 || at > row->size) at = row->size;
  row->chars = realloc(row->chars, row->size + 2);
  memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
  row->size++;
  row->chars[at] = c;
  editorUpdateRow(row);
}

void editorInsertChar(int c) {
  if (E.cy == E.numrows) {
    editorAppendRow("", 0);
  }
  editorRowInsertChar(&E.row[E.cy], E.cx, c);
  E.cx++;
}
```

### メモ 3
- 知らなかったキー入力と挙動の関係性を以下にまとめる。
- Enter と `\r` は同じ意味。

|  ショートカット  |  説明  |
| :----: | :----: |
| backspace / Ctrl + h  |  カーソル位置の前の文字を削除  |
| Ctrl + l |  改ページを行う  |
| Ctrl + d |  bash が終了する  |

### メモ 4
- `open` の引数に `O_TRUNC` を引き渡すパターンと `ftruncate` を呼び出すケースを比較してる。

- `man 2 ftruncate`

```bash
DESCRIPTION
    The truncate() and ftruncate() functions cause the regular file named by path or referenced by fd to be truncated to a size of precisely length bytes.
```

- `man 2 open`

```bash
DESCRIPTION
    O_TRUNC
        If  the  file  already exists and is a regular file and the access mode allows writing (i.e., is O_RDWR or O_WRONLY) it will be truncated to length 0.  If the file is a FIFO or terminal device file, the O_TRUNC flag is ignored.  Otherwise, the effect of O_TRUNC is unspecified.
```

### メモ 5
- 以下の関数に対する解説の意味がわからんかったが、@q2ven からのリプをキッカケに理解できた。
- ファイルを上書きする方法は次の 2 通りある。
  1. `fd = open(*filename, O_TRUNC)` の処理で元あるファイルを空にしてから、 buffer に溜められたエディタ全体の値を書き込む方法。
  2. 普通に `fd = open()` を呼び出して、`ftrunc(fd, buffer の長さ)` で長さを調節する方法。
- 初めは、1 と 2 の方法のどちらを取っても問題ないと思ってたけど、`write()` の処理が失敗した時の安全性が違ってくる。
- 1 の方法だと、`write()` の処理が失敗すると、ファイルに書き込まれたデータがあった時は、それが全て吹っ飛んでしまう。
- 一方で、2 の方法だと、`write()` の処理が失敗しても、全てデータが吹っ飛ぶ可能性が少しは低くなるので、安全性が高まる。
- ただし、2 の方法でも `ftruncate()` の第二引数に 0 や元のバッファよりも小さい値を入れると、元のデータが欠損してしまうので、より細かいチューニングが必要になる。

```bash
void editorSave() {
  if (E.filename == NULL) return;
  int len;
  char *buf = editorRowsToString(&len);
  int fd = open(E.filename, O_RDWR | O_CREAT, 0644);
  ftruncate(fd, len);
  write(fd, buf, len);
  close(fd);
  free(buf);
}
```

```text
The normal way to overwrite a file is to pass the O_TRUNC flag to open(), which truncates the file completely, making it an empty file, before writing the new data into it. By truncating the file ourselves to the same length as the data we are planning to write into it, we are making the whole overwriting operation a little bit safer in case the ftruncate() call succeeds but the write() call fails. In that case, the file would still contain most of the data it had before. But if the file was truncated completely by the open() call and then the write() failed, you’d end up with all of your data lost.

More advanced editors will write to a new, temporary file, and then rename that file to the actual file the user wants to overwrite, and they’ll carefully check for errors through the whole process.

---

ファイルを上書きする通常の方法は、open()にO_TRUNCフラグを渡し、ファイルを完全に切り詰めて空のファイルにしてから、新しいデータを書き込むことです。書き込むデータと同じ長さにファイルを切り詰めることで、ftruncate()の呼び出しが成功してもwrite()の呼び出しが失敗した場合に、上書き操作全体を少しだけ安全にすることができます。この場合、ファイルにはそれまでのデータのほとんどが残っています。しかし、open()でファイルが完全に切り捨てられ、その後write()が失敗した場合は、すべてのデータが失われてしまいます。
```

```twitter
O_TRUNC は open()+truncate(fd, 0) やけど truncate() は指定した文字数に切り詰めたり伸ばしたりも出来る
```

## メモ 6
- すでにあるテキストファイル (今回は verify/input.txt) を `fd = open("input.txt", O_RDWR | O_TRUNC, 0644)` で開けて、`read()` すると、どんな挙動をするかを検証した。
- 結果は、何も表示されない。

### プログラム
- `verify/verify_0.c`

## メモ 7
- 各章で触れられている内容を手短にまとめる。

### フロー
- 標準入力から文字列を受け取る
- カノニカルモードであることの検証 (q コマンドの追加)
- エコーバッグの無効化
- カノニカルモードの無効化
- Ctrl + C, Ctrl + Z を無効化
- Ctrl + S, Ctrl + Q を無効化
- 2 章では、Ctrl + alphabetic Keys をマッピングした。


## Window Size を取得するための関数
- `ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)` のように呼び出すと、`ws` オブジェクトに幅と高さの値が格納される。

- また、以下の文が印象的だった。Python と違って C 言語ならではの書き方である。
- This is a common approach to having functions return multiple values in C. It also allows you to use the return value to indicate success or failure.

```c
int getWindowSize(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}
```

### 参考
- [ターミナルの幅と高さに関するメモ書き](https://zenn.dev/kusaremkn/articles/abdbd2f38c3d98b145eb)

## 課題
- Window Size が変更された時に、screenrows と screencols を変更する処理の実装


## if 文のデバッグ方法
- `if (1 | hoge) {}` でこの if 文の中を必ず通るようにできるテクニック。

## sscanf で文字列を展開できる

## 入力キーとプログラムが受け取った値の対応関係

|  入力キー  |  プログラムが受け取った値  |
| :----: | :----: |
| Page Up  |  \x1b[5~  |
| Page Down |  \x1b[6~  |
| Home |  \x1b[H~  |
| End |  \x1b[F~  |
| Delete |  \x1b[3~  |
