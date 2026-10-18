#ifndef A_LOG_H
#define A_LOG_H

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "io.h"

#define a_perror(s) do { \
	int err = errno; \
	int fd = _AutumnSys_ioOpen("/dev/kmsg", A_WRONLY, 0); \
	if (fd >= 0) { \
		char buf[512]; \
		int len; \
		if ((s) && *(s)) { \
			len = snprintf(buf, sizeof(buf), "<3>AutumnOS_Error: %s: %s (errno: %d)\n", (s), strerror(err), err); \
		} \
		else { \
			len = snprintf(buf, sizeof(buf), "<3>AutumnOS_Error: %s (errno: %d)\n", strerror(err), err); \
		} \
		_AutumnSys_ioWrite(fd, buf, len); \
		_AutumnSys_ioClose(fd); \
	} \
} while(0)

#define a_printf(fmt, ...) do { \
	int kfd = _AutumnSys_ioOpen("/dev/kmsg", A_WRONLY, 0); \
	if (kfd >= 0) { \
		char kbuf[512]; \
		int klen = snprintf(kbuf, sizeof(kbuf), "<6>AutumnOS_Log: " fmt, ##__VA_ARGS__); \
		_AutumnSys_ioWrite(kfd, kbuf, klen); \
		_AutumnSys_ioClose(kfd); \
	} \
	printf(fmt, ##__VA_ARGS__); \
} while(0)

#endif
