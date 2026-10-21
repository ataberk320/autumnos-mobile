#include "../include/HAL/alog.h"
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
#include "../include/HAL/FramebufferStruct.h"
#include "../include/item_ui/AutumnImage.h"
#include "../include/libfbdev.h"
#include "../include/SymbolTable/table.h"
#include <stdbool.h>
#include "../include/HAL/io.h"

extern SYM_TAB* symb;
void* handle = NULL;
void* font_h = NULL;
bool eye_shield = false;
static bool invert_colors = false;
extern void _png_RdCallback(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead);

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

void AutumnAPI_DrawTriangle(FbDev* fb, int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color, int fill) {
	if (fill) {
		if (y0 > y1) { int t = y0; y0 = y1; y1 = t; t = x0, x0 = x1; x1 =t; }
		if (y0 > y2) { int t = y0; y0 = y2; y2 = t; t = x0; x0 = x2; x2 = t; }
        	if (y1 > y2) { int t = y1; y1 = y2; y2 = t; t = x1; x1 = x2; x2 = t; }

		int tot_h = y2 - y0;
		if (tot_h == 0) return;

		for (int i = 0; i <= tot_h; i++) {
			int sec_half = i > y1 - y0 || y1 == y0;
			int seg_h = sec_half ? y2 - y1 : y1 - y0;
			if (seg_h == 0) continue;

			float alpha = (float)i / tot_h;
			float beta = (float)(i - (sec_half ? y1 - y0 : 0)) / seg_h;

			int ax = x0 + (int)((x2 - x0) * alpha);
			int bx = sec_half ? x1 + (int)((x2 - x1) * beta) : x0 + (int)((x1 - x0) * beta);

            		if (ax > bx) { int t = ax; ax = bx; bx = t; }

            		for (int j = ax; j <= bx; j++) {
                		AutumnAPI_DrawPix(fb, j, y0 + i, color);
            		}
        	}
    	}

	else {
        	AutumnAPI_DrawLine(fb, x0, y0, x1, y1, color);
        	AutumnAPI_DrawLine(fb, x1, y1, x2, y2, color);
        	AutumnAPI_DrawLine(fb, x2, y2, x0, y0, color);
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

void AutumnAPI_DrawAdvancedRect(FbDev* fb, int x, int y, int w, int h, uint32_t color, CornerRadius r, int fill) {
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			int target_x = x + j;
			int target_y = y + i;
			
			if (target_x < 0 || target_x >= fb->w || target_y < 0 || target_y >= fb->h)
                		continue;

			int draw = 1;

			if ( i < r.tl && j < r.tl) {
				int dx = r.tl - j;
				int dy = r.tl - i;
				if (dx * dx + dy * dy > r.tl * r.tl) draw = 0;
            		}

			else if (i < r.tr && j >= w - r.tr) {
				int dx = j - (w - 1 - r.tr);
				int dy = r.tr - i;
				if (dx * dx + dy * dy > r.tr * r.tr) draw = 0;
            		}

			else if (i >= h - r.bl && j < r.bl) {
                		int dx = r.bl - j;
                		int dy = i - (h - 1 - r.bl);
                		if (dx * dx + dy * dy > r.bl * r.bl) draw = 0;
            		}

            		else if (i >= h - r.br && j >= w - r.br) {
                		int dx = j - (w - 1 - r.br);
                		int dy = i - (h - 1 - r.br);
                		if (dx * dx + dy * dy > r.br * r.br) draw = 0;
            		}

			if (draw) {
				if (fill) {
					AutumnAPI_DrawPix(fb, target_x, target_y, color);
                		}
				else { }
			}
		}
	}
}

void AutumnAPI_DrawString(FbDev* fb, FT_Face face, const char* text, int x, int y, uint32_t color, int pixel_size) {
    if (!fb || !fb->bk_bf || !face || !text) { a_perror("DrawString"); return; }

    FT_Set_Pixel_Sizes(face, 0, pixel_size);

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

int AutumnAPI_GetStringWidth(FT_Face face, const char* text) {
    int width = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        FT_Load_Char(face, text[i], FT_LOAD_RENDER);
        width += face->glyph->advance.x >> 6;
    }
    return width;
}

