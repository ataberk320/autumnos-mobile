#ifndef BUTTON_H
#define BUTTON_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include "libfbdev.h"
#include "FramebufferStruct.h"
#include "AutumnImage.h"

typedef struct ButtonWidget {
    int x, y;
    int width, height;
    unsigned int normal_color;
    unsigned int press_color;
    char text[64];
    int is_pressed;
    void (*OnClick)(struct ButtonWidget*);
    AutumnImage* icon;
} ButtonWidget;

#endif
