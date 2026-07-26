#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

int sys_scanv4l2dev(const char *name) {
	char dev_path[32];
	int fd = -1;

	for (int i = 0; i < 10; i++) {
		snprintf(dev_path, sizeof(dev_path), "/dev/video%d", i);
		fd = open(dev_path, O_RDWR | O_NONBLOCK);
	
		if (fd < 0) continue;
	
		struct v4l2_capability cap;
		if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == 0) {
			if (strstr((char*)cap.driver, target_name) || strstr((char*)cap.card, target_name)) {
                		return fd; // Bulundu, dosya tanımlayıcısını döndür
            		}
        	}
        	close(fd);
    }
    return -1;
}
