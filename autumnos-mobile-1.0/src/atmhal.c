#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h> // don't forget to enable booleans
#include <stdio.h> // file operations
#include <string.h>
#include <stdlib.h> //system(); (if necessary)
#include <stdint.h>
#include "atmhal.h" //extern HAL definitions
#include <sys/sysinfo.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h> // for serial configuration
#include <sys/statvfs.h> // for Filesystem status
#include <linux/videodev2.h>
#include <sys/ioctl.h>
//#include <pthread.h>
#include <sys/reboot.h>
#include <sys/wait.h> //for PID
//#include "AutumnVideoArg.h" we have shared memory for it already.
#include "AutumnExternModem.h" // for SIM Slot definitions
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include <linux/rfkill.h> // for connection
#include <dirent.h>
#define PWRBT_GPIO "117" // if device is Allwinner
#define MODEM_RST_PIN "118" // if device is Allwinner
int cam_fd = -1; // Is camera device exists?
int input_fd = -1; // Is input device exists?
bool external = false; // External modem or SIM800L?
//HAL config struct
static struct {
	char soc_family[32];
	int max_width; //for camera
	int max_height;
	char pixel_format[16];
	char modem_port[64]; //for SIM800L
	int modem_baudrate;
	int screen_width;
	int screen_height;
	char backlight[128];
	char accelerator[64]; //G2D or DRM?
} AutumnHardwareID_t;

static void _hal_load_config(void) {
	//If !file, load default Allwinner config
	strcpy(AutumnHardwareID_t.soc_family, "allwinner");
	AutumnHardwareID_t.max_width = 640;
	AutumnHardwareID_t.max_height = 480;
	strcpy(AutumnHardwareID_t.pixel_format, "NV21");
	strcpy(AutumnHardwareID_t.modem_port, "/dev/ttyS1");
	AutumnHardwareID_t.modem_baudrate=115200;
	AutumnHardwareID_t.modem_baudrate = 115200;
    	AutumnHardwareID_t.screen_width = 480;
	AutumnHardwareID_t.screen_height = 800;
    	strcpy(AutumnHardwareID_t.accelerator, "/dev/g2d");
	FILE *file = fopen("/etc/DRV_CONF/allwinner/hw_extdrv.conf", "r"); // Our default HAL config
	if (!file) return;
	char line[128];
    	while (fgets(line, sizeof(line), file)) {
        	line[strcspn(line, "\r\n")] = 0;

        	char *key = strtok(line, "=");
        	char *value = strtok(NULL, "=");
		// Save new config data to booleans if config exists
        	if (key && value) {
            		if (strcmp(key, "soc_family") == 0)     strcpy(AutumnHardwareID_t.soc_family, value);
            		if (strcmp(key, "max_width") == 0)      AutumnHardwareID_t.max_width = atoi(value);
            		if (strcmp(key, "max_height") == 0)     AutumnHardwareID_t.max_height = atoi(value);
            		if (strcmp(key, "pixel_format") == 0)   strcpy(AutumnHardwareID_t.pixel_format, value);
            		if (strcmp(key, "modem_port") == 0)     strcpy(AutumnHardwareID_t.modem_port, value);
            		if (strcmp(key, "modem_baudrate") == 0) AutumnHardwareID_t.modem_baudrate = atoi(value);
            		if (strcmp(key, "screen_width") == 0)   AutumnHardwareID_t.screen_width = atoi(value);
            		if (strcmp(key, "screen_height") == 0)  AutumnHardwareID_t.screen_height = atoi(value);
			if (strcmp(key, "backlight_path") == 0) strcpy(AutumnHardwareID_t.backlight, value);
            		if (strcmp(key, "accelerator") == 0)    strcpy(AutumnHardwareID_t.accelerator, value);
        }
    }
    fclose(file);
}
static void slog(const char *content) {
	FILE *fd = fopen("/tmp/logs/atmsys.log", "a"); //"a" for prevent rewriting file
	if (fd != NULL) {
		fprintf(fd, "%s\n", content); // Write content to file
		fclose(fd);
	}
}

static void atmexecve(char *const argv[]) {
	pid_t pid = fork();
	if (pid == 0) { //If the process is not running
		int null_fd = open("/dev/null", O_WRONLY);
		if (null_fd >= 0) {
			dup2(null_fd, STDOUT_FILENO);
			dup2(null_fd, STDERR_FILENO);
			close(null_fd);
		}
		execv(argv[0], argv);
		_exit(127);
	}
	else if (pid > 0) { //Waiting PID while PID is > 0 (running)
		waitpid(pid, NULL, 0);
	}
}

