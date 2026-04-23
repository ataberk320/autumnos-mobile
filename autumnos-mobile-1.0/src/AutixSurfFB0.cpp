#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <linux/fb.h>
#include <cstring>
#include <stdint.h>


struct Surface {
    uint32_t width, height;
    int shm_fd;
    uint32_t *data;
};

class AutixSurfFB {
private:
    int fb_fd;
    uint32_t *fb_ptr;
    long screen_size;
    Surface app_surface;
    Surface ui_surface;

    uint32_t screen_w, screen_h;

public:
    bool init() {
        fb_fd = open("/dev/fb0", O_RDWR);
        if (fb_fd < 0) return false;

        struct fb_var_screeninfo vinfo;
        if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) < 0) return false;

        screen_w = vinfo.xres;
        screen_h = vinfo.yres;
        screen_size = screen_w * screen_h * 4; // ARGB8888 (4 byte)

        fb_ptr = (uint32_t*)mmap(0, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
        if (fb_ptr == MAP_FAILED) return false;

        return true;
    }

    bool connectSurface(const char *name, uint32_t w, uint32_t h, Surface &surf) {
        surf.width = w;
        surf.height = h;
        surf.shm_fd = shm_open(name, O_RDWR | O_CREAT, 0666);
        if (surf.shm_fd < 0) return false;

        ftruncate(surf.shm_fd, w * h * 4);
        surf.data = (uint32_t*)mmap(0, w * h * 4, PROT_READ | PROT_WRITE, MAP_SHARED, surf.shm_fd, 0);
        
        return surf.data != MAP_FAILED;
    }


    void run() {
        connectSurface("/autumn_app", 480, 768, app_surface);
        connectSurface("/autumn_ui", 480, 800, ui_surface);


        while (true) {
            usleep(16000); // ~60 FPS
        }
    }

    ~AutixSurfFB() {
        munmap(fb_ptr, screen_size);
        close(fb_fd);
    }
};

int fb_main() {
    AutixSurfFB sf;
    if (sf.init()) {
        sf.run();
    }
    return 0;
}
