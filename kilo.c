#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);

  struct termios raw = orig_termios;

  raw.c_iflag &= ~(ICRNL | IXON);
  // Disable Ctrl-S and Ctrl-Q (software flow control)
  // Disable carriage return to newline translation (fixes Ctrl-M)
  //
  raw.c_oflag &= ~(OPOST);  // disable translating '\n' to '\r\n' in the output

  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  // Disable echo (ECHO)
  // Disable canonical mode (ICANON), so input is read byte-by-byte
  // Disable Ctrl-V (IEXTEN) — for literal input mode
  // Disable signal keys like Ctrl-C and Ctrl-Z (ISIG)

  raw.c_cc[VMIN] = 0;   //set min bytes needed before read() can return
  raw.c_cc[VTIME] = 1;  //set max time to wait before read() returns (ms)

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

}

int main() {

    enableRawMode();

    while (1) {
        char c = '\0';
        read(STDIN_FILENO, &c, 1);  //read(fd, buffer, count)
        if (iscntrl(c)) {
            printf("%c\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') break;
    }

    return 0;
}
