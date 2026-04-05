#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define PWRBT_GPIO "21"
#define MODEM_RST_PIN "118"

//System configuration functions
void atmsys_set_brightness(uint8_t level) {
	FILE *fp = fopen("/sys/class/backlight/backlight/brightness", "w");
	if (fp != NULL) {
		fprintf(fp, "%d", level);
		fclose(fp);
	}
}

void atmsys_volume_up(void) {
	system("amixer sset 'Master' 5%+ > /dev/null 2>&1");
}

void atmsys_volume_down(void) {
	 system("amixer sset 'Master' 5%- > /dev/null 2>&1"); 
}

void atmsys_safe_volume(uint8_t volume) {
	if (volume > 70) volume = 70;
	char cmd[128];
	sprintf(cmd, "amixer sset 'Master' %d%% > /dev/null 2>&1", volume);
	system(cmd);
}

void playaudio(const char *path) {
	if (access(path, F_OK) != 0) {
		return;
	}
	char cmd[512];
	snprintf(cmd, sizeof(cmd), "aplay -q -N %s &", path);
	int status = system(cmd);
	if (status == -1) printf("ALSA not initialized!");
}

//Power options
void atmsys_reboot(void) {
	sync();
	if (system("reboot") == -1) {
		system("busybox reboot");
	}
}

void atmsys_pwroff(void) {
	sync();
	if (system("poweroff") == -1) {
		system("busybox poweroff");
	}
}

bool atmsys_pwrstat(void) {
	char value;
	FILE *fp = fopen("/sys/class/gpio/gpio" PWRBT_GPIO "/value", "r");
	if (fp ==  NULL) return false;
	fread(&value, 1, 1, fp);
	fclose(fp);
	return (value == '0');
}

int atmsys_battery_perc(void)  {
	FILE *fp;
	char buffer[16];
	int battery_percent = 0;
	fp = fopen("/sys/class/power_supply/bat0/capacity", "r");
	if (fp ==NULL) {
		return -1;
	}
	if (fgets(buffer, sizeof(buffer), fp) != NULL) {
       		battery_percent = atoi(buffer);
        
        // Güvenlik sınırları
        	if (battery_percent > 100) battery_percent = 100;
        	if (battery_percent < 0) battery_percent = 0;
        }
	 else {
        	battery_percent = -1; // Okuma hatası
    	}

    fclose(fp);
    return battery_percent;
}

//Connection functions
void atmsys_flight(bool enable) {
	if (enable) {
		system("rfkill block all");
	}
	else {
		system("rfkill unblock all");
	}
}

void atmsys_modemhdinit(void) {
	system("echo " MODEM_RST_PIN " > /sys/class/gpio/export");
	usleep(100000);
	system ("echo out > /sys/class/gpio/gpio" MODEM_RST_PIN "/direction");
	system("echo 0 > /sys/class/gpio/gpio" MODEM_RST_PIN "/value");
	usleep(200000);
	system("echo 1 > /sys/class/gpio/gpio" MODEM_RST_PIN "/value");
	sleep(3);
}

int atmsys_modeminit(const char* port) {
    int fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) return -1;

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    tcsetattr(fd, TCSANOW, &options);
    
    return fd;
}

void atmsys_modem_software_init(int fd) {
    const char *cmds[] = {
        "AT\r\n",
        "ATE0\r\n",
        "AT+CFUN=1\r\n",
        "AT+CMGF=1\r\n",
        "AT+CNMI=2,1\r\n"
    };

    for(int i = 0; i < 5; i++) {
        write(fd, cmds[i], strlen(cmds[i]));
        tcdrain(fd);
        usleep(300000); 
    }
}

//Performance and storage status (Memory stat)
long atmsys_uptime(void) {
	struct  sysinfo s_info;
	int error = sysinfo(&s_info);
	if (error !=0) return -1;
	return s_info.uptime;
}

long atmsys_get_free_RAM(void) {
	struct sysinfo s_info;
	sysinfo(&s_info);
	return (s_info.freeram * s_info.mem_unit) / (1024 * 1024);
}

