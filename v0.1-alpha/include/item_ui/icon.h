#ifndef ICON_H
#define ICON_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include "FramebufferStruct.h"
#include "AutumnImage.h"

typedef struct IconWidget {
  int x;
  int y;
  int w;
  int h;

  int imgW;
  int imgH;

  AutumnImage* image;
  char name[64];

  int isPressed;

  void (*OnClick)(struct IconWidget*);
} IconWidget;

void SetIcon(IconWidget* icon, int x, int y, int w, int h, AutumnImage* image, const char* name);
void DrawIcon(FbDev* screen, FT_Face face, IconWidget* icon);
int Icon_CheckTouch(IconWidget* icon, int touch_x, int touch_y, int is_touched);

#endif
