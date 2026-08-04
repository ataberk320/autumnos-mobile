#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
//FIXME: Improved memory mechanism and adapted to new V4L2 HAL function

int hal_cdrioctl(void) {
	char dev_path[64];
	if (sys_scanv4l2dev("cedrus", dev_path, sizeof(dev_path)) < 0) {
		printf("No Allwinner Cedrus VPU device found!\n");
		return -1;
	}

	int fd = open(dev_path, O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		perror("Cedrus - Failed to open device");
		return -1;
	}

	struct v4l2_requestbuffers reqbuf;
	memset(&reqbuf, 0, sizeof(reqbuf));
	reqbuf.count = 2;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	reqbuf.memory = V4L2_MEMORY_MMAP;

	if (ioctl(fd, VIDIOC_REQBUFS, &reqbuf) < 0) {
        	perror("Cedrus - Failed ioctl request");
        	close(fd);
        	return -1;
    	}

    close(fd);
    return 0;
}
