#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "atmhal.h"

void mkdir_data(void) {
        mkdir("/tmp/autumnsys", 0777);
        mkdir("/tmp/autumnsys/battery", 0777);
        mkdir("/tmp/autumnsys/uptime", 0777);
        mkdir("/tmp/autumnsys/power", 0777);
}

void check_power_status(void) {
	FILE *fp_pwr = fopen("/tmp/itstimetopoweroff", "r");
	if (fp_pwr != NULL) {
		fclose(fp_pwr);
		remove("/tmp/itstimetopoweroff");
		atmsys_pwroff();

	}

	FILE *fp_reb = fopen("/tmp/itstimetoreboot", "r");
	if (fp_reb != NULL) {
		fclose(fp_reb);
		remove("/tmp/itstimetoreboot");
		atmsys_reboot();
	}
}

void update_system_status(void) {
	int bat = atmsys_battery_perc();
	FILE *fp_bat = fopen("/tmp/autumnbat0", "w");
	if (fp_bat != NULL)  {
		fprintf(fp_bat, "%d", bat);
		fclose(fp_bat);
	}

	long up = atmsys_uptime();
    FILE *fp_up = fopen("/tmp/autumnuptime0", "w");
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
	atmsys_modemhdinit();
	while (1) {
		update_system_status();
		check_power_status();
		usleep(500000);
	}
	return 0;
}
