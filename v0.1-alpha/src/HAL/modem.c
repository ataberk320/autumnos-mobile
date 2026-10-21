#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "../../include/HAL/autumn_ioctl.h"
#include "../../include/HAL/io.h"
#include "../../include/HAL/HAL_Symb.h"

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


typedef struct {
	hal_dev_t base;
	int (*IsOk)(void);
	void(*Init)(void);
	void (*Dial)(const char* num);
	void(*Answer)(void);
	void (*Reject)(void);
	void (*SendSMS)(const char* num, const char* msg);
	int (*CheckPin)(void);
    	void (*Close)(void);
} mod_dev_t;

static int mod_bind_dev(const struct hal_mod_t* mod, const char* name, struct hal_dev_t** device) {
	mod_dev_t* dev = malloc(sizeof(mod_dev_t));
	if (!dev) return -1;

	dev->base.common = (hal_mod_t*)mod;
	dev->IsOk = hal_modemok;
    	dev->Init = hal_modeminit;
    	dev->Dial = hal_modemcall;
    	dev->Answer = hal_callanswer;
    	dev->Reject = hal_callreject;
    	dev->SendSMS = hal_sendsms;
    	dev->CheckPin = hal_modemcheckpin;
    	dev->Close = hal_modemcls;
    
    	*device = (hal_dev_t*)dev;
    	return 0;
}

hal_mod_t HAL_MODULE_INFO_SYM = {
	.id = "modem",
	.version = 1,
	.open = mod_bind_dev
};
