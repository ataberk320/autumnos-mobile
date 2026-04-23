#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <cstring>
#include <drm_fourcc.h>

struct  WindowSrf {
	uint32_t width, height, pitch, fb_id, handle;
	int shm_fd;
};

class AutixSurf {
private:
	int drm_fd;
	uint32_t crtc_id;
	WindowSrf app_window;
	uint32_t plane_app_id = 0;

public:
	bool init() {
		drm_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
		if (drm_fd < 0) return false;
		detectScreenAndPlanes();
		return true;
	}

	void detectScreenAndPlanes() {
		drmModeRes *resources = drmModeGetResources(drm_fd);
		crtc_id = resources->crtcs[0];
		drmModePlaneRes *plane_res = drmModeGetPlaneResources(drm_fd);
		if (plane_res->count_planes >= 2) {
			plane_app_id = plane_res->planes[1];
		}
		drmModeFreeResources(resources);
	}
	bool createSurface(const char *shm_name, uint32_t width, uint32_t height, WindowSrf &surface) {
        surface.width = width;
        surface.height = height;
        surface.pitch = width * 4;
        size_t size = surface.pitch * height;

        surface.shm_fd = shm_open(shm_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        ftruncate(surface.shm_fd, size);

        struct drm_mode_create_dumb creq = {height, width, 32, 0, 0, 0};
        ioctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
        surface.handle = creq.handle;

        uint32_t bo_handles[4] = {surface.handle, 0, 0, 0};
        uint32_t pitches[4] = {surface.pitch, 0, 0, 0};
        uint32_t offsets[4] = {0, 0, 0, 0};
        
        drmModeAddFB2(drm_fd, width, height, DRM_FORMAT_ARGB8888, 
                      bo_handles, pitches, offsets, &surface.fb_id, 0);

        return true;
    }

    void run() {
        createSurface("/autumn_app", 480, 760, app_window);
        
        std::cout << "Flinger is running..." << std::endl;
        while (true) {
            drmModeSetPlane(drm_fd, plane_app_id, crtc_id, app_window.fb_id, 0,
                            0, 50, app_window.width, app_window.height,
                            0, 0, app_window.width << 16, app_window.height << 16);
            usleep(16000); 
        }
    }
};

int main() {
	AutixSurf surfacef;
	if (surfacef.init()) {
		surfacef.run();
	}
	return 0;
}
