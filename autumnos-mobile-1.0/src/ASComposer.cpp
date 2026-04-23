#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static uint32_t non_gpf_buffer[480 * 800] = {0};

int main() {

    usleep(500000);
    int fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd < 0) {
        return 1;
    }

    int ui_fd = shm_open("/autumn_ui", O_RDWR | O_CREAT, 0666);
    int app_fd = shm_open("/autumn_app", O_RDWR | O_CREAT, 0666);

    uint32_t ui_len = 480 * 800 * 4;
    uint32_t app_len = 480 * 768 * 4;
    
    ftruncate(ui_fd, ui_len);
    ftruncate(app_fd, app_len);

    uint32_t* fb_ptr = (uint32_t*)mmap(0, ui_len, PROT_WRITE, MAP_SHARED, fb_fd, 0);
    uint32_t* ui_ptr = (uint32_t*)mmap(0, ui_len, PROT_READ | PROT_WRITE, MAP_SHARED, ui_fd, 0);
    uint32_t* app_ptr = (uint32_t*)mmap(0, app_len, PROT_READ | PROT_WRITE, MAP_SHARED, app_fd, 0);

    if (fb_ptr == MAP_FAILED) return 1;
    if (ui_ptr != MAP_FAILED) memset(ui_ptr, 0, ui_len);
    if (app_ptr != MAP_FAILED) memset(app_ptr, 0, app_len);
    uint32_t* back_buffer = (uint32_t*)malloc(ui_len);
    if (!back_buffer) return 1;

    const uint32_t offset = 15360;
    const uint32_t app_pixel_count = 480 * 768;


    while (1) {
        memcpy(back_buffer, ui_ptr, ui_len);

        if (app_ptr != non_gpf_buffer) {
            for (uint32_t i = 0; i < app_pixel_count; i++) {
                uint32_t pixel = app_ptr[i];
                if (pixel & 0xFF000000) {
                    back_buffer[i + offset] = pixel;
                }
            }
        }
        memcpy(fb_ptr, back_buffer, ui_len);
        usleep(33333);
    }

    free(back_buffer);
    return 0;
}
