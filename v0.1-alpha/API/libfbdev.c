#include <ft2build.h>
#include FT_FREETYPE_H
#include <png.h>
#include <gif_lib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include "FramebufferStruct.h"
#include "AutumnImage.h"
#include "libfbdev.h"
#include "table.h"
typedef struct {
	int version;
	int (*fbinit)(FramebufferStruct*, const char*);
	void (*fbrefresh)(FramebufferStruct*);
	void (*fbreset)(int, int);
} HAL_FramebufferTable;

HAL_FramebufferTable* hfbt;
void* handle = NULL;
void* font_h = NULL;

int InitHardwareFunc() {
	hfbt = malloc(sizeof(HAL_FramebufferTable));
	handle = dlopen("/usr/lib/libfbdhal.so", RTLD_LAZY); //	Loading the HAL library is important for fb0
	font_h = dlopen("/usr/lib/libatmnls.so", RTLD_LAZY);
	if (!handle || !font_h) {
		return -1; // File not found
	}
	
	hfbt->fbinit = dlsym(handle, "hal_fbinit"); // Loading function from library
	hfbt->fbrefresh = dlsym(handle, "hal_fbrefresh");
	hfbt->fbreset   = dlsym(handle, "hal_fbreset");
    	if (!hfbt->fbinit || !hfbt->fbrefresh || !hfbt->fbreset) {
        	return -1; // File not found
    	}

    return 0; // Success
}

//Framebuffer drawing functions
void AutumnAPI_DrawPix(FbDev* fb, int x, int y, uint32_t color) {
    if (x >= 0 && x < fb->w && y >= 0 && y < fb->h) {
        fb->bk_bf[y * fb->stride + x] = color;
    }
}


void AutumnAPI_DrawImg(FbDev* fb, AutumnImage* img, int x, int y) {
    if (!img || !img->buffer) return;

    for (int i = 0; i < img->h; i++) {
        int target_y = y + i;
        if (target_y < 0 || target_y >= fb->h) continue;

        uint32_t* src_row = &img->buffer[i * img->w];
        uint32_t* target_row = fb->bk_bf + (target_y * fb->stride);

        for (int j = 0; j < img->w; j++) {
            int target_x = x + j;
            if (target_x < 0 || target_x >= fb->w) continue;

            uint32_t pixel = src_row[j];
            
            uint8_t r = (pixel >> 0)  & 0xFF;
            uint8_t g = (pixel >> 8)  & 0xFF;
            uint8_t b = (pixel >> 16) & 0xFF;
            uint8_t a = (pixel >> 24) & 0xFF;

            if (a == 0) continue;

            if (a == 255) {
                target_row[target_x] = (r << fb->r_off) | (g << fb->g_off) | (b << fb->b_off);
            } else {
                uint32_t dest = target_row[target_x];
                uint8_t dr = (dest >> fb->r_off) & 0xFF;
                uint8_t dg = (dest >> fb->g_off) & 0xFF;
                uint8_t db = (dest >> fb->b_off) & 0xFF;

                uint8_t out_r = (r * a + dr * (255 - a)) >> 8;
                uint8_t out_g = (g * a + dg * (255 - a)) >> 8;
                uint8_t out_b = (b * a + db * (255 - a)) >> 8;

                target_row[target_x] = (out_r << fb->r_off) | (out_g << fb->g_off) | (out_b << fb->b_off);
            }
        }
    }
}

void AutumnAPI_DrawRect(FbDev* fb, int x, int y, int w, int h, uint32_t color, int fill) {
    if (fill) {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                AutumnAPI_DrawPix(fb, x + j, y + i, color);
            }
        }
    } 
    else {
        for (int i = 0; i < w; i++) {
            AutumnAPI_DrawPix(fb, x + i, y, color);
            AutumnAPI_DrawPix(fb, x + i, y + h - 1, color);
        }

        for (int i = 0; i < h; i++) {
            AutumnAPI_DrawPix(fb, x, y + i, color);
            AutumnAPI_DrawPix(fb, x + w - 1, y + i, color);
        }
    }
}


