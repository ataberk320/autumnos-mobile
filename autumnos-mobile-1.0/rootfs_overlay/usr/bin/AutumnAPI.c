#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AutumnAPI.h"
#include "atmhal.h"

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
	if (fp == NULL) return 0;
	fclose(fp);
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

void AutumnAPI_Play_Video(const char* source, int start_sec) {
    if (!video_raw_pixels) {
        video_raw_pixels = (unsigned char *)malloc(320 * 240 * 3);
    }

    
    VideoThreadArgs *args = (VideoThreadArgs *)malloc(sizeof(VideoThreadArgs));
    args->source = strdup(source);
    args->final_out_buffer = video_raw_pixels;
    args->start_second = start_sec;

    pthread_t tid;
    if (pthread_create(&tid, NULL, atmsys_play_video, (void *)args) == 0) {
        pthread_detach(tid);
    } else {
        free(args->source);
        free(args);
    }
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
