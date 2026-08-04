#ifndef ATMDRV_G2D_H
#define ATMDRV_G2D_H

#include <stdint.h>
#include <sys/ioctl.h>

#define G2D_SUCCESS       0
#define G2D_ERR_INIT     -1
#define G2D_ERR_IOCTL    -2
#define G2D_ERR_INVALID  -3

typedef enum {
    G2D_ROTATE_0   = 0,
    G2D_ROTATE_90  = 1,
    G2D_ROTATE_180 = 2,
    G2D_ROTATE_270 = 3
} g2d_rotate_e;

typedef enum {
    G2D_FMT_ARGB8888 = 0x05,
    G2D_FMT_RGB888   = 0x01
} g2d_fmt_e;

typedef struct {
    uintptr_t addr;
    int w;
    int h;
    int pitch;
    g2d_fmt_e fmt;
} g2d_image_t;

typedef struct {
    g2d_image_t src;
    g2d_image_t dst;
    int x;
    int y;
    g2d_rotate_e rotate;
    uint8_t alpha;
    uint8_t reserved[3];
} g2d_blt_t;

#define G2D_IOC_MAGIC  'g'
#define G2D_CMD_BITBLT _IOW(G2D_IOC_MAGIC, 0x40, g2d_blt_t)

int  hal_initg2d(void);
void hal_disableg2d(void);
int  hal_bitbltg2d(uintptr_t src, uintptr_t dst, int w, int h, g2d_rotate_e rotate);

#endif // ATMDRV_G2D_H
