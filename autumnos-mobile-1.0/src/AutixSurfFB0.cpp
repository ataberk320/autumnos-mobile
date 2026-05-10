#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <linux/fb.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <cstring>
#include <stdint.h>
#include <stdbool.h>
#include <vector>

struct Surface {
    char name[32];
    uint32_t width, height;
    int shm_fd;
    uint32_t *data;
    bool active;
    
};

class AutixSurfFB {
private:
    int fb_fd;
    uint32_t *fb_ptr;
    long screen_size;
    #define MAX_APPS 10
    Surface apps[MAX_APPS];
    int app_count = 0;
    int current_foreground_app = -1;
    int server_fd;
    uint32_t screen_w;
    uint32_t screen_h;
    const char *socket_path = "/etc/autumn_sock/asurf.sock";
    std::vector<Surface> app_list;
    bool setupSocket() {
	server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_fd < 0) return false;

	unlink(socket_path);
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
	if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) return false;
	listen (server_fd, 5);
	fcntl(server_fd, F_SETFL, O_NONBLOCK);
	return true;
   }
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

    bool registerNewApp(const char *name, uint32_t w, uint32_t h) { 
	Surface new_app;
	strncpy(new_app.name, name, 31);
	new_app.width = w;
	new_app.height = h;
	new_app.shm_fd = shm_open(name, O_RDWR | O_CREAT, 0666);
	if (new_app.shm_fd < 0) return false;
	
	ftruncate(new_app.shm_fd, w* h * 4);
	new_app.active = true;

	app_list.push_back(new_app);
	return true;
   }

    void run() {
	setupSocket();

        while (true) {
	    int client_fd = accept(server_fd, NULL, NULL);
	    if (client_fd > 0) {
		char buffer[64];
		int n = read(client_fd, buffer, sizeof(buffer));
		if (n>0) {
			if (strncmp(buffer, "DRAW:", 5) == 0) {
				char *target_name = buffer + 5;
				for (auto &s : app_list) {
					if (strcmp(s.name, target_name) == 0) {
						system("/usr/bin/ASComposer");
					}
				}
			}				
		}
		close(client_fd);
	    }
            usleep(16000);
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
