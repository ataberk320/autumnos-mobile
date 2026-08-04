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

uint32_t AutumnAPI_UnicodeTF8(const char** s);
void AutumnAPI_DrawPix(FbDev* fb, int x, int y, uint32_t color);
void AutumnAPI_DrawRect(FbDev* fb, int x, int y, int w, int h, uint32_t color, int fill);
void AutumnAPI_DrawButton(FbDev* fb, FT_Face face, int x, int y, int w, int h, int r, uint32_t bg_color, const char* text, uint32_t text_color, AutumnImage* icon);
void AutumnAPI_DrawImg(FbDev* fb, AutumnImage* img, int x, int y);
void AutumnAPI_DrawString(FbDev* fb, FT_Face face, const char* text, int x, int y, uint32_t color);

int AutumnAPI_Init(FbDev* fb, const char *path);
void AutumnAPI_URandom(FbDev* fb);
void AutumnAPI_ClearFb(FbDev* fb, uint32_t color);
int AutumnAPI_FbRefresh(FbDev* fb);
void AutumnAPI_FbReset(DRMStruct* fb);

#endif
