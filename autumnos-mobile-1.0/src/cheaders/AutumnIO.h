#ifndef IO_H
#define IO_H

#include <stdint.h>

#define SYS_OPENAT 56
#define SYS_READ   63
#define SYS_WRITE  64
#define SYS_CLOSE  57

void AutumnIO_Write(int fd, const void *buf, uint32_t len);

int AutumnIO_Read(int fd, void *buf, uint32_t len);

int AutumnIO_Open(const char *path, int flags);

int AutumnIO_Close(int fd);

#endif
