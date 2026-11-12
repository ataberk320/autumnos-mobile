#include "../include/HAL/io.h"
#include <unistd.h>

int _Sysrq_Enable(void) {
	int fd = _AutumnSys_ioOpen("/proc/sys/kernel/sysrq", A_WRONLY, 0);
	if (fd < 0)
		return -1;

	int result = (_AutumnSys_ioWrite(fd, "1", 1) == 1) ? 0 : -1;
	_AutumnSys_ioClose(fd);

	return result;
}

int _Sysrq_Trigger(char cmd) {
	int fd = _AutumnSys_ioOpen("/proc/sysrq-trigger", A_WRONLY, 0);
	if (fd < 0)
		return -1;

	int result = (_AutumnSys_ioWrite(fd, &cmd, 1) == 1) ? 0 : -1;
    	_AutumnSys_ioClose(fd);

    	return result;
}
