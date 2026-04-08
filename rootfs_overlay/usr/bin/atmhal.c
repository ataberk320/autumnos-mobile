#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include "atmhal.h"


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

void atmsys_convert_videofrm(AVFrame *pFrame, AVCodecContext *pCodecCtx, unsigned char *out_buffer, int target_width, int target_height) {
    static struct SwsContext *sws_ctx = NULL;
    static int last_w = 0, last_h = 0;

    if (sws_ctx == NULL || last_w != pCodecCtx->width || last_h != pCodecCtx->height) {
        if (sws_ctx) sws_freeContext(sws_ctx);
        
        sws_ctx = sws_getContext(
            pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
            target_width, target_height, AV_PIX_FMT_RGB24,
            SWS_BILINEAR, NULL, NULL, NULL
        );
        
        last_w = pCodecCtx->width;
        last_h = pCodecCtx->height;
    }

    uint8_t *dest[4] = { (uint8_t *)out_buffer, NULL, NULL, NULL };
    int dest_linesize[4] = { target_width * 3, 0, 0, 0 };

    sws_scale(sws_ctx, 
              (const uint8_t * const *)pFrame->data, pFrame->linesize, 
              0, pCodecCtx->height, 
              dest, dest_linesize);
}

void atmsys_play_video(const char *source, unsigned char *final_out_buffer, int start_second) {
    char cmd[1024];
    
    snprintf(cmd, sizeof(cmd), 
             "ffmpeg -ss %d -re -i \"%s\" -f rawvideo -pix_fmt yuv420p -s 320x240 pipe:1 2>/dev/null", 
             start_second, source);

    FILE *pipein = popen(cmd, "r");
    if (!pipein) {
        return;
    }

    AVFrame *pFrame = av_frame_alloc();
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);
    pCodecCtx->width = 320;
    pCodecCtx->height = 240;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    size_t yuv_size = (320 * 240 * 3) / 2;
    unsigned char *raw_yuv = (unsigned char *)malloc(yuv_size);

    while (fread(raw_yuv, 1, yuv_size, pipein) == yuv_size) {
        
        av_image_fill_arrays(pFrame->data, pFrame->linesize, raw_yuv, 
                             AV_PIX_FMT_YUV420P, 320, 240, 1);

        atmsys_convert_videofrm(pFrame, pCodecCtx, final_out_buffer, 320, 240);

        usleep(33000); 
    }

    free(raw_yuv);
    av_frame_free(&pFrame);
    pclose(pipein);
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


