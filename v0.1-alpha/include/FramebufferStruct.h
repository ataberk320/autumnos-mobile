#ifndef FB_TYPES_H
#define FB_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <xf86drmMode.h>

typedef struct {
    int fd;
    uint32_t crtc_id;
    uint32_t connector_id;
    uint32_t fb_id;
    uint32_t handle;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t size;
    void* ptr;
    drmModeModeInfo mode;
    uint32_t r_off; 
    uint32_t g_off;
    uint32_t b_off;
} __attribute__((aligned(8))) DRMStruct;

int DRMinit(DRMStruct* ctx, const char* path);
void DRMpgFlip(DRMStruct* ctx);

#endif
