#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include "AutixSurf.hpp"
#include "cheaders/AutumnAPI.h"
#include "cheaders/AutumnAni.h"
#include "cheaders/AutumnAppSSTimer.h"
#include "cheaders/AtmDrv_G2D.h"

#define TERMINAL_AS_LIB
#include "AutumnCoreUtilsTerminal.c"
#include "AutumnCoreUtilsTerminal.h"

#define AUTUMN_IPC_PATH "/tmp/autumn_conf/AutumnCore0.sock"

uint32_t comp_screen_w = SCREEN_W;
uint32_t comp_screen_h = SCREEN_H;

uint32_t comp_ui_len = SCREEN_W * SCREEN_H * 4;
static int ipc_fd = -1;

void AutixSurfComp_DrawCursor(uint32_t* buffer, int x, int y) {
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			if (x + i < (int)comp_screen_w && y + j < (int)comp_screen_h) {
				buffer[(y + j) * comp_screen_w + (x + i)] = 0xFFFFFFFF;
			}
		}
	}
}

void compose(int app_index) {
	int mouse_fd = -1;
	int status = -1;
	usleep(500000);
	int fb_fd = open("/dev/fb0", O_RDWR);
	if (fb_fd < 0) {
		status = 1;
	}
	char app_shm_pt[64];
	snprintf(app_shm_pt, sizeof(app_shm_pt), "/autumn_app_%d", app_index);
	int ui_fd = shm_open("/autumn_ui", O_RDWR | O_CREAT, 0666);
	int app_fd = shm_open(app_shm_pt, O_RDWR | O_CREAT, 0666);

	uint32_t comp_app_len = comp_screen_w * (comp_screen_h - 32) * 4;
	ftruncate(ui_fd, comp_ui_len);
	ftruncate(app_fd, comp_app_len);

	fchmod(app_fd, 0666);
	uint32_t* ui_ptr = (uint32_t*)mmap(0, comp_ui_len, PROT_READ | PROT_WRITE, MAP_SHARED, ui_fd, 0);
	uint32_t* app_ptr = (uint32_t*)mmap(0, comp_app_len, PROT_READ | PROT_WRITE, MAP_SHARED, app_fd, 0);

	if (ui_ptr != MAP_FAILED) memset(ui_ptr, 0, comp_ui_len);
	uint32_t* back_buffer = (uint32_t*)malloc(comp_ui_len);
	if (!back_buffer) status = 1;

	const uint32_t offset = comp_screen_w * 32;
	const uint32_t app_pixel_count = comp_screen_w * (comp_screen_h - 32);
	MouseData ms = {0, 0, 0};
	fd_set readfds;
	struct timeval tv;

	while (1) {
		if (mouse_fd < 0) {
			mouse_fd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
			if (mouse_fd < 0) {
				usleep(50000);
				continue;
			}
		}

		FD_ZERO(&readfds);
		FD_SET(mouse_fd, &readfds);
		tv.tv_sec = 0;
		tv.tv_usec = 16000;
		int retval = select(mouse_fd + 1, &readfds, NULL, NULL, &tv);
		if (retval > 0 && FD_ISSET(mouse_fd, &readfds)) {
			AutumnAPI_Read_Mouse(mouse_fd, &ms);
		} else if (retval < 0) {
			close(mouse_fd);
			mouse_fd = -1;
			continue;
		}

		memcpy(back_buffer, ui_ptr, comp_ui_len);

		if (app_ptr != NULL) {
			for (uint32_t i = 0; i < app_pixel_count; i++) {
				uint32_t pixel = app_ptr[i]; // Uygulama pikseli (ARGB)
				uint8_t alpha = (pixel >> 24) & 0xFF; // Alpha kanalı

				if (alpha == 0) {
					continue; 
				} 
				else if (alpha == 255) {
					back_buffer[i + offset] = pixel;
				} 
				else {
					uint32_t bg_pixel = back_buffer[i + offset]; // Alttaki SysUI pikseli

					uint8_t rb = (bg_pixel >> 16) & 0xFF; uint8_t gb = (bg_pixel >> 8) & 0xFF; uint8_t bb = bg_pixel & 0xFF;
					uint8_t rf = (pixel >> 16) & 0xFF;    uint8_t gf = (pixel >> 8) & 0xFF;    uint8_t ff = pixel & 0xFF;

					uint8_t r = (rf * alpha + rb * (255 - alpha)) >> 8;
					uint8_t g = (gf * alpha + gb * (255 - alpha)) >> 8;
					uint8_t b = (ff * alpha + bb * (255 - alpha)) >> 8;

					back_buffer[i + offset] = (0xFF << 24) | (r << 16) | (g << 8) | b;
				}
			}
		}

		AutixSurfComp_DrawCursor(back_buffer, ms.x, ms.y);

		if (ipc_fd < 0) {
			ipc_fd = AutumnAPI_ConnSoc(AUTUMN_IPC_PATH);
		}

		if (ipc_fd >= 0) {
			int error = 0;
			socklen_t len = sizeof(error);
			if (getsockopt(ipc_fd, SOL_SOCKET, SO_ERROR, &error, &len) == 0 && error == 0) {
				ssize_t sent = write(ipc_fd, "render:card", 11);
				if (sent < 0) {
					Terminal_PutStr("\033[31mERROR: \033[0mSocket writing failed. Closing socket.\n");
					close(ipc_fd);
					ipc_fd = -1;
				}
			} else {
				close(ipc_fd);
				ipc_fd = -1;
			}
		}
	}
	free(back_buffer);
	if (ui_ptr != MAP_FAILED) munmap(ui_ptr, comp_ui_len);
	if (app_ptr != MAP_FAILED) munmap(app_ptr, comp_app_len);
	close(ui_fd);
	close(app_fd);
}

int main(int argc, char *argv[]) {
	int app_index = 0;
	if (argc > 1) {
		app_index = atoi(argv[1]);
	}
	compose(app_index);
	return 0;
}