//System configuration functions
void atmsys_set_brightness(uint8_t level) {
	FILE *fp = fopen(AutumnHardwareID_t.backlight, "w"); //Our backlight path definition on DRV_CONF
	if (fp != NULL) {
		fprintf(fp, "%u", (unsigned int)level);
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
	if (status == -1)  {
		slog("[AudioPlayer-Log]: ALSA not initialized!");
	}	
}

int  atmsys_camera_init(void) {
	cam_fd = open("/dev/video0", O_RDWR | O_NONBLOCK); //V4L device
	if (cam_fd < 0) {
		slog("[CamDev-Log]: Could not init camera device!");
		return -1;
	}
	struct v4l2_capability cap;
	if (ioctl(cam_fd, VIDIOC_QUERYCAP, &cap) < 0) return -2;
	
	struct v4l2_format fmt = {0};
    	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	// Set pixel format according to HAL config
	if (strcmp(AutumnHardwareID_t.pixel_format, "YUYV") == 0) {
        	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; // ARM
    	} 
	else if (strcmp(AutumnHardwareID_t.pixel_format, "MJPEG") == 0) {
        	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG; // Starfive
    	} 
	else if (strcmp(AutumnHardwareID_t.pixel_format, "NV21") == 0) {
        	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV21; // Allwinner
    	} 
	else {
        	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; // Default camera pixel format
    	}
    	fmt.fmt.pix.field = V4L2_FIELD_NONE;

    	if (ioctl(cam_fd, VIDIOC_S_FMT, &fmt) < 0) {
       		 return -3;
    	}
	return 0;
} 

void atmsys_indev_init(const char *suggested_path) {
	if (input_fd >= 0) {
		close(input_fd);
		input_fd = -1;
	}
	
	input_fd = open(suggested_path, O_RDONLY | O_NONBLOCK);

	if (input_fd >= 0) {
		slog("[Mouse-Log]: Input device initialized");
	}
	else {
		slog("[Mouse-Log]: Could not init input device");
	}
}

void atmsys_indev_type(void) {
	if (input_fd >= 0) return;
	int check_fd = open("/dev/input/event1", O_RDONLY | O_NONBLOCK);
	int is_touch = 0; // event1 is a touchscreen?
	if (check_fd >= 0) {
		unsigned long abs_bits[NBITS(ABS_MAX)];
		if (ioctl(check_fd, EVIOCGBIT(EV_ABS, sizeof(abs_bits)), abs_bits) >= 0) {
            		if (TEST_BIT(ABS_X, abs_bits) && TEST_BIT(ABS_Y, abs_bits)) {
                	is_touch = 1;
            }
        }
        close(check_fd);
    }
    // If event1 is not a touchscreen, switch to default mouse event
    if (is_touch) {
	atmsys_indev_init("/dev/input/event1");
    }
    else {
	atmsys_indev_init("/dev/input/event0");
    }
}

void atmsys_gpio(const char *path, const char *value) {
	int fd = open(path, O_WRONLY); // path->GPIO path, value->Value for writing to GPIO path (can be 1 or 0)
	if (fd >= 0) {
		write(fd, value, strlen(value));
		close(fd);
	}
}

void atmsys_convert_videofrm(AVFrame *pFrame, AVCodecContext *pCodecCtx, unsigned char *out_buffer, int target_width, int target_height) {
    static struct SwsContext *sws_ctx = NULL;
    static int last_w = 0, last_h = 0;
    static enum AVPixelFormat last_fmt = AV_PIX_FMT_NONE;

    if (sws_ctx == NULL || last_w != pCodecCtx->width || last_h != pCodecCtx->height || last_fmt != pCodecCtx->pix_fmt) {
        if (sws_ctx) sws_freeContext(sws_ctx);
        
        sws_ctx = sws_getContext(
            pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
            target_width, target_height, AV_PIX_FMT_RGB24,
            SWS_FAST_BILINEAR, NULL, NULL, NULL
        ); //For keep CPU stabile

        
        if (!sws_ctx) return;
        last_w = pCodecCtx->width; // Video width
        last_h = pCodecCtx->height; // Video height
        last_fmt = pCodecCtx->pix_fmt;
    }

    uint8_t *dest[4] = { (uint8_t *)out_buffer, NULL, NULL, NULL };
    int dest_linesize[4] = { target_width * 3, 0, 0, 0 };

    sws_scale(sws_ctx,
              (const uint8_t * const *)pFrame->data, pFrame->linesize,
              0, pCodecCtx->height,
              dest, dest_linesize);
}
// Decoding ffmpeg video pixels for AutumnCore
int atmsys_decode_videopix_libc(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, int videoStream, AVFrame *pFrame, unsigned char *out_buffer) {
	AVPacket packet;
	int frameFinished = 0;
	while (av_read_frame(pFormatCtx, &packet) >= 0) {
		if (packet.stream_index == videoStream) {
			int response = avcodec_send_packet(pCodecCtx, &packet);
			if (response < 0) {
				av_packet_unref(&packet);
				return 0;
			}

			response = avcodec_receive_frame(pCodecCtx, pFrame);
			if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
				av_packet_unref(&packet);
				continue;
			}
			else if (response < 0) {
				av_packet_unref(&packet);
				return 0;
			}
			atmsys_convert_videofrm(pFrame, pCodecCtx, out_buffer, 320, 240);
			av_packet_unref(&packet);
			return 1;
		}
		av_packet_unref(&packet);
	}
	return 0;
}


