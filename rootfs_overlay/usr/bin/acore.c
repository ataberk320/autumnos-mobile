#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "atmhal.h"

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
