#include <iostream>
#include <fcntl.h>
#include <unistd.h>

extern int main();
extern int fb_main();


int detect_card() {
	int drm_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
	if (drm_fd >= 0) {
		close (drm_fd);
		return main();
	}

	int fb_fd = open("/dev/fb0", O_RDWR);
	if (fb_fd >= 0) {
		close(fb_fd);
		return fb_main();
	}
	return -1;
}

int detect() {
	return detect_card();
}
