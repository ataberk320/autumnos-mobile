#ifndef WINDOW_H
#define WINDOW_H

typedef struct {
    int x, y, w, h, r;
    uint32_t bg_color, border_color;
    char title[64];
    char message[256];
    ButtonWidget buttons[3];
    char *labels[3];
    int btn_count;

    int alpha;
    int step;
    void (*anim)(void* win_ptr, void* fb_ptr, int speed);
} Window;
#endif
