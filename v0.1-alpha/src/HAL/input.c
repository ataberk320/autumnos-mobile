#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include "../../include/HAL/HAL_Symb.h"

int hal_msdevinit(void) {
	struct dirent *entry;
	DIR *dp = opendir("/dev/input");
	if (!dp) {
		perror("libmshal: Error: cannot open /dev/input directory");
		return -1;
	}

	while ((entry = readdir(dp))) {
		if (strncmp(entry->d_name, "event", 5) == 0) {
			char path[64];
			snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);
			
			int fd = open(path, O_RDONLY | O_NONBLOCK);
			if (fd < 0) continue;

			unsigned char evbits[EV_MAX / 8 + 1];
			memset(evbits, 0, sizeof(evbits));

			if (ioctl(fd, EVIOCGBIT(0, sizeof(evbits)), evbits) >= 0) {
				int has_abs = (evbits[EV_ABS / 8] & (1 << (EV_ABS % 8))) != 0;
				int has_rel = (evbits[EV_REL / 8] & (1 << (EV_REL % 8))) != 0;

				if (has_abs || has_rel) {
					char name[256] = "Unknown Device";
					ioctl(fd, EVIOCGNAME(sizeof(name) - 1), name);

					printf("libmshal: Success: Found Input Device: '%s' on %s\n", name, path);

					if (ioctl(fd, EVIOCGRAB, 1) < 0) {
						perror("libmshal: Error: EVIOCGRAB failed");
					}
					
					closedir(dp);
					return fd;
				}
			}
			close(fd);
		}
	}
	closedir(dp);
	fprintf(stderr, "No suitable touchscreen, touchpad or mouse detected in /dev/input!\n");
	return -1;
}

int hal_msdevread(int fd, struct input_event *ev) {
	if (fd < 0 || !ev) return -1;
	return read(fd, ev, sizeof(struct input_event));
}

void hal_msdevclose(int fd) {
	if (fd >= 0) {
		ioctl(fd, EVIOCGRAB, 0);
		close(fd);
	}
}

typedef struct {
    hal_dev_t base;
    int (*Init)(void);
    int (*Read)(int fd, struct input_event *ev);
    void (*Close)(int fd);
} input_dev_t;

static int input_bind_dev(const struct hal_mod_t* module, const char* name, struct hal_dev_t** device) {
    input_dev_t* dev = malloc(sizeof(input_dev_t));
    if (!dev) return -1;
    
    dev->base.common = (hal_mod_t*)module;
    dev->Init = hal_msdevinit;
    dev->Read = hal_msdevread;
    dev->Close = hal_msdevclose;
    
    *device = (hal_dev_t*)dev;
    return 0;
}

hal_mod_t HAL_MODULE_INFO_SYM = {
    .id = "input",
    .version = 1,
    .open = input_bind_dev
};