void AutumnAPI_DrawString(FbDev* fb, FT_Face face, const char* text, int x, int y, uint32_t color) {
    if (!fb || !fb->bk_bf || !face || !text) { perror("DrawString"); return; }

    const char* ptr = text;
    
    uint8_t tr = (color >> 16) & 0xFF;
    uint8_t tg = (color >> 8) & 0xFF;
    uint8_t tb = color & 0xFF;

    while (*ptr) {
        uint32_t cp = AutumnAPI_UnicodeTF8(&ptr);
        if (cp == 0) break;

        if (FT_Load_Char(face, cp, FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL)) continue;

        FT_Bitmap* bmp = &face->glyph->bitmap;
        
        for (int gy = 0; gy < bmp->rows; gy++) {
            for (int gx = 0; gx < bmp->width; gx++) {
                int px = x + face->glyph->bitmap_left + gx;
                int py = y - face->glyph->bitmap_top + gy;

                if (px < 0 || px >= fb->w || py < 0 || py >= fb->h) continue;

                uint8_t alpha = bmp->buffer[gy * bmp->width + gx];
                
                if (alpha == 0) continue;

                if (alpha == 255) {
                    fb->bk_bf[py * fb->stride + px] = color;
                }
		else {
                    uint32_t bg_pixel = fb->bk_bf[py * fb->stride + px];
                    
                    uint8_t br = (bg_pixel >> 16) & 0xFF;
                    uint8_t bg = (bg_pixel >> 8) & 0xFF;
                    uint8_t bb = bg_pixel & 0xFF;

                    uint8_t fr = (tr * alpha + br * (255 - alpha)) >> 8;
                    uint8_t fg = (tg * alpha + bg * (255 - alpha)) >> 8;
                    uint8_t fb_val = (tb * alpha + bb * (255 - alpha)) >> 8;

                    fb->bk_bf[py * fb->stride + px] = (fr << 16) | (fg << 8) | fb_val;
                }
            }
        }
        x += face->glyph->advance.x >> 6;
    }
}

uint32_t GetGradientColor(uint32_t base_color, int y, int start_y, int height) {
    int factor = ((y - start_y) * 40 / height) - 20;
    int r = (base_color >> 16) & 0xFF;
    int g = (base_color >> 8) & 0xFF;
    int b = base_color & 0xFF;
    r = (r + factor > 255) ? 255 : (r + factor < 0) ? 0 : r + factor;
    g = (g + factor > 255) ? 255 : (g + factor < 0) ? 0 : g + factor;
    b = (b + factor > 255) ? 255 : (b + factor < 0) ? 0 : b + factor;
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}

void AutumnAPI_DrawButton(FbDev* fb, FT_Face face, int x, int y, int w, int h, int r, uint32_t bg_color, const char* text, uint32_t text_color) {
    uint32_t border_color = 0xFF808080;

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int dx = (j < r) ? (r - j) : ((j > w - 1 - r) ? (j - (w - 1 - r)) : 0);
            int dy = (i < r) ? (r - i) : ((i > h - 1 - r) ? (i - (h - 1 - r)) : 0);
            
            if (dx * dx + dy * dy < r * r) {
                uint32_t color = GetGradientColor(bg_color, y + i, y, h);
                AutumnAPI_DrawRect(fb, x + j, y + i, 1, 1, color, 1);
                
                int dist_sq = dx * dx + dy * dy;
                if (dist_sq >= (r - 1) * (r - 1)) {
                    AutumnAPI_DrawRect(fb, x + j, y + i, 1, 1, border_color, 1);
                }
            }
        }
    }

    int text_w = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        FT_Load_Char(face, text[i], FT_LOAD_RENDER);
        text_w += face->glyph->advance.x >> 6;
    }

    int ascender  = face->size->metrics.ascender >> 6;
    int descender = face->size->metrics.descender >> 6;

    int tx = x + (w - text_w) / 2;
    int ty = y + (h / 2) + ((ascender + descender) >> 1);

    AutumnAPI_DrawString(fb, face, text, tx, ty, text_color);
}

