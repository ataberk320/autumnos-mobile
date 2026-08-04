#include <stdint.h>
#include <stddef.h>

//UPDATE: moved color format functions from FBDEV API to Color Format API.
uint32_t AutumnAPI_GetGradientColor(uint32_t base_color, int y, int start_y, int height) {
    int factor = ((y - start_y) * 40 / height) - 20;
    int r = (base_color >> 16) & 0xFF;
    int g = (base_color >> 8) & 0xFF;
    int b = base_color & 0xFF;
    r = (r + factor > 255) ? 255 : (r + factor < 0) ? 0 : r + factor;
    g = (g + factor > 255) ? 255 : (g + factor < 0) ? 0 : g + factor;
    b = (b + factor > 255) ? 255 : (b + factor < 0) ? 0 : b + factor;
    return (0xFF << 24) | (r << 16) | (g << 8) | b;
}
//new alpha blend function!
uint32_t AutumnAPI_AlBlend(FbDev *fb, int x, int y, uint32_t fg_color, int alpha) {
        if (alpha <= 0) {
                uint32_t* fb_pixels = (uint32_t*)fb->fb_ptr;
                return fb_pixels[y * fb->w + x];
        }

        if (alpha >= 255) return fg_color;

        if (x < 0 || x >= fb->w || y < 0 || y >= fb->h) return fg_color;

        uint32_t* fb_pixels = (uint32_t*)fb->fb_ptr;
        uint32_t bg_color = fb_pixels[y * fb->w + x];

        uint8_t bg_r = (bg_color >> 16) & 0xFF;
        uint8_t bg_g = (bg_color >> 8) & 0xFF;
        uint8_t bg_b = bg_color & 0xFF;

        uint8_t fg_r = (fg_color >> 16) & 0xFF;
        uint8_t fg_g = (fg_color >> 8) & 0xFF;
        uint8_t fg_b = fg_color & 0xFF;

        uint8_t r = (fg_r * alpha + bg_r * (255 - alpha)) / 255;
        uint8_t g = (fg_g * alpha + bg_g * (255 - alpha)) / 255;
        uint8_t b = (fg_b * alpha + bg_b * (255 - alpha)) / 255;

        return (0xFF000000) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}
