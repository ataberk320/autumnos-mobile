#define _DEFAULT_SOURCE

#include "rfkill.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/rfkill.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "io.h"
#define RFKSYSFS "/sys/class/rfkill"
#define RFKDEV "/dev/rfkill"

int _AutumnSys_rfkilldevlist(rfkillDev *devs, size_t capacity, size_t *count) {
	if (!count || (!devs && capacity)) {
		errno = EINVAL;
		return -1;
	}

	*count = 0;

	DIR *dir = opendir(RFKSYSFS);
	if (!dir) return -1;

	struct dirent *entry;
	size_t total = 0;

	while ((entry = readdir(dir)) != NULL) {
		unsigned int index;
		char extra;

		if (sscanf(entry->d_name, "rfkill%u%c", &index, &extra) != 1) continue;

		char path[256];
        	char name[64];
        	char type[32];
        	int soft, hard;
        	FILE *file;

		snprintf(path, sizeof(path), RFKSYSFS "/%s/name", entry->d_name);
		file = fopen(path, "r");
		if (!file) continue;

		if (!fgets(name, sizeof(name), file)) {
			fclose(file);
			continue;
		}

		fclose(file);
		name [strcspn(name, "\r\n")] = '\0';

		snprintf(path, sizeof(path), RFKSYSFS "/%s/type", entry->d_name);

		file = fopen(path, "r");
		if (!file) continue;

		if (!fgets(type, sizeof(type), file)) {
			fclose(file);
			continue;
		}

		fclose(file);

		type[strcspn(type, "\r\n")] = '\0';

		snprintf(path, sizeof(path), RFKSYSFS "/%s/soft, entry->d_name);

		file = fopen(path, "r");

		if (!file)
			continue;

		if (fscanf(file, "%d", &hard) != 1) {
			fclose(file);
			continue;
		}

		fclose(file);
		if ((soft != 0 && soft != 1) ||
            	   (hard != 0 && hard != 1)) continue;

        	if (total < capacity) {
            		rfkillDev *device = &devices[total];

            		device->index = index;
            		device->type = RFKILL_TYPE_ALL;
            		device->soft_blocked = (uint8_t)soft;
            		device->hard_blocked = (uint8_t)hard;

            		snprintf(device->name, sizeof(device->name),
                     	"%s", name);

            		if (strcmp(type, "wlan") == 0)
                		device->type = RFKILL_TYPE_WLAN;
            		else if (strcmp(type, "bluetooth") == 0)
                		device->type = RFKILL_TYPE_BLUETOOTH;
            		else if (strcmp(type, "wwan") == 0)
                		device->type = RFKILL_TYPE_WWAN;
            		else if (strcmp(type, "wimax") == 0)
                		device->type = RFKILL_TYPE_WIMAX;
            		else if (strcmp(type, "gps") == 0)
                		device->type = RFKILL_TYPE_GPS;
            		else if (strcmp(type, "fm") == 0)
                		device->type = RFKILL_TYPE_FM;
            		else if (strcmp(type, "nfc") == 0)
                		device->type = RFKILL_TYPE_NFC;
            		else if (strcmp(type, "uwb") == 0)
                		device->type = RFKILL_TYPE_UWB;
        	}

        	total++;
    	}

    	closedir(dir);
    	*count = total;

    	if (total > capacity) {
        	errno = ENOSPC;
        	return -1;
    	}

    	return 0;
}

int _AutumnSys_rfkillsetblk(uint32_t index, int blocked) {
	if (blocked != 0 && blocked != 1) {
		errno = EINVAL;
		return -1;
	}

	int fd = _AutumnSys_ioOpen(RFKDEV, O_WRONLY, 0);

	if (fd < 0) return -1;

	struct rfkill_event event = {0};

	event.idx = index;
	event.op = RFKILL_OP_CHANGE;
	event.soft = (uint8_t)blocked;

	ssize_t result = _AutumnSys_ioWrite(fd, &event, sizeof(event));

	int saved_errno = errno;

	_AutumnSys_ioClose(fd);

	if (result != (ssize_t)sizeof(event)) {
        	errno = result < 0 ? saved_errno : EIO;
        	return -1;
    	}

    	return 0;
}
