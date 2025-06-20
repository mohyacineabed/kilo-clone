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

  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);   //disable echo, canonical mode, ctrl-v and signals

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
  // Disable echo (ECHO)
  // Disable canonical mode (ICANON), so input is read byte-by-byte
  // Disable Ctrl-V (IEXTEN) — for literal input mode
  // Disable signal keys like Ctrl-C and Ctrl-Z (ISIG)
}

int main() {

    enableRawMode();

    char c;
    while(read(STDIN_FILENO, &c, 1) == 1 && c != 'q') { //read(fd, buffer, count)

        if(iscntrl(c)) {

            printf("%c\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }
    }
    return 0;
}
