#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "AtmDrv_G2D.h"
#include <stdlib.h>
#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>
#include <sys/mman.h>
#include <sys/stat.h>
static int g2d_fd = -1;
static int drms_fd = -1;
int AutumnAPI_GraphDriver_G2D_Init(void) {
	g2d_fd = open("/dev/g2d", O_RDWR);
	if (g2d_fd < 0) {
		system("echo 'AtmDrv_G2D: initialize error.' > /dev/kmsg");
		return -1;
	}
	return 0;
}

void AutumnAPI_GraphDriver_G2D_Deactive(void) {
	if (g2d_fd >= 0) {
		close(g2d_fd);
		system("echo 'AtmDrv_G2D: successfully deactived.' > /dev/kmsg");
		g2d_fd = -1;
	}
}

int AutumnAPI_GraphDriver_G2D_Blit(uintptr_t src, uintptr_t dst, int w, int h, int rotate) {
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
	return 0;
}

void AutumnAPI_GraphDriver_DRM_Blit(uintptr_t src, uint32_t width, uint32_t height) {
        struct drm_mode_create_dumb creq;
        struct drm_mode_map_dumb mreq;
        uint32_t *map_address = NULL;

        creq.height = height;
        creq.width = width;
        creq.bpp = 32;
        creq.flags = 0;
        creq.handle = 0;
        creq.pitch = 0;
        creq.size = 0;

        if (ioctl(drms_fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq) < 0) {
                printf("[DRM-Log]: DRM_IOCTL_MODE_CREATE_DUMB failed!");
                return;
        }

        mreq.handle = creq.handle;
        mreq.pad = 0;
        mreq.offset = 0;

        if (ioctl(drms_fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq) < 0) {
                printf("[DRM-Log]: In/Out control failed!");
                return;
        }

        map_address = (uint32_t*)mmap(0, creq.size, PROT_READ | PROT_WRITE, MAP_SHARED, drms_fd, mreq.offset);
        if (map_address == MAP_FAILED) {
                printf("[DRM-Log]: mmap failed!");
                return;
        }

        uint32_t *source = (uint32_t*)src;
        int total_pixels = width * height;
        for (int i = 0; i < total_pixels; i++) {
                map_address[i] = source[i];
        }

        munmap(map_address, creq.size);
        struct drm_mode_destroy_dumb dreq = { creq.handle };
        ioctl(drms_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
}
