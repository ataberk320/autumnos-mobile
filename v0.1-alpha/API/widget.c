#include <string.h>
#include "button.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "table.h"
#include "timer.h"
extern GFX_API* gfx;

void Button_Set(ButtonWidget* btn, int x, int y, int w, int h, unsigned int n_color, unsigned int p_color, const char* txt) {
	btn->x = x;
	btn->y = y;
	btn->width = w;
	btn->height = h;
	btn->normal_color = n_color;
	btn->press_color = p_color;
	btn->is_pressed = 0;
	strncpy(btn->text, txt, sizeof(btn->text) - 1);
}

int ChkTouchEv(ButtonWidget* btn, int touch_x, int touch_y, int is_touched) {
    int inside = (touch_x >= btn->x && touch_x <= (btn->x + btn->width) &&
                  touch_y >= btn->y && touch_y <= (btn->y + btn->height));

    if (is_touched && inside) {
        if (btn->is_pressed == 0) {
            btn->is_pressed = 1;
            
            if (btn->OnClick != NULL) {
                btn->OnClick(btn);
            }
            
            return 1;
        }
        return 0;
    }

    if (!is_touched || !inside) {
        if (btn->is_pressed == 1) {
            btn->is_pressed = 0;
        }
    }

    return 0;
}

void CreateButton(FbDev* screen, FT_Face face, ButtonWidget* btn) {
    	unsigned int current_color = btn->is_pressed ? btn->press_color : btn->normal_color;
    	gfx->DrawButton(screen, face, btn->x, btn->y, btn->width, btn->height, 15, current_color, btn->text, 0x00000000);
}



void CreateElapsedTimer(FbDev* screen, FT_Face face, TimerWidget* tw) {
	char time_str[16];
	snprintf(time_str, sizeof(time_str), "%02d:%02d / %02d:%02d", tw->elapsed / 60, tw->elapsed % 60, tw->total / 60, tw->total % 60);
	gfx->Text(screen, face, time_str, tw->x, tw->y, tw->color);
}
