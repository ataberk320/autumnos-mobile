#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <stdint.h>
extern "C" {
	#include "cheaders/atmhal.h"
}

#define AUTUMN_SOCKET "/tmp/autumn_conf/AutumnCore0.sock"

int SCREEN_W = 480;
int SCREEN_H = 640;
uint32_t UI_LEN = 0;

int gfx_mode = 3;
int drm_fd = -1;
uint32_t crtc_id = 0, plane_app_id = 0, fb_id = 0, handle = 0;

uint32_t* ui_shm_ptr = NULL;
uint32_t* fb_ptr = NULL;

void init_scrcard() {
	int fb_fd = open("/dev/fb0", O_RDWR);
	if (fb_fd >= 0) {
		struct fb_var_screeninfo vinfo;
        	if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) == 0) {
            		SCREEN_W = vinfo.xres;
            		SCREEN_H = vinfo.yres;
        	}
        	fb_ptr = (uint32_t*)mmap(0, SCREEN_W * SCREEN_H * 4, PROT_WRITE, MAP_SHARED, fb_fd, 0);
        	close(fb_fd);
    	}
	UI_LEN = SCREEN_W * SCREEN_H * 4;
	int ui_fd = shm_open("/autumn_ui", O_RDWR | O_CREAT, 0666);
    ftruncate(ui_fd, UI_LEN);
    ui_shm_ptr = (uint32_t*)mmap(0, UI_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, ui_fd, 0);

    drm_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (drm_fd >= 0) {
        drmModeRes *res = drmModeGetResources(drm_fd);
        if (res) {
            crtc_id = res->crtcs[0];
            drmModePlaneRes *pr = drmModeGetPlaneResources(drm_fd);
            if (pr && pr->count_planes >= 2) plane_app_id = pr->planes[1];
            drmModeFreeResources(res);
        }
    }

    char cpu_vendor[64] = {0};
    cpuv(cpu_vendor, sizeof(cpu_vendor));

    if (access("/dev/g2d", F_OK) == 0 && ui_shm_ptr && fb_ptr) {
        gfx_mode = 1; // Allwinner G2D
    }
    else if (drm_fd >= 0 && ui_shm_ptr && plane_app_id > 0) {
        gfx_mode = 2; // DRM/KMS Plane
        struct drm_mode_create_dumb creq = {(uint32_t)SCREEN_H, (uint32_t)SCREEN_W, 32, 0, 0, 0, 0, 0};
        ioctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
        handle = creq.handle;
        uint32_t bo_handles[4] = {handle, 0, 0, 0}, pitches[4] = {(uint32_t)SCREEN_W * 4, 0, 0, 0}, offsets[4] = {0};
        drmModeAddFB2(drm_fd, SCREEN_W, SCREEN_H, DRM_FORMAT_ARGB8888, bo_handles, pitches, offsets, &fb_id, 0);
    }
    else if (strcasestr(cpu_vendor, "QEMU") || strcasestr(cpu_vendor, "qemu")) {
        gfx_mode = 3; // QEMU Memcpy
    }
}

int main() {
    init_graphics_core();

    int server_fd = atmhal_ipc_init_server(AUTUMN_SOCKET);
    int client_fd = -1;
    char buffer[64];

    if (server_fd < 0) return printf("Socket connecting error!\n"), 1;

    while (1) {
        if (atmhal_ipc_read_cmd(server_fd, &client_fd, buffer, sizeof(buffer)) > 0) {
            if (strncmp(buffer, "render:card", 11) == 0) {
                switch (gfx_mode) {
                    case 1: 
                        atmsys_g2d_blit((uintptr_t)ui_shm_ptr, (uintptr_t)fb_ptr, SCREEN_W, SCREEN_H, 0); 
                        break;
                    case 2: 
                        atmsys_drm_drw((uintptr_t)ui_shm_ptr, SCREEN_W, SCREEN_H);
                        drmModeSetPlane(drm_fd, plane_app_id, crtc_id, fb_id, 0, 0, 0, SCREEN_W, SCREEN_H, 0, 0, SCREEN_W << 16, SCREEN_H << 16);
                        break;
                    case 3: 
                        if (fb_ptr && ui_shm_ptr) memcpy(fb_ptr, ui_shm_ptr, UI_LEN); 
                        break;
                }
            }
        }
        usleep(1000);
    }
    return 0;
}