int atmsys_is_headphone(void) {
	if (input_fd < 0) return -1;
	unsigned long sw_bits[NBITS(SW_MAX)];
	if (ioctl(input_fd, EVIOCGSW(sizeof(sw_bits)), sw_bits) >= 0) {
		if (TEST_BIT(SW_HEADPHONE_INSERT, sw_bits)) {  // Listening headphone
			printf("A headphone device detected!");
			slog("[HeadphoneDev-Log]: A headphone device detected!");
			return 1;
		}
	}
	return 0;
}

//Power options
void atmsys_reboot(void) {
	sync(); //Save all data for data loss before reboot
	reboot(RB_AUTOBOOT);
}

void atmsys_pwroff(void) {
	sync();
	reboot(RB_POWER_OFF); 
}

void atmsys_emergency_pwroff(void) {
	char *const argv[] = {"/sbin/poweroff", "-f", NULL}; //Forcing system to power off
	execv(argv[0], argv);
    	system("echo o > /proc/sysrq-trigger"); //If the system frozen, force kernel to power off
	_exit(1);
}

int atmsys_pwrstat(void) {
	char val[2];
	int fd = open("/sys/class/gpio/gpio117/value", O_RDONLY); //Read power status from GPIO117
	if (fd < 0) {
		return 0;
	}

	if (read(fd, val, 1) < 0) {
		close(fd);
		return 0;
	}

	close(fd);
	return (val[0] == '0') ? 1 : 0; // Return to current status

}

int atmsys_battery_perc(void) {
    DIR *dir = opendir("/sys/class/power_supply/");
    if (!dir) {
        slog("[PowerHAL-Log]: Failed to open power supply class directory.");
        return -1;
    }

    struct dirent *entry;
    char target_path[128] = {0};

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char test_path[128];
        snprintf(test_path, sizeof(test_path), "/sys/class/power_supply/%s/capacity", entry->d_name);

        FILE *fp = fopen(test_path, "r");
        if (fp != NULL) {
            fclose(fp);
            strncpy(target_path, test_path, sizeof(target_path));
            break;
        }
    }
    closedir(dir);

    if (strlen(target_path) == 0) {
        slog("[PowerHAL-Log]: No valid battery hardware discovered via automatic scanning.");
        return -1;
    }

    FILE *fp = fopen(target_path, "r");
    if (fp == NULL) {
        slog("[PowerHAL-Log]: Failed to open discovered battery capacity file.");
        return -1;
    }

    int percentage = 0;
    if (fscanf(fp, "%d", &percentage) != 1) {
        slog("[PowerHAL-Log]: Failed to parse battery percentage value.");
        percentage = -1;
    }

    fclose(fp);
    return percentage;
}


//Connection functions
//Toggle for connections
void atmsys_conn_status(uint8_t rf_type, bool block) {
	int rfk_fd = open("/dev/rfkill", O_WRONLY | O_NONBLOCK); //rfkill device
        if (rfk_fd < 0) {
		slog("[Conn-Log]: No rfkill device found.");
		slog("[Conn-Log]: Network unavailable.");
                printf("[AUTUMNOS]: No rfkill device found. Network unavailable!");
                return;
        }
        struct rfkill_event event;
        memset(&event, 0, sizeof(event));
        event.op = RFKILL_OP_CHANGE_ALL;
	event.type = rf_type;
        event.soft = block ? 1 : 0; // Toggle
        write(rfk_fd, &event, sizeof(event)); //Write all command to device
	close(rfk_fd);
}

//Connection functions using toggle --> atmsys_conn_status
void atmsys_flight(bool enable) {
	atmsys_conn_status(RFKILL_TYPE_ALL, enable); // Kill all connections
}

void atmsys_bt(bool enable) {
	atmsys_conn_status(RFKILL_TYPE_BLUETOOTH, !enable);
}

void atmsys_wifi(bool enable) {
	atmsys_conn_status(RFKILL_TYPE_WLAN, !enable);
}

