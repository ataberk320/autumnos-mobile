#include <png.h>
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
#include "libfbdev.h"
typedef struct {
	int version;
	int (*fbinit)(FramebufferStruct*);
	void (*fbrefresh)(FramebufferStruct*);
	void (*fbreset)(int, int);
} HAL_FramebufferTable;

HAL_FramebufferTable* hfbt;
void* handle = NULL;

int InitHardwareFunc() {
	hfbt = malloc(sizeof(HAL_FramebufferTable));
	handle = dlopen("/usr/lib/libatmhal.so", RTLD_LAZY); //	Loading the HAL library is important for fb0

	if (!handle) {
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
        fb->fb_ptr[y * fb->stride + x] = color;
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

void AutumnAPI_DrawButton(FbDev* fb, int x, int y, int w, int h, int r, uint32_t bg_color) {
	if (bg_color == 0) bg_color = 0xFFE0E0E0;
	AutumnAPI_DrawRect(fb, x + 3, y + 3, w, h, 0x30000000, 1);
	
	for (int i = 0; i < r; i++) {
		AutumnAPI_DrawRect(fb, x + r - i, y + i, w - 2*(r - i), h - 2*i, bg_color, 1);
	}
	AutumnAPI_DrawRect(fb, x, y + r, w, h - 2*r, bg_color, 1);

    	AutumnAPI_DrawRect(fb, x + 2, y + 2, 1, h - 4, 0x40000000, 1);
    	AutumnAPI_DrawRect(fb, x + 2, y + 2, w - 4, 1, 0x40000000, 1);
    
    	AutumnAPI_DrawRect(fb, x + w - 3, y + 2, 1, h - 4, 0x40FFFFFF, 1);
    	AutumnAPI_DrawRect(fb, x + 2, y + h - 3, w - 4, 1, 0x40FFFFFF, 1);
}

void AutumnAPI_DrawImg(FbDev* fb, const char* path, int x, int y) {
	FILE* fp = fopen(path, "rb");
	png_structp png =  png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info = png_create_info_struct(png);
	png_init_io(png, fp);
	png_read_info(png, info);
	int w = png_get_image_width(png, info);
	int h = png_get_image_height(png, info);
	png_bytep* rows = malloc(sizeof(png_bytep) * h);
	for(int i=0; i<h; i++) rows[i] = malloc(png_get_rowbytes(png, info));
    	png_read_image(png, rows);

    	for (int i=0; i<h; i++) {
		uint8_t* row_ptr = (uint8_t*)rows[i];

        	for(int j=0; j<w; j++) {
        		uint8_t r = row_ptr[j * 4 + 0];
        		uint8_t g = row_ptr[j * 4 + 1];
        		uint8_t b = row_ptr[j * 4 + 2];
        		uint32_t final_pixel = (r << fb->r_off) | (g << fb->g_off) | (b << fb->b_off);
        		fb->fb_ptr[(y + i) * fb->stride + (x + j)] = final_pixel;
        	}

        	free(rows[i]);
    	}
    	free(rows); fclose(fp); png_destroy_read_struct(&png, &info, NULL);
}

void AutumnAPI_DrawString(FbDev* fb, FT_Face face, const char* text, int x, int y, uint32_t color) {
	for (int i = 0; text[i]; i++) {
        	FT_Load_Char(face, text[i], FT_LOAD_RENDER);
        	FT_Bitmap* bmp = &face->glyph->bitmap;
        	for (int gy = 0; gy < bmp->rows; gy++) {
            		for (int gx = 0; gx < bmp->width; gx++) {
                		if (bmp->buffer[gy * bmp->width + gx]) {
                    			int px = x + face->glyph->bitmap_left + gx;
                    			int py = y - face->glyph->bitmap_top + gy;
                    			fb->fb_ptr[py * fb->stride + px] = color;
                		}
            		}
        	}
        	x += face->glyph->advance.x >> 6;
    	}
}

//Framebuffer device test functions
int AutumnAPI_Init(FbDev* fb) {
    if (hfbt == NULL) {
        if (InitHardwareFunc() != 0) return -1;
    }
    
    FramebufferStruct hal_ctx;
    if (hfbt->fbinit(&hal_ctx) != 0) return -1; 
    
    fb->fd     = hal_ctx.fd;
    fb->w      = hal_ctx.width;
    fb->h      = hal_ctx.height;
    fb->stride = hal_ctx.stride;
    fb->fb_ptr = (uint32_t*)hal_ctx.ptr;
    
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
			fb->fb_ptr[y * fb->stride + x] = color;
		}
	}
}

void AutumnAPI_FbRefresh(FbDev* fb) {
	if (hfbt->fbrefresh) {
		hfbt->fbrefresh(fb->fd);
	}
}

void AutumnAPI_FbReset(FbDev* fb) {
    if (hfbt->fbreset) {
        hfbt->fbreset(fb->fd, 1); 
    }
}
