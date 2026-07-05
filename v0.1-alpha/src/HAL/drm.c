#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm/drm_mode.h>
#include "FramebufferStruct.h"
#include <unistd.h>

static void pfhandler(int fd, unsigned int frame, unsigned int tv_sec, unsigned int tv_usec, void *data) {
    // Kanka, burası boş kalsın. 
    // Kernel buraya girdiği an, o flip için tuttuğu "event queue" yu boşaltır.
    // Senin tek yapman gereken kernel'a "sinyali okudum" demek.
}

int DRMinit(DRMStruct* ctx, const char* path) {
    ctx->fd = open(path, O_RDWR);
    if (ctx->fd < 0) return -1;

    drmModeRes* res = drmModeGetResources(ctx->fd);
    if (!res) {
        close(ctx->fd);
        return -1;
    }

    drmModeConnector* conn = NULL;
    for (int i = 0; i < res->count_connectors; i++) {
        conn = drmModeGetConnector(ctx->fd, res->connectors[i]);
        if (conn && conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) break;
        drmModeFreeConnector(conn);
        conn = NULL;
    }

    if (!conn) {
        drmModeFreeResources(res);
        close(ctx->fd);
        return -1;
    }

    ctx->width = conn->modes[0].hdisplay;
    ctx->height = conn->modes[0].vdisplay;
    ctx->mode = conn->modes[0];
    ctx->crtc_id = res->crtcs[0];

    struct drm_mode_create_dumb creq = { .width = ctx->width, .height = ctx->height, .bpp = 32 };
    if (ioctl(ctx->fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq) < 0) {
        drmModeFreeConnector(conn);
        drmModeFreeResources(res);
        close(ctx->fd);
        return -1;
    }

    ctx->stride = creq.pitch;
    ctx->size = creq.size;
    uint32_t handle = creq.handle;

    if (drmModeAddFB(ctx->fd, ctx->width, ctx->height, 24, 32, ctx->stride, handle, &ctx->fb_id) < 0) {
        drmModeFreeConnector(conn);
        drmModeFreeResources(res);
        close(ctx->fd);
        return -1;
    }

    struct drm_mode_map_dumb mreq = { .handle = handle };
    ioctl(ctx->fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
    ctx->ptr = mmap(NULL, ctx->size, PROT_WRITE, MAP_SHARED, ctx->fd, mreq.offset);

    if (ctx->ptr == MAP_FAILED) {
        drmModeFreeConnector(conn);
        drmModeFreeResources(res);
        close(ctx->fd);
        return -1;
    }

    if (drmModeSetCrtc(ctx->fd, ctx->crtc_id, ctx->fb_id, 0, 0, &conn->connector_id, 1, &ctx->mode) < 0) {
        drmModeFreeConnector(conn);
        drmModeFreeResources(res);
        close(ctx->fd);
        return -1;
    }

    drmModeFreeConnector(conn);
    drmModeFreeResources(res);
    return 0;
}

