#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include "../../include/HAL/HAL_Symb.h"

int hal_usbpwrctl(const char* p_name, int enable) {
	char path[128];
	snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/power/control", p_name);

	int fd = open(path, O_WRONLY);
	if (fd < 0) return -1;

	const char *mode = enable ? "on" : "auto";
    	int ret = write(fd, mode, strlen(mode));
    	close(fd);

    	return (ret < 0) ? -1 : 0;
}

int hal_usbotg_tgl(int enable) {
	const char *otgp[] = {
		"/sys/devices/platform/soc/usbc/otg_power",
        	"/sys/kernel/debug/usb/ohci/power"
    	};

	for (int i = 0; i < 2; i++) {
        	int fd = open(otg_paths[i], O_WRONLY);
        	if (fd >= 0) {
            		const char *val = enable ? "1" : "0";
            		write(fd, val, strlen(val));
            		close(fd);
            		return 0;
        	}
    	}
    	return -1;
}

typedef struct {
    dev_t base;
    int (*PowerControl)(const char* port_name, int enable);
    int (*OtgToggle)(int enable);
} usb_dev_t;

static int usb_bind_dev(const struct hal_mod_t* module, const char* name, struct hal_dev_t** device) {
    usb_dev_t* dev = malloc(sizeof(usb_dev_t));
    if (!dev) return -1;
    
    dev->base.common = (hal_mod_t*)module;
    dev->PowerControl = hal_usbpwrctl;
    dev->OtgToggle = hal_usbotg_tgl;
    
    *device = (hal_dev_t*)dev;
    return 0;
}

hal_mod_t HAL_MODULE_INFO_SYM = {
    .id = "usb",
    .version = 1,
    .open = usb_bind_dev
};
