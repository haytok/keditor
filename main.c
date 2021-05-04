#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#define CTRL_KEY(value) ((value) & 0x1f)

void enableRauMode();
void disableRauMode();
void die(const char *msg);

struct termios orig_termios;

void enableRauMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        die("tcgetattr");
    }
    atexit(disableRauMode);

    struct termios raw = orig_termios;
    // Ctrl + S, Ctrl + Q を無効化
    // Ctrl + J が 10 を取っているので、Enter と Ctrl + M を 10 から 13 に移行させる。
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    // キャリッジリターンなどの出力処理機能を無効化
    // これを無効化すると、出力が段々になる。
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag &= ~(CS8);
    // エコーバッグを無効化 (その役割を果たす bit を 0 にするための操作。)
    // カノニカルモードを無効化
    // Ctrl + C, Ctrl + Z, Ctrl + Z を無効化
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    // The TCSAFLUSH argument specifies when to apply the change
    raw.c_cc[VMIN] = 0;
    // この値を 0 にすると、とめどなく画面が流れる。値を受け取る周期を設定する。
    raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        die("tcsetattr");
    }
}

void disableRauMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        die("tcsetattr");
    }
}

void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(void) {
    printf("Start ...\n");
    enableRauMode();

    // 無限ループで入力を待ち受けるが、Enter を押すと、処理が終了する。(
    // read() == 0 としている時にこれが生じる。
    // read() は読み込んだ Byte 数を返すので、この条件式だと一文字読み込んでバッファに書き込んだ時点で while が終了する。
    // デフォルトではカノニカルモード (cooked mode) なので、Enter が押されるまでプログラムにキー入力が受け渡されない。
    // この際、標準入力から受け取った値は、バッファに溜められ、Enter が押されると吐き出される。
    // したがって、エディタの機能を実装するためには、非カノニカルモード (raw mode) を実現する必要がある。
    while (true) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) < 0 && errno != EAGAIN) {
            die("read");
            break;
        }
        if (iscntrl(c)) {
            // この処理に流れてきた時に、マルチバイトに対して条件分岐で処理を実装する必要がある。
            // 何も入力されていない時は、この出力がなされる。
            printf("Ctrl %d\r\n", c);
        } else {
            printf("String %d ( %c )\r\n", c, c);
        }
        // printf("Input char is %c.\n", c);
        if (c == 'q' || c == CTRL_KEY('q')) {
            break;
        }
    }
    return EXIT_SUCCESS;
}
