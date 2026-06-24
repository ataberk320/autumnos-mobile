#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <stdint.h>
#include "FramebufferStruct.h"

void hal_fbrefresh(FramebufferStruct* fb) {
        if (ioctl(fb->fd, FBIO_WAITFORVSYNC, 0) < 0) {
                usleep(16666);
        }
}

void hal_fbreset(int fd, int mode) {
        ioctl(fd, FBIOBLANK, mode);
	if (mode != FB_BLANK_POWERDOWN && mode != FB_BLANK_UNBLANK) {
		printf("Invalid Framebuffer reset mode!");
		return;
	}
}

int hal_fbinit(FramebufferStruct* ctx) {
        ctx->fd = open("/dev/fb0", O_RDWR);
        if (ctx->fd < 0) return -1;
        struct fb_var_screeninfo vinfo;
        struct fb_fix_screeninfo finfo;
        ioctl(ctx->fd, FBIOGET_VSCREENINFO, &vinfo);
        ioctl(ctx->fd, FBIOGET_FSCREENINFO, &finfo);
        ctx->width = vinfo.xres;
        ctx->height = vinfo.yres;
        ctx->stride = finfo.line_length / 4;
	ctx->size = finfo.smem_len;
        ctx->ptr = (uint32_t*)mmap(0, ctx->size, PROT_WRITE, MAP_SHARED, ctx->fd, 0);
        return (ctx->ptr == MAP_FAILED) ? -1 : 0;
}

void hal_fbexit(int fd, void* ptr, size_t size) {
    	munmap(ptr, size);
    	close(fd);
}
