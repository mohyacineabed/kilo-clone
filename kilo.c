#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <errno.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)    //mimic the CTRL key

/*** data ***/

struct editorConfig {
    int screenrows;
    int screencols;
    struct termios orig_termios;
};
struct editorConfig E;

/*** terminal ***/

void die(const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) {
        die("tcsetattr");
    }
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) {
      die("tcgetattr");
  }
  atexit(disableRawMode);

  struct termios raw = E.orig_termios;

  raw.c_iflag &= ~(ICRNL | IXON);
  // Disable carriage return to newline translation (fixes Ctrl-M)
  // Disable Ctrl-S and Ctrl-Q (software flow control)

  raw.c_oflag &= ~(OPOST);  // disable translating '\n' to '\r\n' in the output

  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  // Disable echo (ECHO)
  // Disable canonical mode (ICANON), so input is read byte-by-byte
  // Disable Ctrl-V (IEXTEN) — for literal input mode
  // Disable signal keys like Ctrl-C and Ctrl-Z (ISIG)

  raw.c_cc[VMIN] = 0;   //set min bytes needed before read() can return
  raw.c_cc[VTIME] = 1;  //set max time to wait before read() returns (ms)

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
      die("tcsetattr");
  }

}

char editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}

int getWindowSize(int *rows, int* cols) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    } else {
        *rows = ws.ws_row;
        *cols = ws.ws_col;
        return 0;
    }
}

/*** output ***/

void editorDrawRows() {
    int y;
    for (y = 0; y < E.screenrows; y++) {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editorRefreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    //write esc char "\x1b" followed by '[' and "2J" to clear the screen
    write(STDOUT_FILENO, "\x1b[H", 3);
    //reposition the cursor to top of the screen ("<esc>[Row ; Col H")

    editorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/

void editorProcessKeypress() {
    char c = editorReadKey();

    switch(c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
    }
}

/*** init ***/

void initEditor() {
    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

int main() {

    enableRawMode();
    initEditor();

    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }

    return 0;
}