void AutumnAPI_DrawGifAni(FbDev* fb, GifFileType* gif, int x, int y, int idx) {
	if (!gif || idx < 0 || idx >= gif->ImageCount) return;

	SavedImage* frame = &gif->SavedImages[idx];
	GifImageDesc* desc = &frame->ImageDesc;
	ColorMapObject* cmap = desc->ColorMap ? desc->ColorMap : gif->SColorMap;

	int trans_idx = -1;
	for (int k = 0; k < frame->ExtensionBlockCount; k++) {
		if (frame->ExtensionBlocks[k].Function == GRAPHICS_EXT_FUNC_CODE) {
			if (frame->ExtensionBlocks[k].Bytes[0] & 0x01) {
				trans_idx = (unsigned char)frame->ExtensionBlocks[k].Bytes[3];
            		}
        	}
    	}

	for (int i = 0; i < desc->Height; i++) {
		int target_y = y + desc->Top + i;
		if (target_y < 0 || target_y >= fb->h) continue;
	
		uint32_t* target_row = fb->bk_bf + (target_y * fb->stride);
		
		for (int j = 0; j < desc->Width; j++) {
			int target_x = x + desc->Left + j;
			if (target_x < 0 || target_x >= fb->w) continue;
			
			GifByteType pixel_idx = frame->RasterBits[i * desc->Width + j];
			
			if (pixel_idx == trans_idx) continue;

            		GifColorType color = cmap->Colors[pixel_idx];
            		target_row[target_x] = (color.Red << fb->r_off) |
                                   	       (color.Green << fb->g_off) |
                                               (color.Blue << fb->b_off);
        	}
    	}
}


//Framebuffer device test functions
int AutumnAPI_Init(FbDev* fb, const char *path) {
    if (hfbt == NULL) {
        if (InitHardwareFunc() != 0) return -1;
    }
    
    FramebufferStruct hal_ctx;
    if (hfbt->fbinit(&hal_ctx, "/dev/fb0") != 0) return -1;     
    fb->fd     = hal_ctx.fd;
    fb->w      = hal_ctx.width;
    fb->h      = hal_ctx.height;
    fb->stride = hal_ctx.stride;
    fb->fb_ptr = (uint32_t*)hal_ctx.ptr;
    fb->r_off = hal_ctx.r_off;
    fb->g_off = hal_ctx.g_off;
    fb->b_off = hal_ctx.b_off;
    fb->bk_bf = malloc(fb->h * fb->stride * sizeof(uint32_t));
    return 0;
}

void AutumnAPI_URandom(FbDev* fb) {
	int ur_fd = open("/dev/urandom", O_RDONLY); //Glitch test XD
	if (ur_fd < 0) return;
	
	uint32_t pixel;
	for (int i = 0; i < (fb->w * fb->h); i++) {
        	read(ur_fd, &pixel, sizeof(uint32_t));
        	fb->fb_ptr[i] = pixel;
    	}

    	close(ur_fd);
}

void AutumnAPI_ClearFb(FbDev* fb, uint32_t color) {
	for (int y = 0; y < fb->h; y++) {
		for (int x = 0; x < fb->w; x++) {
			fb->bk_bf[y * fb->stride + x] = color;
		}
	}
}

void AutumnAPI_FbHwRefresh(FbDev* fb) {
	if (hfbt->fbrefresh) {
		FramebufferStruct ctx;
		ctx.fd = fb->fd;
		hfbt->fbrefresh(&ctx);
	}
}

int AutumnAPI_FbRefresh(FbDev* fb) {
	if (!fb->fb_ptr || !fb->bk_bf) return -1;
	for (int y = 0; y < fb->h; y++) {
		uint32_t* dest = fb->fb_ptr + (y * fb->stride);
		uint32_t* src = fb->bk_bf + (y * fb->stride);
		memcpy(dest, src, fb->w * sizeof(uint32_t));
	}    
    	return 0;
}

void AutumnAPI_FbReset(FbDev* fb) {
    	if (hfbt->fbreset) {
        	hfbt->fbreset(fb->fd, 1); 
    	}
}
