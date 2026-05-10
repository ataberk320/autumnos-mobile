#include "AutumnIO.h"
#include <stdint.h>
#include "AutumnSyscall.h"
void AutumnIO_Write(int fd, const void *buf, uint32_t len) {
	AutumnSys_Syscall(SYS_WRITE, fd, (long)buf, len, 0, 0, 0);
}

int AutumnIO_Read(int fd, void *buf, uint32_t len) {
	return (int)AutumnSys_Syscall(SYS_READ, fd, (long)buf, len, 0, 0, 0);
}

int AutumnIO_Open(const char *path, int flags) {
	return (int)AutumnSys_Syscall(SYS_OPENAT, -100, (long)path, flags, 0, 0, 0);
}

int AutumnIO_Close(int fd) {
	AutumnSys_Syscall(SYS_CLOSE, fd, 0, 0, 0, 0, 0);
}

