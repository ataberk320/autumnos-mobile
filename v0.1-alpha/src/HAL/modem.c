#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "autumn_ioctl.h"
#include "io.h"
//Implementing AutumnOS local driver ioctl commands instead of UART HAL table.

static int modem_fd = -1;

int hal_modemopen() {
	if (modem_fd < 0) {
		modem_fd = _AutumnSys_ioOpen("/dev/simmod0", 0x2, 0);
	}
	return modem_fd;
}

void hal_modemcls() {
	if (modem_fd >= 0) {
		_AutumnSys_ioClose(modem_fd);
		modem_fd = -1;
	}
}

int hal_modemok() {
	int status = 0;
	if (hal_modemopen() < 0) return 0;
	ioctl(modem_fd, IOCMODGETSTAT, &status);
	return status;
}

void hal_modeminit() {
	if (hal_modemopen() < 0) return;
	ioctl(modem_fd, IOCMODSCAN);
}

void hal_sendsms(const char* num, const char* msg) {
    	if (hal_modemopen() < 0) return;
    
    	ioctl(modem_fd, IOCMODSNDCMD, "AT+CMGF=1\r\n");
    	usleep(100000);
    
    	char cmd[128];
    	snprintf(cmd, sizeof(cmd), "AT+CMGS=\"%s\"\r\n", num);
    	ioctl(modem_fd, IOCMODSNDCMD, cmd);
    	usleep(100000);
    
    	ioctl(modem_fd, IOCMODSNDCMD, (char*)msg);
    
    	char ctrl_z[2] = {0x1A, 0x00};
    	ioctl(modem_fd, IOCMODSNDCMD, ctrl_z);
}

void hal_modemcall(const char* num) {
    	if (hal_modemopen() < 0) return;
    	char cmd[64];
    	snprintf(cmd, sizeof(cmd), "ATD%s\r\n", num);
    	ioctl(modem_fd, IOCMODSNDCMD, cmd);
}

void hal_callanswer() {
    	if (hal_modemopen() < 0) return;
    	ioctl(modem_fd, IOCMODSNDCMD, "ATA\r\n");
}

void hal_callreject() {
    	if (hal_modemopen() < 0) return;
    	ioctl(modem_fd, IOCMODSNDCMD, "ATH\r\n");
}

int hal_modemcheckpin() {
    	if (hal_modemopen() < 0) return 0;
    	ioctl(modem_fd, IOCMODSNDCMD, "AT+CPIN?\r\n");
    	usleep(100000);
    
    	char buf[128] = {0};
    	ioctl(modem_fd, IOCMODRDRESP, buf);
    	return (strstr(buf, "READY") != NULL);
}
