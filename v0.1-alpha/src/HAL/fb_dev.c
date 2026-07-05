#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <stdint.h>
#include "FramebufferStruct.h"
#include <xf86drm.h>
#include <string.h>
#include <xf86drmMode.h>
#include <sys/select.h> // FD_ZERO, FD_SET, select()
#include <sys/time.h>   // struct timeval
#include <unistd.h>     // select()

static void pfhandler(int fd, unsigned int frame, unsigned int tv_sec, unsigned int tv_usec, void *data) {
    // Event for page flip for prevent allocate error.
}

uint32_t get_prop_id(int fd, uint32_t conn_id, const char* name) {
    drmModeConnector* conn = drmModeGetConnector(fd, conn_id);
    if (!conn) return 0;
    
    uint32_t prop_id = 0;
    for (int i = 0; i < conn->count_props; i++) {
        drmModePropertyPtr prop = drmModeGetProperty(fd, conn->props[i]);
        if (prop && strcmp(prop->name, name) == 0) {
            prop_id = prop->prop_id;
            drmModeFreeProperty(prop);
            break;
        }
        if (prop) drmModeFreeProperty(prop);
    }
    drmModeFreeConnector(conn);
    return prop_id;
}

void hal_drmpgflip(DRMStruct* ctx) {
	int ret = drmModePageFlip(ctx->fd, ctx->crtc_id, ctx->fb_id, DRM_MODE_PAGE_FLIP_EVENT, NULL);
        if (ret != 0) {
                perror("DRMpgFlip");
        }

        struct timeval tv = { .tv_sec = 0, .tv_usec = 0 };
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(ctx->fd, &fds);

        if (select(ctx->fd + 1, &fds, NULL, NULL, &tv) > 0) {
                drmEventContext ev = {
                        .version = DRM_EVENT_CONTEXT_VERSION,
                        .page_flip_handler = pfhandler
                };
                drmHandleEvent(ctx->fd, &ev);
        }
}

void hal_fbrefresh(DRMStruct* ctx) {
	hal_drmpgflip(ctx);
}

void hal_fbreset(DRMStruct* ctx, int mode) {
	uint32_t dpms_id = get_prop_id(ctx->fd, ctx->connector_id, "DPMS");
    	if (dpms_id != 0) {
        	drmModeConnectorSetProperty(ctx->fd, ctx->connector_id, dpms_id, mode);
    	}
}

int hal_fbinit(DRMStruct* ctx, const char* dev_p) {
	return DRMinit(ctx, dev_p);
}

void hal_fbexit(DRMStruct* ctx) {
	struct drm_mode_destroy_dumb dreq = { .handle = ctx->handle };
    	ioctl(ctx->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
    
    	munmap(ctx->ptr, ctx->size);
    	close(ctx->fd);
}

uint32_t hal_tofbcolor(uint32_t r_off, uint32_t g_off, uint32_t b_off, uint32_t color) {
	uint32_t r = (color >> 16) & 0xFF;
	uint32_t g = (color >> 8)  & 0xFF;
	uint32_t b = color & 0xFF;
	return (r << r_off) | (g << g_off) | (b << b_off);
}
