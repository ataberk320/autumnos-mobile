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

extern FBDEV_HAL* fbd;
void* handle = NULL;
void* font_h = NULL;

void AutumnAPI_DrawPix(FbDev* fb, int x, int y, uint32_t color) {
	if (x >= 0 && x < fb->w && y >= 0 && y < fb->h) {
        	uint8_t* row_start = ((uint8_t*)fb->bk_bf) + (y * fb->stride);
        	((uint32_t*)row_start)[x] = color;
    	}
}

void AutumnAPI_DrawLine(FbDev* fb, int x0, int y0, int x1, int y1, uint32_t color) {
	int dx = abs(x1 - x0);
	int dy = -abs(y1 - y0);
	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;
	int err = dx + dy;
	int e2;

	while (1) {
		AutumnAPI_DrawPix(fb, x0, y0, color);
		if (x0 == x1 && y0 == y1) break;
		e2 = 2 * err;
		if (e2 >= dy) {
            		err += dy;
            		x0 += sx;
        	}
        	if (e2 <= dx) {
            		err += dx;
            		y0 += sy;
        	}
    	}
}

void AutumnAPI_DrawImg(FbDev* fb, AutumnImage* img, int x, int y) {
    if (!img || !img->buffer) return;

    for (int i = 0; i < img->h; i++) {
        int target_y = y + i;
        if (target_y < 0 || target_y >= fb->h) continue;

        uint32_t* src_row = &img->buffer[i * img->w];
        uint32_t* target_row = fb->bk_bf + (target_y * (fb->stride / 4));

        for (int j = 0; j < img->w; j++) {
            int target_x = x + j;
            if (target_x < 0 || target_x >= fb->w) continue;

            uint32_t pixel = src_row[j];
            uint8_t a = (pixel >> 24) & 0xFF;

            if (a == 0) continue;

            uint8_t r = (pixel >> 0)  & 0xFF;
            uint8_t g = (pixel >> 8)  & 0xFF;
            uint8_t b = (pixel >> 16) & 0xFF;

            if (a < 255) {
                uint32_t dest = target_row[target_x];
                uint8_t dr = (dest >> fb->r_off) & 0xFF;
                uint8_t dg = (dest >> fb->g_off) & 0xFF;
                uint8_t db = (dest >> fb->b_off) & 0xFF;

                r = (r * a + dr * (255 - a)) / 255;
                g = (g * a + dg * (255 - a)) / 255;
                b = (b * a + db * (255 - a)) / 255;
            }

            target_row[target_x] = (r << fb->r_off) | (g << fb->g_off) | (b << fb->b_off);
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
                    fb->bk_bf[py * (fb->stride / 4) + px] = (tr << fb->r_off) | (tg << fb->g_off) | (tb << fb->b_off);
                }
		else {
                    uint32_t bg_pixel = fb->bk_bf[py * (fb->stride / 4) + px];
                    
                    uint8_t br = (bg_pixel >> fb->r_off) & 0xFF;
                    uint8_t bg = (bg_pixel >> fb->g_off) & 0xFF;
                    uint8_t bb = (bg_pixel >> fb->b_off) & 0xFF;

                    uint8_t fr = (tr * alpha + br * (255 - alpha)) / 255;
                    uint8_t fg = (tg * alpha + bg * (255 - alpha)) / 255;
                    uint8_t fb_val = (tb * alpha + bb * (255 - alpha)) / 255;
			
		    fb->bk_bf[py * (fb->stride / 4) + px] = (fr << fb->r_off) | (fg << fb->g_off) | (fb_val << fb->b_off);
                }
            }
        }
        x += face->glyph->advance.x >> 6;
    }
}

//UPDATE: palette functions are removed from FBDEV API!

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

