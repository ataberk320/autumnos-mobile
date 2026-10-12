#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include "../../include/HAL/io.h"

int sys_scanv4l2dev(const char *name) {
	if (!name)
		return -1;

	char dev_path[32];

	for (int i = 0; i < 10; i++) {
        snprintf(dev_path, sizeof(dev_path), "/dev/video%d", i);

        int fd = _AutumnSys_ioOpen(
            dev_path, O_RDWR | O_NONBLOCK
        );

        if (fd < 0)
            continue;

        struct v4l2_capability cap = {0};

        if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == 0 &&
            (strstr((char *)cap.driver, name) ||
             strstr((char *)cap.card, name))) {
            return fd;
        }

        _AutumnSys_ioClose(fd);
    }

    return -1;
}
