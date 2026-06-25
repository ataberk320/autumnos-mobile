#ifndef FB_TYPES_H
#define FB_TYPES_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    int fd;
    void* ptr;
    size_t size;
    int width;
    int height;
    int stride;
    uint32_t r_off;
    uint32_t g_off;
    uint32_t b_off;
    uint32_t bpp;
} FramebufferStruct;

#endif
