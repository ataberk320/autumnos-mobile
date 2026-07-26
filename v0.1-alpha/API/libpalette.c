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
