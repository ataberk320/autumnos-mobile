#include <string.h>
#include <stdint.h>
#include <ft2build.h>
#include "FramebufferStruct.h"
#include "table.h"
#include "window.h"
#include "libfbdev.h"
#include <math.h>
extern GFX_API* gfx;
extern WIDGET_API* wid;
extern PALETTE_API* plt;

void AutumnAPI_DrawWindow(FbDev *fb, Window *win, FT_Face face, int an_speed) {
    if (win->anim != NULL) {
        win->anim(win, fb, an_speed);
    }

    if (win->alpha <= 0 && win->anim == NULL) {
        return;
    }

    win->x = (fb->w - win->w) / 2;
    win->y = (fb->h - win->h) / 2;

    int alpha = win->alpha;
    uint32_t dark_green_bg = 0xFF1B4D3E;
    //rounding
    for (int py = 0; py < win->h; py++) {
        int start_offset = 0, end_offset = win->w;
        int current_y = win->y + py;

        if (py < win->r) {
            int dy = win->r - py;
            int dx = win->r - (int)(sqrt((double)(win->r * win->r - dy * dy)) + 0.5);
            start_offset = dx; end_offset = win->w - dx;
        } else if (py >= win->h - win->r) {
            int dy = py - (win->h - win->r);
            int dx = win->r - (int)(sqrt((double)(win->r * win->r - dy * dy)) + 0.5);
            start_offset = dx; end_offset = win->w - dx;
        }
		//gradient
        uint32_t grad_color = plt->ApplyGrad(dark_green_bg, py, 0, win->h);

        for (int px = start_offset; px < end_offset; px++) {
            int target_x = win->x + px;
            if (target_x >= 0 && target_x < fb->w && current_y >= 0 && current_y < fb->h) {
                uint32_t final_color = plt->Blend(fb, target_x, current_y, grad_color, alpha);
                if (fb->bk_bf) fb->bk_bf[current_y * (fb->stride / 4) + target_x] = final_color;
            }
        }
    }

    int panel_h = 50, border_inset = 2;
    int panel_x = win->x + border_inset, panel_w = win->w - (border_inset * 2);
    int panel_y = win->y + win->h - panel_h - border_inset;
    int r = win->r - border_inset; if (r < 0) r = 0;

    for (int py = 0; py < panel_h; py++) {
        int start_offset = 0, end_offset = panel_w, current_y = panel_y + py;
        if (py >= panel_h - r) {
            int dy = py - (panel_h - r);
            int dx = r - (int)(sqrt((double)(r * r - dy * dy)) + 0.5);
            start_offset = dx; end_offset = panel_w - dx;
        }
        for (int px = start_offset; px < end_offset; px++) {
            int target_x = panel_x + px;
            if (target_x >= 0 && target_x < fb->w && current_y >= 0 && current_y < fb->h) {
                uint32_t final_color = plt->Blend(fb, target_x, current_y, 0xFF808080, alpha);
                if (fb->bk_bf) fb->bk_bf[current_y * (fb->stride / 4) + target_x] = final_color;
            }
        }
    }

    int btn_w = 180, btn_h = 50, spacing = 20;
    int total_w = (win->btn_count * btn_w) + ((win->btn_count - 1) * spacing);
    int start_x = win->x + (win->w - total_w) / 2;

    for (int i = 0; i < win->btn_count; i++) {
        int btn_x = start_x + (i * (btn_w + spacing));
        int btn_y = panel_y + (panel_h - btn_h) / 2;
        
        wid->SetBtnEnv(&win->buttons[i], btn_x, btn_y, btn_w, btn_h, 0xFFD3D3D3, 0xFFE6A23C, win->labels[i]);

        for (int by = 0; by < btn_h; by++) {
            for (int bx = 0; bx < btn_w; bx++) {
                int target_x = btn_x + bx;
                int current_y = btn_y + by;
                if (target_x >= 0 && target_x < fb->w && current_y >= 0 && current_y < fb->h) {
                    uint32_t final_btn_color = plt->Blend(fb, target_x, current_y, 0xFFD3D3D3, alpha);
                    if (fb->bk_bf) fb->bk_bf[current_y * (fb->stride / 4) + target_x] = final_btn_color;
                }
            }
        }
    }

    uint32_t text_color = (0xFF << 24) | ((255 * alpha / 255) << 16) | ((255 * alpha / 255) << 8) | (255 * alpha / 255);
    uint32_t green_line = (0xFF << 24) | (0 << 16) | ((255 * alpha / 255) << 8) | (0);

    gfx->Text(fb, face, win->title, win->x + 15, win->y + 25, text_color);
    gfx->DrawLine(fb, win->x + 10, win->y + 40, win->x + win->w - 10, win->y + 40, green_line);	

    int cur_x = win->x + 10, cur_y = win->y + 60;
    char buffer[256];
    strncpy(buffer, win->message, 255);
    char *token = strtok(buffer, " ");

    while (token != NULL) {
        int tw = gfx->GetStrWidth(face, token);
        if (cur_x + tw > win->x + win->w - 10) { cur_x = win->x + 10; cur_y += 25; }
        gfx->Text(fb, face, token, cur_x, cur_y, text_color);
        cur_x += tw + 10;
        token = strtok(NULL, " ");
    }

    for (int i = 0; i < win->btn_count; i++) {
        int btn_x = start_x + (i * (btn_w + spacing));
        int btn_y = panel_y + (panel_h - btn_h) / 2;
        int tw = gfx->GetStrWidth(face, win->labels[i]);
        
        uint32_t btn_text_c = (0xFF << 24) | ((30 * alpha / 255) << 16) | ((30 * alpha / 255) << 8) | (30 * alpha / 255);
        gfx->Text(fb, face, win->labels[i], btn_x + (btn_w - tw) / 2, btn_y + (btn_h / 2) - 6, btn_text_c);
    }
}
//speed supported fade animations!
void AutumnAPI_Anim_FadeIn(void* win_ptr, void* fb_ptr, int speed) {
	Window* win = (Window*)win_ptr;

	if (win->alpha < 255) {
		win->alpha += speed;
		if (win->alpha > 255) win->alpha = 255;
	}
	else {
		win->anim = NULL;
	}
}

void AutumnAPI_Anim_FadeOut(void* win_ptr, void* fb_ptr, int speed) {
    Window* win = (Window*)win_ptr;
    
    if (win->alpha > 0) {
        win->alpha -= speed;
        if (win->alpha < 0) win->alpha = 0;
    }
    else {
        win->anim = NULL;
    }
}
