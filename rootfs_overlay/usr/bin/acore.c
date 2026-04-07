
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "atmhal.h"

void mkdir_data(void) {
	mkdir("/tmp/autumnsys", 0777);
    	mkdir("/tmp/autumnsys/battery", 0777);
    	mkdir("/tmp/autumnsys/uptime", 0777);
    	mkdir("/tmp/autumnsys/power", 0777);
}

void check_power_status(void) {
    char line[64];
    int req_val = -1;

    FILE *fp_pwr = fopen("/tmp/autumnsys/power/itstimetopoweroff", "r");
    if (fp_pwr != NULL) {
        while (fgets(line, sizeof(line), fp_pwr)) {
            if (sscanf(line, "request=%d", &req_val) == 1) {
                break;
            }
        }
        fclose(fp_pwr);
        remove("/tmp/autumnsys/power/itstimetopoweroff");

        if (req_val == 1) {
            atmsys_pwroff();
        }
    }

    req_val = -1;
    FILE *fp_reb = fopen("/tmp/autumnsys/power/itstimetoreboot", "r");
    if (fp_reb != NULL) {
        while (fgets(line, sizeof(line), fp_reb)) {
            if (sscanf(line, "request=%d", &req_val) == 1) {
                break;
            }
        }
        fclose(fp_reb);
        remove("/tmp/autumnsys/power/itstimetoreboot");

        if (req_val == 1) {
            atmsys_reboot();
        }
    }
}

void update_system_status(void) {
	int bat = atmsys_battery_perc();
	FILE *fp_bat = fopen("/tmp/autumnsys/battery/autumnbat0", "w");
	if (fp_bat != NULL)  {
		fprintf(fp_bat, "%d", bat);
		fclose(fp_bat);
	}

	long up = atmsys_uptime();
	FILE *fp_up = fopen("/tmp/autumnsys/uptime/autumnuptime0", "w");
	if (fp_up != NULL) {
		if (up == -1) {
			fprintf(fp_up, "HAL error");
		}
		else {
			fprintf(fp_up, "%ld", up);
		}
		fclose(fp_up);
}

int main(void) {
	mkdir_data();
	atmsys_safe_volume(65);
	atmsys_modemhdinit();
	while (1) {
		update_system_status();
		check_power_status();
		usleep(500000);
	}
	return 0;
}
