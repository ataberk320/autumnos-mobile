#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THERM_DEV "/sys/class/thermal/thermal_zone0/temp"

static char current_dev[128] = THERM_DEV;

int SRV_thermDevInit(const char* path) {
	if (path != NULL && strlen(path) > 0) {
		strncpy(current_therm_path, custom_path, sizeof(current_therm_path) - 1);
    	}
    	return 0;
}

int SRV_thermGetStat() {
	FILE* fp = fopen(current_dev, "r");
	if (!fp) return -1;
	
	char buffer[16];
	int temp = -1;
	
	if (fgets(buffer, sizeof(buffer), fp) != NULL) {
		temp = atoi(buffer) / 1000;
	}

	fclose(fp);
	return temp;
}
