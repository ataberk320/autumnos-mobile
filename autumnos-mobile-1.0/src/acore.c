#include <fcntl.h>
#include <unistd.h>
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
	mkdir("/tmp/autumnsys/memory", 0777);
	mkdir("/tmp/autumnsys/storage", 0777);
	mkdir("/tmp/autumnsys/connection", 0777);
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

void update_system_status(int serial_fd) {
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

	long used_ram = atmsys_get_used_RAM();
	FILE *fp_ram = fopen("/tmp/autumnsys/memory/autumnram0", "w");
	if (fp_ram !=NULL) {
		fprintf(fp_ram, "%ld", used_ram);
		fclose(fp_ram);
	}

	long free_disk = atmsys_get_free_disk_space("/");
	FILE *fp_disk = fopen("/tmp/autumnsys/storage/autumndisk0", "w");
	if (fp_disk != NULL) {
		fprintf(fp_disk, "%ld", free_disk);
		fclose(fp_disk);
	}
	
	int status = atmsys_is_sim_inserted(serial_fd);
	FILE *fp = fopen("/tmp/autumnsys/connection/autumnsim0", "w");
	if (fp) {
		fprintf (fp, "%d", status);
		fclose(fp);
	}
}

int main(void) {
	mkdir_data();
	atmsys_safe_volume(65);
	atmsys_modemhdinit();
	int serial_fd = open("/dev/ttyS1", O_RDWR | O_NOCTTY);
	if (atmsys_camera_init() == 0) {
		//komut gelcek
	}
	else {
		//komut gelcek
	}

	while (1) {
		update_system_status(serial_fd);
		check_power_status();
		usleep(500000);
	}
	return 0;
}
