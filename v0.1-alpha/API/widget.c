#include <string.h>
#include "../include/item_ui/button.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "../include/SymbolTable/table.h"
#include "../include/item_ui/timer.h"
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

void Icon_Set(IconWidget* icon, int x, int y, int w, int h, AutumnImage* image, const char* name) {
	if (!icon) return;
	icon->x = x;
	icon->y = y;
	icon->w = w;
	icon->h = h;

	icon->image = image;

	icon->isPressed = 0; 
	icon->OnClick = NULL;
	if (name) {
		strncpy(icon->name, name, sizeof(icon->name) - 1 );
		icon->name[sizeof(icon->name) - 1] = '\0';
	} else {
		icon->name[0] = '\0'; }
}

int ChkIcTouchEv(IconWidget* icon, int touch_x, int touch_y, int is_touched) {
	if (!icon) return;
	int inside = touch_x >= icon->x && touch_x < icon->x + icon->width && touch_y >= icon->y && touch_y < icon->y + icon->height;

	if (isTouched && inside) {
		if (!icon->isPressed) {
			icon->isPressed = 1;
			if (icon->OnClick)
				icon->OnClick(icon);

			return 1;
		}
		return 0;
	}
	if (!is_touched || !inside) icon->is_pressed = 0; return 0;
}

void CreateIcon(FbDev* screen, FT_Face face, IconWidget* icon) {
	if (!screen || !icon || !gfx) return;

	if (icon->image) {
		int imageX = icon->x + (icon->w - icon->image->w) / 2;
		int imageY = icon->y;
		gfx->DrawImg(screen, icon->image, imageX, imageY);
	}
	if (icon->name[0] != '\0') { 
		int text_width = gfx->GetStrWidth( face, icon->name ); 
		int text_x = icon->x + (icon->width - text_width) / 2;
		int text_y = icon->y + (icon->image ? icon->image->height : 0) + 8;
		gfx->Text( screen, face, icon->name, text_x, text_y, 0xFFFFFFFF );
	}
}

void Grid_Init(GridWidget* grid, int x, int y, int width, int height, int columns) { 
	if (!grid) return;
	if (columns < 1) columns = 1;
	grid->x = x;
	grid->y = y;
	grid->width = width;
	grid->height = height;
	grid->columns = columns;
	grid->spacing_x = 8;
	grid->spacing_y = 8;
	grid->item_count = 0;
	
	for (int i = 0; i < GRID_MAX_ITEMS; i++) grid->items[i] = NULL;
} 

int Grid_Add( GridWidget* grid, IconWidget* icon ) 
{ 
	if (!grid || !icon) return -1;
	if (grid->item_count >= GRID_MAX_ITEMS) return -1;
	
	grid->items[grid->item_count] = icon;
	grid->item_count++; Grid_Layout(grid);
	
	return grid->item_count - 1;
} 

void Grid_Layout( GridWidget* grid ) {
	if (!grid) return;
	if (grid->columns <= 0) return;
	
	if (grid->item_count <= 0) return;
	int rows = (grid->item_count + grid->columns - 1) / grid->columns;
	
	int total_spacing_x = (grid->columns - 1) * grid->spacing_x;
	
	int total_spacing_y = (rows - 1) * grid->spacing_y;
	
	int cell_width = (grid->width - total_spacing_x) / grid->columns;
	int cell_height = (grid->height - total_spacing_y) / rows;
	
	for (int i = 0; i < grid->item_count; i++) { 
		IconWidget* icon = grid->items[i]; 
		
		if (!icon) continue; 
		
		int column = i % grid->columns; 
		
		int row = i / grid->columns; 
		icon->x = grid->x + column * (cell_width + grid->spacing_x);
		icon->y = grid->y + row * (cell_height + grid->spacing_y);
		icon->width = cell_width;
		icon->height = cell_height;
	} 
} 

void CreateGrid( FbDev* screen, FT_Face face, GridWidget* grid ) {
	if (!screen || !grid) return;
	
	for (int i = 0; i < grid->item_count; i++) 
	{ 
		if (!grid->items[i]) continue;
		CreateIcon( screen, face, grid->items[i] );
	}
}

int ChkGridTouchEv( GridWidget* grid, int touch_x, int touch_y, int is_touched ) 
{ 
	if (!grid) return 0;
	
	for (int i = 0; i < grid->item_count; i++) {
		if (!grid->items[i]) continue;
		if (ChkIconTouchEv( grid->items[i], touch_x, touch_y, is_touched)) { 
			return 1;
		}
	}
	return 0; 
}

void CreateElapsedTimer(FbDev* screen, FT_Face face, TimerWidget* tw) {
	char time_str[16];
	snprintf(time_str, sizeof(time_str), "%02d:%02d / %02d:%02d", tw->elapsed / 60, tw->elapsed % 60, tw->total / 60, tw->total % 60);
	gfx->Text(screen, face, time_str, tw->x, tw->y, tw->color);
}
