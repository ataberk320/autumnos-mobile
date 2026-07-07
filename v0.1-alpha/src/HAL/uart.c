#include <fcntl.h>
#include <unistd.h>
#include <asm/termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include "table.h"

static int current_fd = -1;

int hal_uartconnect(const char* dev) {
	current_fd = open(dev, O_RDWR | O_NOCTTY);
	if (current_fd < 0) return -1;

	struct termios2 tty;
	ioctl(current_fd, TCGETS2, &tty);

	tty.c_cflag &= ~CBAUD;
	tty.c_cflag |= B9600;

	tty.c_cflag &= ~CSIZE;
    	tty.c_cflag |= CS8;
    	tty.c_cflag &= ~PARENB;
    	tty.c_cflag &= ~CSTOPB;
	
	ioctl(current_fd, TCSETS2, &tty);

	return 0;
}

void hal_uartsend(const char *cmd) {
	write(current_fd, cmd, strlen(cmd));
}


int hal_uartrd(char* buf, int max_len) {
	return read(current_fd, buf, max_len);
}
