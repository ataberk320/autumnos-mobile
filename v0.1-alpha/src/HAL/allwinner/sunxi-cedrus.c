#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

int hal_cdrioctl(void) {
	int fd = sys_scanv4l2dev("cedrus");
	if (fd < 0) {
		printf("No Allwinner Cedrus VPU device found!\n");
		return -1;
	}

	struct v4l2_requestbuffers reqbuf;
	memset(&reqbuf, 0, sizeof(reqbuf));
	reqbuf.count = 2;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	reqbuf.memory = V4L2_MEMORY_MMAP;

	if (ioctl(fd, VIDIOC_REQBUFS, &reqbuf) < 0) {
        	perror("Cedrus - Failed ioctl request");
        	return -1;
    	}

    return 0;
}
