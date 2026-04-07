#include <stdio.h>
#include <stdlib.h>
#include "AutumnAPI.h"

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

