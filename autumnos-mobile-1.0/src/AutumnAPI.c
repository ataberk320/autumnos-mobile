#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "AutumnAPI.h"

static unsigned char *video_raw_pixels = NULL;
extern void atmsys_play_video(const char *source, unsigned char *final_out_buffer, int start_second);
void AutumnAPI_Request_PowerOff(void) {
	FILE *fp = fopen("/tmp/autumnsys/power/itstimetopoweroff", "w");
	if (fp) {
		fprintf(fp, "request=1\n");
		fclose(fp);
	}
}

void AutumnAPI_Request_Reboot(void) {
	FILE *fp = fopen("/tmp/autumnsys/power/itstimetoreboot", "w");
	if (fp) {
		fprintf(fp, "request=1\n");
		fclose(fp);
	}
}

int AutumnAPI_Read_Battery_Level(void) {
    int bat = 0;
    FILE *fp = fopen("/tmp/autumnsys/battery/autumnbat0", "r");
    if (fp) {
        fscanf(fp, "%d", &bat);
        fclose(fp);
    }
    return bat;
}

long AutumnAPI_Read_Uptime(void) {
	long up = 0;
	FILE *fp = fopen("/tmp/autumnsys/uptime/autumnuptime0", "r");
	if (fp) {
		if (fscanf(fp, "%ld", &up) != 1) up = -1;
		fclose(fp);
	}
	return up;
}

long AutumnAPI_Read_Used_RAM(void) {
    long ram = 0;
    FILE *fp = fopen("/tmp/autumnsys/mem/autumnram0", "r");
    if (fp) {
        fscanf(fp, "%ld", &ram);
        fclose(fp);
    }
    return ram;
}

long AutumnAPI_Read_Free_Disk(void) {
    long disk = 0;
    FILE *fp = fopen("/tmp/autumnsys/storage/autumndisk0", "r");
    if (fp) {
        fscanf(fp, "%ld", &disk);
        fclose(fp);
    }
    return disk;
}

void AutumnAPI_Play_Video(const char* source) {
	if (!video_raw_pixels) {
		video_raw_pixels = (unsigned char *)malloc(320 * 240 * 3);
	}

	char *thread_source = strdup(source);

	pthread_t tid;
	pthread_create(&tid, NULL, (void *(*)(void *))atmsys_play_video, (void *)source);
    
        pthread_detach(tid);
}

int AutumnAPI_SIM_Status(void) {
	int status = 0;
	FILE *fp = fopen("/tmp/autumnsys/connection/autumnsim0", "r");
	if (fp) {
		if (fscanf(fp, "%d", &status) != 1) {
			status = 0;
		}
		fclose(fp);
	}
	return status;
}
