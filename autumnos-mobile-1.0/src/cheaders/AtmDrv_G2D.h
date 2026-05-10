#ifndef G2D_H
#define G2D_H

#include <stdint.h>
#include <sys/ioctl.h>

typedef enum {
	G2D_FORMAT_ARGB8888 = 0x00,
	G2D_FORMAT_RGB565 = 0x04,
	G2D_FORMAT_YUV420 = 0x08,
}  g2d_format_t;

typedef struct {
	uintptr_t addr;
	int32_t   w;
	int32_t   h;
	g2d_format_t fmt;
	int32_t pitch;
} g2d_image_t;

typedef struct {
	g2d_image_t src;
	g2d_image_t dst;
	int32_t     x;
	int32_t     y;
	int32_t     rotate;
	uint32_t    alpha;
} g2d_blt_t;

#define G2D_IOC_MAGIC       'G'
#define G2D_CMD_BITBLT      _IOWR(G2D_IOC_MAGIC, 0x01, g2d_blt_t)
#define G2D_CMD_FILLRECT    _IOWR(G2D_IOC_MAGIC, 0x02, g2d_blt_t)

int  AutumnCore_G2D_Init(void);
void AutumnCore_G2D_Deinit(void);
int  AutumnCore_G2D_Blit(uintptr_t src, uintptr_t dst, int w, int h, int rotate);

#endif
