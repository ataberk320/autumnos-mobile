#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <termios.h>
#include "AutumnExternModem.h"
int AutumnModem_Int_Init(AutumnSIM_t *modem) {
	modem->fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (modem->ctrl_fd < 0) {
		printf("Cannot init external SIM card");
		return -1;
	}
	struct termios tty;
	tcgetattr(modem->ctrl_fd, &tty);
	cfsetospeed(&tty, B115200);
	cfsetispeed(&tty, B115200);
	tty.c_cflag |= (CLOCAL | CREAD | CS8);
	tty.c_cflag &= ~(PARENB | CSTOPB);
	tcsetattr(modem->ctrl_fd, TCSANOW, &tty);

	modem->data_fd = open("/dev/cdc-wdm0", O_RDWR | O_NONBLOCK);
	printf("External SIM card hardware is ready");
	return 0;
}


int AutumnModem_Check(AutumnSIM_t *modem) {
	char buffer[64];
	write(modem->ctrl_fd, "AT+CPIN?\r\n", 10);
	usleep(100000);
	int len = read(modem->ctrl_fd, buffer, sizeof(buffer));
	if (len > 0 && strstr(buffer, "READY")) {
		return 1;
	}
	return 0;
}

void AutumnModem_SignalINF(AutumnSIM_t *modem) {
	char buffer[128];
	write(modem->ctrl_fd, "AT+CSQ;+COPS?\r\n", 15);
	usleep(100000);
	read(modem->ctrl_fd, buffer, sizeof(buffer));
}

void AutumnModem_Call(AutumnSIM_t *modem, const char *number) {
	char cmd[64];
	sprintf(cmd, "ATD%s;\r\n", number);
    	write(modem->ctrl_fd, cmd, strlen(cmd));
}

void AutumnModem_CloseRsc(AutumnSIM_t *modem) {
    if (modem->ctrl_fd >= 0) close(modem->ctrl_fd);
    if (modem->data_fd >= 0) close(modem->data_fd);
}
