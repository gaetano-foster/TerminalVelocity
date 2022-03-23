#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

int SCR_W, SCR_H, SCR_S;
struct termios orig_termios;

void reset_terminal_mode() {
	tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode() {
	struct termios new_termios;

	/* two copies - one for now, one for later */
	tcgetattr(0, &orig_termios);
	memcpy(&new_termios, &orig_termios, sizeof(new_termios));

	/* register cleanup handler, and set new terminal mode */
	atexit(reset_terminal_mode);
	cfmakeraw(&new_termios);
	tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit() {
	struct timeval tv = { 0L, 0L };
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(0, &fds);
	return select(1, &fds, NULL, NULL, &tv) > 0;
}

int getch() {
	int r;
	unsigned char c;
	if ((r = read(0, &c, sizeof(c))) < 0) {
		return r;
	}
	else {
		return c;
	}
}

void bswap(char **buf1, char **buf2) {
	char *temp = *buf1;
	*buf1 = *buf2;
	*buf2 = temp;
}

void gotoxy(int x, int y) {
	printf("%c[%d;%df", 0x1B, y, x);
}

void draw(char *b, int x, int y, char c) {
	b[y*SCR_W+x] = c;
}

void draw_rect(char *b, int x, int y, int w, int h, char c) {
	int j, i;

	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			draw(b, j + x, i + y, c);
		}
	}
}

void* process_input(void *vargp) {
	while (1) {
		if (getch() == 'p') {
			exit(0);
		}
	}
	(void)getch(); /* consume character and terminate program */
}

int main() {
	int x, y;
	float px, py;
	struct winsize w;
	pthread_t iothread;
	char *back, *front;

	set_conio_terminal_mode();	
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	SCR_W = w.ws_col;
	SCR_H = w.ws_row;
	SCR_S = SCR_W * SCR_H;

	back = malloc(SCR_S);
	front = malloc(SCR_S);
	memset(back, ' ', SCR_S);
	memset(front, ' ', SCR_S);
	pthread_create(&iothread, NULL, process_input, NULL);

	while (!kbhit()) {
		/* operate on back */
		memset(back, ' ', SCR_S);
		draw_rect(back, 10, 5, 10, 5, 'o');

		/* draw front */
		for (y = 0; y < SCR_H; y++) {
			for (x = 0; x < SCR_W; x++) {
				gotoxy(x, y);
				putchar(front[y*SCR_W+x]);
			}
		}
		bswap(&back, &front);
	}
		
	pthread_join(iothread, NULL);
	free(back);
	free(front);
	return 0;
}
