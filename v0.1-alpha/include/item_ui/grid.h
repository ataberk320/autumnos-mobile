#ifndef GRID_H
#define GRID_H

#include "FramebufferStruct.h"
#include "icon.h"

#define MAX_ITEM 64

typedef struct GridWidget {
  int x;
  int y;
  int w;
  int h;

  int columns;
  int rows;

  int cW;
  int cH;

  int spacingX;
  int spacingY;

  IconWidget* items[MAX_ITEM];
  int item_count;
}

void Init(GridWidget* grid, int x, int y, int w, int h, int columns);
int AddGrid(GridWidget* grid, IconWidget* icon);
void Layout(GridWidget* grid);
void DrawGrid(FbDev* screen, FT_FACE face, GridWidget* grid);
int ChkTouch(GridWidget* grid, int touch_x, int touch_y, int is_touched);

#endif
