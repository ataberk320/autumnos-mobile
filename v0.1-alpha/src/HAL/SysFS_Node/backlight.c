#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define BKLDEV "/sys/class/backlight"
#define PATH_SIZE 4096

static char dev[PATH_SIZE];

int _AutumnSys_bklgdiscover(void) {
	DIR *dir = opendir(BKLDEV);
	if (!dir) return -errno;

	dev[0] = '\0';
	struct dirent *entry;

	while ((entry = readdir(dir))) {
		if (entry->d_name[0] == ".") continue;

		char path[PATH_SIZE];
		snprintf(device, sizeof(dev, "%s/%s, BKLDEV, entry->d_name);
		snprintf(path, sizeof(path), "%s/max_brightness", dev);
		
		FILE *f = fopen(path, "r");

		if (!f) continue;

		int max;
		int ok = fscanf(f, "%d", &max) == 1 && max > 0;
		fclose(f);
		if (!ok) continue;

		snprintf(path, sizeof(path), "%s/brightness", device);
        	f = fopen(path, "r");
        	if (!f) continue;

        	int brightness;
        	ok = fscanf(f, "%d", &brightness) == 1 &&
             	     brightness >= 0 && brightness <= max;
        	fclose(f);

		if (ok) {
            		closedir(dir);
            		return 0;
        	}
    	}

    	device[0] = '\0';
    	closedir(dir);
    	return -ENODEV;
}

int _AutumnSys_blkget(int *brightness) {
	if (!dev[0] || !brightness)
		return -EINVAL;
	
	char path[PATH_SIZE];
	snprintf(path, sizeof(path), "%s/brightness", dev);
	FILE *f = fopen(path, "r");
	if (!f) return -errno;

	int rc = fscanf(f, "%d", brightness) == 1 ? 0 : -EIO;
    	fclose(f);
    	return rc;
}


int _AutumnSys_blkgetmaxbrlev(int *max_brightness)
{
    if (!dev[0] || !max_brightness)
        return -EINVAL;

    char path[PATH_SIZE];
    snprintf(path, sizeof(path), "%s/max_brightness", dev);

    FILE *f = fopen(path, "r");
    if (!f)
        return -errno;

    int rc = fscanf(f, "%d", max_brightness) == 1 ? 0 : -EIO;
    fclose(f);
    return rc;
}
