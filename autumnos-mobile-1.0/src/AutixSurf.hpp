#ifndef AUTIXSURF_HPP
#define AUTIXSURF_HPP

#include <stdint.h>
#include <stddef.h>

extern int SCREEN_W;
extern int SCREEN_H;
extern uint32_t UI_LEN;

class AutixSurf {
private:
    int gfx_mode;
    int drm_fd;
    uint32_t crtc_id;
    uint32_t plane_app_id;
    uint32_t fb_id;
    uint32_t handle;

    uint32_t* ui_shm_ptr;
    uint32_t* fb_ptr;

public:
    AutixSurf();
    ~AutixSurf();

    void init_scrcard();
    void run_server();
    
    uint32_t* get_ui_shm_ptr() { return ui_shm_ptr; }
};

#endif // AUTIXSURF_HPP
