#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <asm/termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "table.h"

extern UART_HAL* uart;

int hal_modemok() {
	char buf[32];
	uart->Read(buf, 31);
	return (strstr(buf, "OK") != NULL);
}

void hal_modeminit() {
	uart->Send("AT\r\n");
	sleep(1);
	uart->Send("AT+CFUN=1\r\n");
}

void hal_sendsms(const char* num, const char* msg) {
	uart->Send("AT+CMGF=1\r\n"); //Text mode
	sleep(1);
	char cmd[64];
	sprintf(cmd, "AT+CMGS=\"%s\"\r\n", num);
	uart->Send(cmd);
	sleep(1);
	uart->Send(msg);
	uart->Send("\x1A"); //CTRL+Z
}

void hal_modemcall(const char* num) {
	char cmd[32];
	sprintf(cmd, "ATD%s\r\n", num);
	uart->Send(cmd);
}

void hal_callanswer() {
	uart->Send("ATA\r\n");
}

void hal_callreject() {
	uart->Send("ATH\r\n");
}

int hal_modemcheckpin() {
    uart->Send("AT+CPIN?\r\n");
    char buf[64];
    uart->Read(buf, 63);
    return (strstr(buf, "READY") != NULL);
}
