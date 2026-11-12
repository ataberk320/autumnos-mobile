
#include <string.h>
#include <stdint.h>
#include <ft2build.h>
#include <math.h>

#include "../include/HAL/FramebufferStruct.h"
#include "../include/SymbolTable/table.h"
#include "../include/item_ui/window.h"
#include "../include/libfbdev.h"

extern SYM_TAB* symb;

void AutumnAPI_DrawWindow(FbDev *fb, Window *win, FT_Face face, int an_speed)
{
    if (!fb || !win || !symb ||
        !symb->plt || !symb->gfx || !symb->wid)
        return;

    if (!symb->plt->ApplyGrad ||
        !symb->plt->Blend ||
        !symb->wid->SetBtnEnv ||
        !symb->gfx->Text ||
        !symb->gfx->DrawLine ||
        !symb->gfx->GetStrWidth)
        return;

    if (win->anim != NULL)
        win->anim(win, fb, an_speed);

    if (win->alpha <= 0 && win->anim == NULL)
        return;

    win->x = (fb->w - win->w) / 2;
    win->y = (fb->h - win->h) / 2;

    int alpha = win->alpha;
    uint32_t dark_green_bg = 0xFF1B4D3E;

    // Rounded window + gradient
    for (int py = 0; py < win->h; py++) {
        int start_offset = 0;
        int end_offset = win->w;
        int current_y = win->y + py;

        if (py < win->r) {
            int dy = win->r - py;
            int dx = win->r -
                (int)(sqrt((double)(win->r * win->r - dy * dy)) + 0.5);

            start_offset = dx;
            end_offset = win->w - dx;
        }
        else if (py >= win->h - win->r) {
            int dy = py - (win->h - win->r);
            int dx = win->r -
                (int)(sqrt((double)(win->r * win->r - dy * dy)) + 0.5);

            start_offset = dx;
            end_offset = win->w - dx;
        }

        uint32_t grad_color =
            symb->plt->ApplyGrad(dark_green_bg, py, 0, win->h);

        for (int px = start_offset; px < end_offset; px++) {
            int target_x = win->x + px;

            if (target_x >= 0 && target_x < fb->w &&
                current_y >= 0 && current_y < fb->h) {

                uint32_t final_color =
                    symb->plt->Blend(
                        fb, target_x, current_y,
                        grad_color, alpha
                    );

                if (fb->bk_bf)
                    fb->bk_bf[current_y * (fb->stride / 4) + target_x]
                        = final_color;
            }
        }
    }

    // Bottom panel
    int panel_h = 50;
    int border_inset = 2;

    int panel_x = win->x + border_inset;
    int panel_w = win->w - (border_inset * 2);
    int panel_y = win->y + win->h - panel_h - border_inset;

    int r = win->r - border_inset;
    if (r < 0)
        r = 0;

    for (int py = 0; py < panel_h; py++) {
        int start_offset = 0;
        int end_offset = panel_w;
        int current_y = panel_y + py;

        if (r > 0 && py >= panel_h - r) {
            int dy = py - (panel_h - r);
            int dx = r -
                (int)(sqrt((double)(r * r - dy * dy)) + 0.5);

            start_offset = dx;
            end_offset = panel_w - dx;
        }

        for (int px = start_offset; px < end_offset; px++) {
            int target_x = panel_x + px;

            if (target_x >= 0 && target_x < fb->w &&
                current_y >= 0 && current_y < fb->h) {

                uint32_t final_color =
                    symb->plt->Blend(
                        fb, target_x, current_y,
                        0xFF808080, alpha
                    );

                if (fb->bk_bf)
                    fb->bk_bf[current_y * (fb->stride / 4) + target_x]
                        = final_color;
            }
        }
    }

    // Buttons
    int btn_w = 180;
    int btn_h = 50;
    int spacing = 20;

    int total_w = (win->btn_count * btn_w) +
                  ((win->btn_count > 0 ? win->btn_count - 1 : 0) * spacing);

    int start_x = win->x + (win->w - total_w) / 2;

    for (int i = 0; i < win->btn_count; i++) {
        int btn_x = start_x + (i * (btn_w + spacing));
        int btn_y = panel_y + (panel_h - btn_h) / 2;

        symb->wid->SetBtnEnv(
            &win->buttons[i],
            btn_x, btn_y,
            btn_w, btn_h,
            0xFFD3D3D3,
            0xFFE6A23C,
            win->labels[i]
        );

        for (int by = 0; by < btn_h; by++) {
            for (int bx = 0; bx < btn_w; bx++) {
                int target_x = btn_x + bx;
                int current_y = btn_y + by;

                if (target_x >= 0 && target_x < fb->w &&
                    current_y >= 0 && current_y < fb->h) {

                    uint32_t final_btn_color =
                        symb->plt->Blend(
                            fb, target_x, current_y,
                            0xFFD3D3D3, alpha
                        );

                    if (fb->bk_bf)
                        fb->bk_bf[current_y * (fb->stride / 4) + target_x]
                            = final_btn_color;
                }
            }
        }
    }

    // Text colors
    uint32_t text_color =
        (0xFF << 24) |
        ((255 * alpha / 255) << 16) |
        ((255 * alpha / 255) << 8) |
        (255 * alpha / 255);

    uint32_t green_line =
        (0xFF << 24) |
        (0 << 16) |
        ((255 * alpha / 255) << 8) |
        0;

    // Window title
    if (face && win->title) {
        symb->gfx->Text(
            fb, face, win->title,
            win->x + 15, win->y + 25,
            text_color
        );
    }

    // Separator
    symb->gfx->DrawLine(
        fb,
        win->x + 10,
        win->y + 40,
        win->x + win->w - 10,
        win->y + 40,
        green_line
    );

    // Window message
    if (face && win->message) {
        int cur_x = win->x + 10;
        int cur_y = win->y + 60;

        char buffer[256];

        strncpy(buffer, win->message, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        char *token = strtok(buffer, " ");

        while (token != NULL) {
            int tw = symb->gfx->GetStrWidth(face, token);

            if (cur_x + tw > win->x + win->w - 10) {
                cur_x = win->x + 10;
                cur_y += 25;
            }

            symb->gfx->Text(
                fb, face, token,
                cur_x, cur_y,
                text_color
            );

            cur_x += tw + 10;
            token = strtok(NULL, " ");
        }
    }

    // Button labels
    if (face) {
        for (int i = 0; i < win->btn_count; i++) {
            if (!win->labels[i])
                continue;

            int btn_x = start_x + (i * (btn_w + spacing));
            int btn_y = panel_y + (panel_h - btn_h) / 2;

            int tw = symb->gfx->GetStrWidth(face, win->labels[i]);

            uint32_t btn_text_c =
                (0xFF << 24) |
                ((30 * alpha / 255) << 16) |
                ((30 * alpha / 255) << 8) |
                (30 * alpha / 255);

            symb->gfx->Text(
                fb, face, win->labels[i],
                btn_x + (btn_w - tw) / 2,
                btn_y + (btn_h / 2) - 6,
                btn_text_c
            );
        }
    }
}

// Fade animations
void AutumnAPI_Anim_FadeIn(void *win_ptr, void *fb_ptr, int speed)
{
    (void)fb_ptr;

    Window *win = (Window*)win_ptr;
    if (!win)
        return;

    if (win->alpha < 255) {
        win->alpha += speed;

        if (win->alpha > 255)
            win->alpha = 255;
    }
    else {
        win->anim = NULL;
    }
}

void AutumnAPI_Anim_FadeOut(void *win_ptr, void *fb_ptr, int speed)
{
    (void)fb_ptr;

    Window *win = (Window*)win_ptr;
    if (!win)
        return;

    if (win->alpha > 0) {
        win->alpha -= speed;

        if (win->alpha < 0)
            win->alpha = 0;
    }
    else {
        win->anim = NULL;
    }
}
