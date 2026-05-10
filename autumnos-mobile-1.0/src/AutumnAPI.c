#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include "AutumnAPI.h"
#include "AutumnVideoArg.h"
#include "atmhal.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "AutumnGestureArg.h"
#include <time.h>
#define STATUS_SOCK_PATH "/etc/autumn_conf/status.sock"
#define MS_SOCK_PATH "/etc/autumn_conf/mscoord.sock"
#define MPR121_ADDR 0x5A
static unsigned char *video_raw_pixels = NULL;
extern void atmsys_play_video_wrapper(void *arg);
extern void atmsys_play_video(const char *source, unsigned char *final_out_buffer, int start_second);

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

void AutumnAPI_Emergency_PowerOff() {
	system("poweroff -f");
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

int AutumnAPI_Show_Toast(const char *msg) {
	int sock = 0;
	struct sockaddr_un serv_addr;
	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		return -1;
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strncpy(serv_addr.sun_path, STATUS_SOCK_PATH, sizeof(serv_addr.sun_path) - 1);
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		close(sock);
		return -2;
	}
	if(write(sock, msg, strlen(msg)) < 0) {
		close(sock);
		return -3;
	}
	close(sock);
	return 0;
}

//void AutumnAPI_Error_Center() {
	//dummy: system("/usr/bin/com.autumnos.errcnt.atm &");
//}

int AutumnAPI_Set_MsSock() {
	int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, MS_SOCK_PATH, sizeof(addr.sun_path) - 1);
        unlink(MS_SOCK_PATH);
        bind(sock, (struct sockaddr*)&addr, sizeof(addr));
        fcntl(sock, F_SETFL, O_NONBLOCK);
        return sock;
}

void AutumnAPI_Read_Mouse(int sock, MouseData *data) {
        char buffer[64];
	int dx = 0, dy = 0, pr = 0;
	int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
	if (n > 0) {
		buffer[n] = '\0';
		if (sscanf(buffer, "%d %d %d", &dx, &dy, &pr) == 3) {
			data->x += dx;
			data->y += dy;
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

	VideoThreadArgs *args = (VideoThreadArgs *)malloc(sizeof(VideoThreadArgs));
	args->source = strdup(source);
	args->out_buffer = video_raw_pixels;
	args->start_sec = 0;
	pthread_t tid;
	pthread_create(&tid, NULL, (void *(*)(void *))atmsys_play_video_wrapper, args);
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

int AutumnAPI_List_SSID(char ssid_list[][33], int max_count) {
	FILE *fp = popen("iw dev wlan0 scan | grep 'SSID:'", "r");
	if (fp == NULL) return -1;
	char line[128];
	int count = 0;

	while (fgets(line, sizeof(line), fp) && count < max_count) {
		char *start = strstr(line, "SSID: ");
		if (start) {
			start += 6;
			start[strcspn(start, "\n")] = 0;
			if (strlen(start) > 0) {
				strncpy(ssid_list[count], start, 32);
				ssid_list[count][32] = '\0';
				count++;
			}
		}
	}
	pclose(fp);
	return count;
}

void AutumnAPI_Bluetooth(bool enable) {
	if (enable) {
		system("rfkill unblock bluetooth");
		system("hciconfig hci0 up > /dev/null 2>&1");
	}
	else {
		system("hciconfig hci0 down > /dev/null 2>&1");
		system("rfkill block bluetooth");
	}
}

void AutumnAPI_WiFi(bool enable) {
	if (enable) {
		system("rfkill unblock wifi");
		system("ifconfig wlan0 up > /dev/null 2>&1");
	}
	else {
		system("ifconfig wlan0 down > /dev/null 2>&1");
		system("rfkill block wifi");
	}
}

bool AutumnAPI_Connection(void) {
	char buffer[128];
	bool is_wifi_blk = false;
	bool is_bt_blk = false;
	
	FILE *wififp = popen("rfkill list wifi", "r");
    	if (wififp) {
        	while (fgets(buffer, sizeof(buffer), wififp)) {
            		if (strstr(buffer, "Soft blocked: yes")) {
                		is_wifi_blk = true;
                		break;
            		}
        	}
        	pclose(wififp);
    	}

    	FILE *btfp = popen("rfkill list bluetooth", "r");
    	if (btfp) {
        	while (fgets(buffer, sizeof(buffer), btfp)) {
            		if (strstr(buffer, "Soft blocked: yes")) {
                		is_bt_blk = true;
                		break;
            		}
        	}
        	pclose(btfp);
    	}	

	return (!is_wifi_blk && !is_bt_blk);
}
