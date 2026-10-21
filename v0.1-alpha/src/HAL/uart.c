#define _GNU_SOURCE
#include <stddef.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "../../include/SymbolTable/table.h"
#include "../../include/HAL/HAL_Symb.h"
struct termios2 {
    unsigned int c_iflag;
    unsigned int c_oflag;
    unsigned int c_cflag;
    unsigned int c_lflag;
    unsigned char c_cc[65];
    unsigned int c_ispeed;
    unsigned int c_ospeed;
};

#ifndef TCGETS2
#define TCGETS2 _IOR('T', 0x2A, struct termios2)
#endif

#ifndef TCSETS2
#define TCSETS2 _IOW('T', 0x2B, struct termios2)
#endif

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

typedef struct {
    hal_dev_t base;
    int (*Connect)(const char* dev);
    void (*Send)(const char *cmd);
    int (*Read)(char* buf, int max_len);
} uart_dev_t;

static int uart_bind_dev(const struct hal_mod_t* module, const char* name, struct hal_dev_t** device) {
    uart_dev_t* dev = malloc(sizeof(uart_dev_t));
    if (!dev) return -1;
    
    dev->base.common = (hal_mod_t*)module;
    dev->Connect = hal_uartconnect;
    dev->Send = hal_uartsend;
    dev->Read = hal_uartrd;
    
    *device = (hal_dev_t*)dev;
    return 0;
}

hal_mod_t HAL_MODULE_INFO_SYM = {
    .id = "uart",
    .version = 1,
    .open = uart_bind_dev
};
