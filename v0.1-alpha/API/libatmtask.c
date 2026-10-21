#include "../include/HAL/alog.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <dirent.h>

unsigned long AutumnAPI_GetTotRAM_MB(void) {
	struct sysinfo i;
	sysinfo(&i);
	return (i.totalram * i.mem_unit) / (1024 * 1024);
}

unsigned long AutumnAPI_GetFrRam_MB(void) {
	struct sysinfo i;
	sysinfo(&i);
	return (i.freeram * i.mem_unit) / (1024 * 1024);
}

int AutumnAPI_GetProc(void) {
	DIR *dir = opendir("/proc");
	if (!dir) return -1;

	struct dirent *entry;
	int count = 0;

	while ((entry = readdir(dir)) != NULL) {
        	if (entry->d_type == DT_DIR) {
            		int pid = atoi(entry->d_name);
            		if (pid > 0) {
                		count++;
            		}
        	}
    	}
    	closedir(dir);
    	return count;
}

int AutumnAPI_CheckProcess(const char *name) {
    DIR *dir = opendir("/proc");
    if (!dir) return 0;

    struct dirent *entry;
    int running = 0;

    while ((entry = readdir(dir)) != NULL) {
        int pid = atoi(entry->d_name);
        if (pid > 0) {
            char comm_path[64];
            char comm_name[256];
            snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);
            FILE *cf = fopen(comm_path, "r");
            if (cf != NULL) {
                if (fgets(comm_name, sizeof(comm_name), cf) != NULL) {
                    comm_name[strcspn(comm_name, "\n")] = 0;
                    if (strcmp(comm_name, name) == 0) {
                        running = 1;
                        fclose(cf);
                        break;
                    }
                }
                fclose(cf);
            }
        }
    }
    closedir(dir);
    return running;
}