bool atmsys_wifi_stat(void) {
	int wifi_fd = open("/dev/rfkill", O_RDONLY | O_NONBLOCK);
	if (wifi_fd < 0) return false;
	struct rfkill_event event;
	bool is_blocked = false;
	
	while (read(wifi_fd, &event, sizeof(event)) == sizeof(event)) {
		if (event.type == RFKILL_TYPE_WLAN) {
			is_blocked = (event.soft == 1); // Read connection status
		}
	}
	close(wifi_fd);
	return is_blocked; //Return bool to get current status
}

bool atmsys_bt_stat(void) {
	int bt_fd = open("/dev/rfkill", O_WRONLY | O_RDONLY | O_NONBLOCK);
	if (bt_fd < 0) {
		bt_fd = open("/dev/rfkill", O_RDONLY | O_NONBLOCK);
		if (bt_fd < 0) return false;
	}
	struct rfkill_event event;
	bool is_blocked = false;
	while (read(bt_fd, &event, sizeof(event)) == sizeof(event)) {
		if (event.type == RFKILL_TYPE_BLUETOOTH) {
			is_blocked = (event.soft == 1);
		}
	}
	close(bt_fd);
	return is_blocked;
}

void atmsys_parse_modem(void) {
	static AutumnSIM_t md;
	int serial_fd = AutumnModem_Int_Init(&md);
	if (serial_fd >= 0) {
		external = true; //External modem, SIM slot (Quectel etc.)
	}
	else {
		external = false; //SIM800L
		atmsys_modemhdinit();
	}
}

void atmsys_modemhdinit(void) {
	char path[128];
	atmsys_gpio("/sys/class/gpio/export", MODEM_RST_PIN); // Export Reset Pin address to GPIO
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
    // Opening a serial for SIM800L
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
	write (fd, "AT+CPIN?\r\n", 10); // Pull SIM status data from SIM800L
	tcdrain(fd);
	usleep(200000);
	read(fd, buffer, sizeof(buffer));
	if (strstr(buffer, "READY")) {
		return 1;
	} //SIM inserted.
	else if (strstr(buffer, "SIM PIN")) {
		return 2;
	} // Locked SIM
	else {
		return 0;
	} // No SIM!
}

void atmsys_get_sim_operator_name(int fd, char *provider_name, size_t max_len) {
	char buffer[128];
	memset(buffer, 0, sizeof(buffer));
	write(fd, "AT+COPS?\r\n", 10); // Pull operator name from SIM800L
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
    
    
    strncpy(provider_name, "Servis yok - Yalnızca acil aramalar.", max_len); //No service.
}

void atmsys_modem_answer(int serial_fd) {
	write(serial_fd, "ATA\r\n", 5); //AT Answer
}

void atmsys_modem_reject(int serial_fd) {
	write(serial_fd, "ATH\r\n", 5); //AT Hang
}

//Performance and storage status (Memory stat)
long atmsys_uptime(void) {
	struct  sysinfo s_info; // We pull uptime, RAM info from sysinfo
	int error = sysinfo(&s_info);
	if (error !=0) return -1;
	return s_info.uptime;
}

long atmsys_get_free_RAM(void) {
	struct sysinfo s_info;
	sysinfo(&s_info);
	return (s_info.freeram * s_info.mem_unit) / (1024 * 1024); // Converting data
}

long atmsys_get_used_RAM(void) {
	struct sysinfo s_info;
	if (sysinfo(&s_info) != 0) {
        	return -1; 
   	}
	unsigned long total_mb = (s_info.totalram * s_info.mem_unit) / (1024 * 1024);
   	unsigned long free_mb = (s_info.freeram * s_info.mem_unit) / (1024 * 1024);
    	unsigned long used_mb = total_mb - free_mb; //Total - Free = Used!
	return (long)used_mb; //Calculating
}

long atmsys_get_free_disk_space(const char *path) {
	struct statvfs vfs;
	if(statvfs(path, &vfs) != 0) return -1;
	unsigned long free_bytes = vfs.f_bavail * vfs.f_frsize;
	return free_bytes/(1024*1024);
}

void cpuv(char *buffer, size_t max_len) {
	FILE *fp = fopen("/proc/cpuinfo", "r");
	if (fp == NULL) return;
	
	char line[256];
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "vendor_id", 9) == 0 || strncmp(line, "isa", 3) == 0) {
            		char *delim = strchr(line, ':');
			if (delim != NULL) {
				delim++;
				while (*delim == ' ' || *delim == '\t') delim++;
				size_t len = strlen(delim);
				if (len > 0 && delim[len - 1] == '\n') {
					delim[len - 1] = '\0';
				}
				snprintf(buffer, max_len, "%s", delim);
				fclose(fp);
				return;
			}
		}
	}
	fclose(fp);
}
