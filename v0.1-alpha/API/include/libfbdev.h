#ifndef UIGFX_H
#define UIGFX_H

#include <stdint.h>
#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
	int fd;
	uint32_t* fb_ptr;
	int w, h;
	int stride;	
	int r_off; 
    	int g_off; 
    	int b_off;
} FbDev;

void AutumnAPI_DrawPix(FbDev* fb, int x, int y, uint32_t color);
void AutumnAPI_DrawRect(FbDev* fb, int x, int y, int w, int h, uint32_t color, int fill);
void AutumnAPI_DrawButton(FbDev* fb, int x, int y, int w, int h, int r, uint32_t bg_color);
void AutumnAPI_DrawImg(FbDev* fb, const char* path, int x, int y);
void AutumnAPI_DrawString(FbDev* fb, FT_Face face, const char* text, int x, int y, uint32_t color);

int AutumnAPI_Init(FbDev* fb);
void AutumnAPI_URandom(FbDev* fb);
void AutumnAPI_ClearFb(FbDev* fb, uint32_t color);
void AutumnAPI_FbRefresh(FbDev* fb);
void AutumnAPI_FbReset(FbDev* fb);

#endif
