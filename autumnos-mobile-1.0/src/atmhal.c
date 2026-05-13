#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/statvfs.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/reboot.h>
#include "atmhal.h"
#include "AutumnVideoArg.h"
#include <linux/input.h>
#include <linux/input-event-codes.h>
#define PWRBT_GPIO "117"
#define MODEM_RST_PIN "118"
int cam_fd = -1;
static int input_fd = -1;
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
	if (volume > 50) volume = 50;
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

int  atmsys_camera_init(void) {
	cam_fd = open("/dev/video0", O_RDWR | O_NONBLOCK);
	if (cam_fd < 0) return -1;

	struct v4l2_capability cap;
	if (ioctl(cam_fd, VIDIOC_QUERYCAP, &cap) < 0) return -2;
	
	struct v4l2_format fmt = {0};
    	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    	fmt.fmt.pix.width = 320;
    	fmt.fmt.pix.height = 240;
    	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG; 
    	fmt.fmt.pix.field = V4L2_FIELD_NONE;

    	if (ioctl(cam_fd, VIDIOC_S_FMT, &fmt) < 0) {
       		 return -3;
    	}
	return 0;
} 

void atmsys_indev_init(const char *suggested_path) {
	input_fd = open(suggested_path, O_RDONLY | O_NONBLOCK);
	if (input_fd >= 0) {
		unsigned long abs_bits[NBITS(ABS_MAX)];
		
		if (ioctl(input_fd, EVIOCGBIT(EV_ABS, sizeof(abs_bits)), abs_bits) >= 0) {
			if (TEST_BIT(ABS_X, abs_bits) && TEST_BIT(ABS_Y, abs_bits)) {
				printf("[AUTUMNOS]: Touchsreen hardware detected");
				return;
			}
		}
		close(input_fd);
		input_fd = (open("/dev/input/mouse0", O_RDONLY | O_NONBLOCK));
	}
}

void atmsys_indev_type(void) {
	int check_fd = open("/dev/input/event1", O_RDONLY | O_NONBLOCK);
	int is_touch = 0;
	if (check_fd >= 0) {
		unsigned long abs_bits[NBITS(ABS_MAX)];
		if (ioctl(check_fd, EVIOCGBIT(EV_ABS, sizeof(abs_bits)), abs_bits) >= 0) {
            		if (TEST_BIT(ABS_X, abs_bits) && TEST_BIT(ABS_Y, abs_bits)) {
                	is_touch = 1;
            }
        }
        close(check_fd);
    }

    if (is_touch) {
	atmsys_indev_init("/dev/input/event1");
    }
    else {
	atmsys_indev_init("/dev/input/event0");
    }
}

void atmsys_gpio(const char *path, const char *value) {
	int fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, value, strlen(value));
		close(fd);
	}
}

void atmsys_convert_videofrm(AVFrame *pFrame, AVCodecContext *pCodecCtx, unsigned char *out_buffer, int target_width, int target_height) {
    static struct SwsContext *sws_ctx = NULL;
    static int last_w = 0, last_h = 0;
    static enum AVPixelFormat last_fmt = AV_PIX_FMT_NONE;

    if (sws_ctx == NULL || last_w != pCodecCtx->width || last_h != pCodecCtx->height || last_h != pCodecCtx->pix_fmt) {
        if (sws_ctx) sws_freeContext(sws_ctx);
        sws_ctx = sws_getContext(
            pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
            target_width, target_height, AV_PIX_FMT_RGB24,
            SWS_FAST_BILINEAR, NULL, NULL, NULL
        );
        if (!sws_ctx) return;
        last_w = pCodecCtx->width;
        last_h = pCodecCtx->height;
        last_fmt = pCodecCtx->pix_fmt;
    }
    uint8_t *dest[4] = { (uint8_t *)out_buffer, NULL, NULL, NULL };
    int dest_linesize[4] = { target_width * 3, 0, 0, 0 };
    sws_scale(sws_ctx,
              (const uint8_t * const *)pFrame->data, pFrame->linesize,
              0, pCodecCtx->height,
              dest, dest_linesize);
}

void atmsys_play_video_wrapper(void *arg) {
	VideoThreadArgs *args = (VideoThreadArgs *)arg;
	if (!args) return;
	atmsys_play_video(args->source, args->out_buffer,  args->start_sec);
	free(args->source);
	free(args);
	pthread_exit(NULL);
}

