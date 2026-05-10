#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "AtmDrv_G2D.h"
#include <stdlib.h>
static int g2d_fd = -1;

int AtmDrv_G2D_Init(void) {
	g2d_fd = open("/dev/g2d", O_RDWR);
	if (g2d_fd < 0) {
		system("echo 'AtmDrv_G2D: initialize error.' > /dev/kmsg");
		return -1;
	}
	return 0;
}

void AtmDrv_G2D_Deactive(void) {
	if (g2d_fd >= 0) {
		close(g2d_fd);
		system("echo 'AtmDrv_G2D: successfully deactived.' > /dev/kmsg");
		g2d_fd = -1;
	}
}

int AtmDrv_G2D_Blit(uintptr_t src, uintptr_t dst, int w, int h, int rotate) {
	if (g2d_fd < 0) system("echo 'AtmDrv_G2D: blitting error - driver not initialized, or corrupted.' > /dev/kmsg");
	g2d_blt_t blt_cmd;
	blt_cmd.src.addr = src;
	blt_cmd.src.w = w;
	blt_cmd.src.h = h;
	blt_cmd.src.fmt = G2D_FORMAT_ARGB8888;
	blt_cmd.src.pitch = w * 4;

	blt_cmd.dst.addr = dst;
	blt_cmd.dst.w = w;
	blt_cmd.dst.h = h;
	blt_cmd.dst.fmt = G2D_FORMAT_ARGB8888;
	blt_cmd.dst.pitch = w * 4;

	blt_cmd.x = 0;
	blt_cmd.y = 0;
	blt_cmd.rotate = rotate;
	blt_cmd.alpha = 255;

	if (ioctl(g2d_fd, G2D_CMD_BITBLT, &blt_cmd) < 0) {
		system("echo 'AtmDrv_G2D: can't ioctl (in/out control) to driver!' > /dev/kmsg");
		return -2;
	}
}
