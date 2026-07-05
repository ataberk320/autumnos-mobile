#include <sys/ioctl.h>
#include <linux/kd.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>

void hal_disablecrs() {
	printf("\033[?25l");
	
}

int hal_disablefbcon() {
	hal_disablecrs();
	int fd = open("/dev/tty0", O_RDWR);
	if (fd < 0) return -1;

	int mode;
	ioctl(fd, KDGETMODE, &mode);
	
	int new_mode = (mode == KD_TEXT) ? KD_GRAPHICS : KD_TEXT;
    	int res = ioctl(fd, KDSETMODE, new_mode);

    	close(fd);
    	return res;
}

void hal_fbcondisableecho(int fd) {
        if (fd < 0) return;
        struct termios toptions;
        if (tcgetattr(fd, &toptions) < 0) return;

        toptions.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON | ISIG);
        toptions.c_iflag &= ~(INPCK | ISTRIP | IGNCR | ICRNL | INLCR | IXON | IXOFF);
        toptions.c_oflag &= ~(OPOST);
        toptions.c_cflag &= ~PARENB;
        toptions.c_cflag &= ~CSTOPB;
        toptions.c_cflag &= ~CSIZE;
        toptions.c_cflag |= CS8;

        toptions.c_cc[VMIN] = 1;
        toptions.c_cc[VTIME] = 0;
        tcsetattr(fd, TCSANOW, &toptions);
}