void AutumnAPI_DrawTextbox(FbDev* fb, FT_Face face, int x, int y, int w, int h, int r, const char* text, const char* placeholder, uint32_t text_color) {
    uint32_t border_color = 0xFFFFFFFF; // white
    uint32_t box_bg_color = 0xFF2A2A2A; // gray

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int dx = (j < r) ? (r - j) : ((j > w - 1 - r) ? (j - (w - 1 - r)) : 0);
            int dy = (i < r) ? (r - i) : ((i > h - 1 - r) ? (i - (h - 1 - r)) : 0);
            
            if (dx * dx + dy * dy < r * r) {
                int dist_sq = dx * dx + dy * dy;
                if (dist_sq >= (r - 1) * (r - 1) || i == 0 || i == h - 1 || j == 0 || j == w - 1) {
                    AutumnAPI_DrawRect(fb, x + j, y + i, 1, 1, border_color, 1);
                } else {
                    AutumnAPI_DrawRect(fb, x + j, y + i, 1, 1, box_bg_color, 1);
                }
            }
        }
    }

    int has_text = (text != NULL && text[0] != '\0');
    const char* display_str = has_text ? text : placeholder;
    
    uint32_t final_text_color = has_text ? text_color : 0xFF888888;

    int padding_x = 10;
    int ascender  = face->size->metrics.ascender >> 6;
    int descender = face->size->metrics.descender >> 6;

    int tx = x + padding_x;
    int ty = y + (h / 2) + ((ascender + descender) >> 1);

    AutumnAPI_DrawString(fb, face, display_str, tx, ty, final_text_color);
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
	
		uint32_t* target_row = fb->bk_bf + (target_y * (fb->stride / 4));
		
		for (int j = 0; j < desc->Width; j++) {
			int target_x = x + desc->Left + j;
			if (target_x < 0 || target_x >= fb->w) continue;
			
			GifByteType pixel_idx = frame->RasterBits[i * desc->Width + j];
			
			if (pixel_idx == trans_idx) continue;

            		GifColorType color = cmap->Colors[pixel_idx];
			uint8_t r = color.Red;
			uint8_t g = color.Green;
			uint8_t b = color.Blue;

			target_row[target_x] = (r << fb->r_off) | (g << fb->g_off) | (b << fb->b_off);

        	}
    	}
}


int AutumnAPI_Init(FbDev* fb, const char *path) {
    fb->hal_ctx = malloc(sizeof(DRMStruct));
    if (!fb->hal_ctx) return -1;

    if (fbd->InitFbCard(fb->hal_ctx, "/dev/dri/card0") != 0) {
        printf("Turning to terminal.");
	free(fb->hal_ctx);
	return -1;
    }

    fb->fd     = fb->hal_ctx->fd;
    fb->w      = fb->hal_ctx->width;
    fb->h      = fb->hal_ctx->height;
    fb->fb_ptr = (uint32_t*)fb->hal_ctx->ptr;
    fb->stride = fb->hal_ctx->stride;
    fb->r_off = 16;
    fb->g_off = 8;
    fb->b_off = 0;
    printf("R:%d, G:%d, B:%d\n", fb->r_off, fb->g_off, fb->b_off);
    printf("height: %d, stride: %d, malloc size: %lu\n", fb->h, fb->stride, (size_t)fb->h * fb->stride);    
    fb->bk_bf = malloc((size_t)fb->h * fb->stride);
    if (!fb->bk_bf) {
    	free(fb->hal_ctx);
    	return -1;
    }
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
			fb->bk_bf[y * (fb->stride / 4) + x] = color;
		}
	}
}

int AutumnAPI_FbRefresh(FbDev* fb) {
	if (!fb || !fb->hal_ctx || !fb->bk_bf || !fb->hal_ctx->ptr) {
		printf("NULL screen pointer\n");
		return -1;
	}

	uint32_t* src = fb->bk_bf;
	uint32_t* dst = (uint32_t*)fb->hal_ctx->ptr;

	for (int y = 0; y < fb->h; y++) {
		memcpy(dst + (y * (fb->stride / 4)), src + (y * (fb->stride / 4)), fb->w * sizeof(uint32_t));
	}

	fbd->FbFlip(fb->hal_ctx);
    	return 0;
}

void AutumnAPI_FbReset(DRMStruct* fb) {
        	fbd->ResetFbCard(fb, 1); 
}
