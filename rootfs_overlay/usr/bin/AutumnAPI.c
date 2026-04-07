#include <stdio.h>
#include <stdlib.h>
#include "AutumnAPI.h"

void AutumnAPI_Request_PowerOff(void) {
	FILE *fp = fopen("/tmp/itstimetopoweroff", "w");
	if (fp) {
		fprintf(fp, "1");
		fclose(fp);
	}
}

void AutumnAPI_Request_Reboot(void) {
	FILE *fp = fopen("/tmp/itstimetoreboot", "w");
	if (fp) {
		fprintf(fp, "1");
		fclose(fp);
	}
}

int AutumnAPI_Read_Battery_Level(void) {
    int bat = 0;
    FILE *fp = fopen("/tmp/autumnbat0", "r");
    if (fp) {
        fscanf(fp, "%d", &bat);
        fclose(fp);
    }
    return bat;
}
