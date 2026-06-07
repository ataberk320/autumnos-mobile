#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include "AutumnAPI.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "AutumnGestureArg.h"
#include <time.h>
#include "AutumnPwrBtnArg.h"
#include <linux/rfkill.h>
#define STATUS_SOCK_PATH "/tmp/autumn_sock/status.sock"
#define AUTUMN_IPC_PATH "/tmp/autumn_conf/AutumnCore0.sock"
#define MPR121_ADDR 0x5A
static unsigned char *video_raw_pixels = NULL;
ButtonTracker power_track = {1, {0,0}};
typedef int (*crt_srv_t)(const char*);
typedef void (*send_cmd_t)(const char*, const char*);

int AutumnAPI_System(const char *cmd) {
	if (cmd == NULL) return -1;
	
	void *handle = dlopen("/usr/lib/libatmsock.so", RTLD_LAZY);
	if (!handle) return -1;
	
	send_cmd_t atm_send_cmd = (send_cmd_t)dlsym(handle, "AutumnAPI_Socket_SendCmd");
    
    	if (atm_send_cmd) {
        	atm_send_cmd(AUTUMN_IPC_PATH, cmd);
    	}

    	dlclose(handle);
    	return 0;
}

uint32_t AutumnAPI_Get_TickMS() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}


void AutumnAPI_Request_PowerOff(void) {
	FILE *fp = fopen("/tmp/autumnsys/power/itstimetopoweroff", "w");
	if (fp) {
		fprintf(fp, "request=1\n");
		fclose(fp);
	}
}


int AutumnAPI_MPR_Read() {
	int file;
	const char *bus = "/dev/i2c-0";
	unsigned char reg = 0x00;
	unsigned char data[2];

	if ((file = open(bus, O_RDWR)) < 0) return -1;
	ioctl(file, I2C_SLAVE, MPR121_ADDR);

	write(file, &reg, 1);
	read(file, data, 2);
	
	int touched = ((data[1] & 0x0F) << 8) | data[0];

	close(file);
	return touched;
}

AutumnGestureEvent AutumnAPI_Detect_Gesture() {
	time_t now = time(NULL);
	static uint32_t start_time = 0;
	static int start_pin = -1;
	static int last_state = 0;	
	AutumnGestureEvent event = {AUTUMN_G_NONE, 0, 0};
	int current_state = AutumnAPI_MPR_Read();
	
	if (current_state > 0 && last_state == 0) {
		start_time = now;
		for (int i = 0; i < 12; i++) {
			if (current_state & (1 << i)) {
				start_pin = i;
				break;
			}
		}
	}
	else if (current_state == 0 && last_state > 0) {
		uint32_t duration = now - start_time;
		event.duration = duration;
		
		if (duration > 800 && (last_state & (1 << start_pin))) {
			event.type = AUTUMN_G_LONG_PRESS;
		}

		else if (duration < 500) {
			if (start_pin == 2 && (last_state & (1 << 0))) {
				event.type = AUTUMN_G_S_TO_L;
			}
			else if (start_pin == 3 && (last_state & (1 << 4))) {
				event.type = AUTUMN_G_TAP;
			}
			else {
				event.type = AUTUMN_G_TAP;
			}
		}

		start_pin = -1;
		start_time = 0;
	}
	last_state = current_state;
	return event;
}

int AutumnAPI_Read_Power_Button_Status() {
	char val;
	int fd = open("/sys/class/gpio/gpio200/value", O_RDONLY);
	read(fd, &val, 1);
	close(fd);
	int current_state = (val == '0');
	
	if (current_state && power_track.last_state == 0) {
		clock_gettime(CLOCK_MONOTONIC, &power_track.press_start);
		power_track.last_state = 1;
	}
	else if (current_state && power_track.last_state == 1) {
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		long elapsed = now.tv_sec - power_track.press_start.tv_sec;
		if (elapsed >= 10) return 2;
		if (elapsed >= 5) return 1;
	}
	else if(!current_state) {
		power_track.last_state = 0;
	}
	return 0;
}

int AutumnAPI_Show_Toast(const char *msg) {
	void *handle = dlopen("/usr/lib/libatmsock.so", RTLD_LAZY);
	if (!handle) {
		return -1;
	}
	crt_srv_t show = (crt_srv_t)dlsym(handle, "AutumnAPI_Socket_CreateServer");
	if (!show) {
		dlclose(handle);
		return -1;
	}
	int result = show(msg);
	dlclose(handle);
	return result;
}

void AutumnAPI_Read_Mouse(int fd, MouseData *data) {
    char buffer[64];
    ssize_t n;
    
    while ((n = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        int dx = 0, dy = 0, pr = 0;
        
        if (sscanf(buffer, "%d %d %d", &dx, &dy, &pr) == 3) {
            data->x = dx;
            data->y = dy;
            data->pressed = pr;

            if(data->x < 0) data->x = 0;
            if(data->y < 0) data->y = 0;
            if(data->x >= 480) data->x = 479;
            if(data->y >= 800) data->y = 799;
        }
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
    FILE *fp = fopen("/tmp/autumnsys/memory/autumnram0", "r");
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

void AutumnAPI_Play_Audio(const char* path) {
	char *argv[] = { (char*)"/usr/bin/mpg123", (char*)"-q", (char*)path, NULL };
	pid_t pid = fork();
	if (pid == 0) {
		execv(argv[0], argv);
		exit(0);
	}
}