void atmsys_play_video(const char *source, unsigned char *final_out_buffer, int start_second) {
	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "ffmpeg -ss %d -re -i \"%s\" -f rawvideo -pix_fmt rgb24 -s 320x240 pipe:1 2>/dev/null", start_second, source);
	FILE *pipein = popen(cmd, "r");
	if (!pipein) return;
	size_t rgb_size = 320 * 240 * 3;
	while (fread(final_out_buffer, 1, rgb_size, pipein) == rgb_size) {
		usleep(25000);
	}
	pclose(pipein);
}

int atmsys_is_headphone(void) {
	if (input_fd < 0) return -1;
	unsigned long sw_bits[NBITS(SW_MAX)];
	if (ioctl(input_fd, EVIOCGSW(sizeof(sw_bits)), sw_bits) >= 0) {
		if (TEST_BIT(SW_HEADPHONE_INSERT, sw_bits)) {
			printf("A headphone device detected!");
			return 1;
		}
	}
	return 0;
}

//Power options
void atmsys_reboot(void) {
	sync();
	if (reboot(RB_AUTOBOOT) == -1) {
		printf("Reboot is failed");
	}
}

void atmsys_pwroff(void) {
	sync();
	if (reboot(RB_POWER_OFF) == -1) {
		printf("Shutdown is failed");
	}
}

int atmsys_pwrstat(void) {
	char val[2];
	int fd = open("/sys/class/gpio/gpio117/value", O_RDONLY);
	if (fd < 0) {
		return 0;
	}

	if (read(fd, val, 1) < 0) {
		close(fd);
		return 0;
	}

	close(fd);
	return (val[0] == '0') ? 1 : 0;

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
        
        	if (battery_percent > 100) battery_percent = 100;
        	if (battery_percent < 0) battery_percent = 0;
        }
	 else {
        	battery_percent = -1;
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
	char path[128];
	atmsys_gpio("/sys/class/gpio/export", MODEM_RST_PIN);
	usleep(100000);
	sprintf(path, "/sys/class/gpio/gpio%s/direction", MODEM_RST_PIN);
	atmsys_gpio(path, "out");
	sprintf(path, "/sys/class/gpio/gpio%s/value", MODEM_RST_PIN);
	atmsys_gpio(path, "0");
	usleep(200000);
	atmsys_gpio(path, "1");
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

int atmsys_is_sim_inserted(int fd) {
	char buffer[128];
	write (fd, "AT+CPIN?\r\n", 10);
	tcdrain(fd);
	usleep(200000);
	read(fd, buffer, sizeof(buffer));
	if (strstr(buffer, "READY")) {
		return 1;
	}
	else if (strstr(buffer, "SIM PIN")) {
		return 2;
	}
	else {
		return 0;
	}
}

void atmsys_get_sim_operator_name(int fd, char *provider_name, size_t max_len) {
	char buffer[128];
	memset(buffer, 0, sizeof(buffer));
	write(fd, "AT+COPS?\r\n", 10);
	usleep(100000);
        read(fd, buffer, sizeof(buffer));
	char *start = strchr(buffer, '\"');
   	if (start) {
        	start++;
        	char *end = strchr(start, '\"');
        if (end) {
            size_t len = end - start;
            if (len >= max_len) len = max_len - 1;
            strncpy(provider_name, start, len);
            provider_name[len] = '\0';
            return;
        }
    }
    
    
    strncpy(provider_name, "Servis yok - Yalnızca acil aramalar.", max_len);
}

void atmsys_modem_answer(int serial_fd) {
	write(serial_fd, "ATA\r\n", 5);
}

void atmsys_modem_reject(int serial_fd) {
	write(serial_fd, "ATH\r\n", 5);
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

long atmsys_get_used_RAM(void) {
	struct sysinfo s_info;
	if (sysinfo(&s_info) != 0) {
        	return -1; 
   	}
	unsigned long total_mb = (s_info.totalram * s_info.mem_unit) / (1024 * 1024);
   	unsigned long free_mb = (s_info.freeram * s_info.mem_unit) / (1024 * 1024);
    	unsigned long used_mb = total_mb - free_mb;
	return (long)used_mb;
}

long atmsys_get_free_disk_space(const char *path) {
	struct statvfs vfs;
	if(statvfs(path, &vfs) != 0) return -1;
	unsigned long free_bytes = vfs.f_bavail * vfs.f_frsize;
	return free_bytes/(1024*1024);
}