void AutumnAPI_DrawButton(FbDev* fb, FT_Face face, int x, int y, int w, int h, int r, uint32_t bg_color, const char* text, uint32_t text_color, AutumnImage* icon) {
    uint32_t border_color = 0xFF808080;

    int spacing = 4;
    int icon_w = icon ? icon->w : 0;
    int icon_h = icon ? icon->h : 0;

    int text_w = 0;
    if (text && text[0] != '\0') {
        for (int i = 0; text[i] != '\0'; i++) {
            FT_Load_Char(face, text[i], FT_LOAD_RENDER);
            text_w += face->glyph->advance.x >> 6;
        }
    }
    int ascender  = face->size->metrics.ascender >> 6;
    int descender = face->size->metrics.descender >> 6;
    int text_h = ascender - descender;
    
    int has_icon = (icon != NULL && icon->buffer != NULL);
    int has_text = (text != NULL && text[0] != '\0');

    int min_w = text_w + 20;
    if (has_icon && icon_w + 20 > min_w) {
        min_w = icon_w + 20;
    }
    
    if (w < min_w) {
        w = min_w;
    }

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int dx = (j < r) ? (r - j) : ((j > w - 1 - r) ? (j - (w - 1 - r)) : 0);
            int dy = (i < r) ? (r - i) : ((i > h - 1 - r) ? (i - (h - 1 - r)) : 0);
            
            if (dx * dx + dy * dy < r * r) {
                uint32_t color = (symb && symb->ApplyGrad) ? symb->ApplyGrad(bg_color, i, 0, h) : bg_color;
                AutumnAPI_DrawRect(fb, x + j, y + i, 1, 1, color, 1);
                
                int dist_sq = dx * dx + dy * dy;
                if (dist_sq >= (r - 1) * (r - 1)) {
                    AutumnAPI_DrawRect(fb, x + j, y + i, 1, 1, border_color, 1);
                }
            }
        }
    }

    int total_content_h = (has_icon ? icon_h : 0) + (has_icon && has_text ? spacing : 0) + (has_text ? text_h : 0);
    int start_y = y + (h - total_content_h) / 2;
    
    if (has_icon) {
        int icon_x = x + (w - icon_w) / 2;
        int icon_y = start_y;
        AutumnAPI_DrawImg(fb, icon, icon_x, icon_y);
    }

    if (has_text) {
        int tx = x + (w - text_w) / 2;
        int ty = start_y + (has_icon ? icon_h + spacing : 0) + ascender;
        AutumnAPI_DrawString(fb, face, text, tx, ty, text_color, 16);
    }
}

void AutumnAPI_DrawTextbox(FbDev* fb, FT_Face face, int x, int y, int w, int h, int r, const char* text, const char* placeholder, uint32_t text_color) {
    uint32_t border_color = 0xFFFFFFFF; // White frame
    uint32_t box_bg_color = 0xFF2A2A2A; // Gray

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

    AutumnAPI_DrawString(fb, face, display_str, tx, ty, final_text_color, 16);
}

void AutumnAPI_DrawWindowBody(FbDev *fb, int x, int y, int w, int h, int r, uint32_t window_bg, uint32_t border_color) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int dx = (j < r) ? (r - j) : ((j > w - 1 - r) ? (j - (w - 1 - r)) : 0);
            int dy = (i < r) ? (r - i) : ((i > h - 1 - r) ? (i - (h - 1 - r)) : 0);
            
            if (dx * dx + dy * dy >= r * r) continue;

            if (dx * dx + dy * dy >= (r - 1) * (r - 1)) {
                AutumnAPI_DrawRect(fb, x + j, y + i, 1, 1, border_color, 1);
            } 

            else {
                AutumnAPI_DrawRect(fb, x + j, y + i, 1, 1, window_bg, 1);
            }
        }
    }
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

    if (!symb || !symb->InitFbCard || symb->InitFbCard(fb->hal_ctx, path) != 0) {
        printf("Turning to terminal.");
	free(fb->hal_ctx);
	return -1;
    }

    fb->fd     = fb->hal_ctx->fd;
    fb->w      = fb->hal_ctx->width;
    fb->h      = fb->hal_ctx->height;
    fb->fb_ptr = (uint32_t*)fb->hal_ctx->ptr;
    fb->stride = fb->hal_ctx->stride;
    fb->r_off    = fb->hal_ctx->r_off;
    fb->g_off    = fb->hal_ctx->g_off;
    fb->b_off    = fb->hal_ctx->b_off;

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
	int ur_fd = _AutumnSys_ioOpen("/dev/urandom", A_RDONLY, 0); //Glitch test XD
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
    if (!fb || !fb->hal_ctx || !fb->bk_bf || !fb->hal_ctx->ptr) return -1;

    uint32_t* src = fb->bk_bf;
    uint32_t* dst = (uint32_t*)fb->hal_ctx->ptr;

    for (int y = 0; y < fb->h; y++) {
        for (int x = 0; x < fb->w; x++) {
            int i = y * (fb->stride / 4) + x;
            uint32_t p = src[i];
		
	    uint8_t r = (p >> fb->r_off) & 0xFF;
            uint8_t g = (p >> fb->g_off) & 0xFF;
            uint8_t b = (p >> fb->b_off) & 0xFF;

	    if (invert_colors) {
                r = 255 - r;
                g = 255 - g;
                b = 255 - b;
            }

            if (eye_shield) {
                int alpha = 30; 
                r = (r * (255 - alpha) + 255 * alpha) / 255;
                g = (g * (255 - alpha) + 180 * alpha) / 255;
                b = (b * (255 - alpha) + 100 * alpha) / 255;
            }

            dst[i] = (r << fb->r_off) | (g << fb->g_off) | (b << fb->b_off);
        }
    }

    if (symb && symb->FbFlip) {
        symb->FbFlip(fb->hal_ctx);
    }
    return 0;
}


void AutumnAPI_FbReset(DRMStruct* fb) {
    if (symb && symb->ResetFbCard) {
        symb->ResetFbCard(fb, 1); 
    }
}
