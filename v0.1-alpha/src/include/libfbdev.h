#ifndef UIGFX_H
#define UIGFX_H

#include <stdint.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "AutumnImage.h"
#include "FramebufferStruct.h"

typedef struct {
	int fd;
	uint32_t* bk_bf;
	uint32_t* fb_ptr;
	int w, h;
	int stride;	
	int r_off; 
    	int g_off; 
    	int b_off;
	DRMStruct* hal_ctx;
} FbDev;

#endif
