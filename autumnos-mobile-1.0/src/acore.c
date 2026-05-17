#define TERMINAL_AS_LIB
#include "AutumnCoreUtilsTerminal.c"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/input.h>
#include "atmhal.h"
#include "AutumnMouseArg.h"
#include "AtmDrv_G2D.h"
#include "acoreutils.h"
#include <termios.h>
#include <pthread.h>
#include <sys/mman.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#define LIMIT 6

static int pwr_counter = 0;
static int kmsg_fd = -1;
static int mouse_fd = -1;
extern int input_fd;

#define AUTUMN_IPC_PATH "/tmp/autumn_conf/AutumnCore0"
#define AUTUMN_MS_PATH "/tmp/autumn_sock/AutumnMsP0"
#define SHM_NAME "/autumn_vid"
#define VIDEO_W  320
#define VIDEO_H  240

static autumn_touchpad_t touch = {0, 0, 0};
static AVFormatContext *pFormatCtx = NULL;
static AVCodecContext *pCodecCtx = NULL;
static AVFrame *pFrame = NULL;
static int video_stream_idx = -1;
unsigned char *HAL_buf = NULL;
uint16_t *AtmUI_buf = NULL;
int shm_fd = -1;

void mkdir_data(void) {
	mkdir("/tmp/autumnsys", 0777);
    	mkdir("/tmp/autumnsys/battery", 0777);
    	mkdir("/tmp/autumnsys/uptime", 0777);
    	mkdir("/tmp/autumnsys/power", 0777);
	mkdir("/tmp/autumnsys/memory", 0777);
	mkdir("/tmp/autumnsys/storage", 0777);
	mkdir("/tmp/autumnsys/connection", 0777);
	mkdir("/tmp/autumn_sock", 0777);
	mkdir("/tmp/autumn_conf", 0777);
}

void setup_ipc() {
	mkfifo(AUTUMN_IPC_PATH, 0666);
}

void serial_raw(int fd) {
	if (fd < 0) return;
	struct termios toptions;

	if (tcgetattr(fd, &toptions) < 0) {
		return;
	}

	toptions.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON | ISIG);

	toptions.c_iflag &= ~(INPCK | ISTRIP | IGNCR | ICRNL | INLCR | IXON | IXOFF);
	toptions.c_oflag &= ~(OPOST);

	toptions.c_cflag &= ~PARENB;
    	toptions.c_cflag &= ~CSTOPB;
    	toptions.c_cflag &= ~CSIZE;
    	toptions.c_cflag |= CS8;
    
    	toptions.c_cc[VMIN] = 1;
    	toptions.c_cc[VTIME] = 0;
    
    	tcsetattr(fd, TCSANOW, &toptions);
}

int init_shared_memory(void) {
	size_t shm_size = VIDEO_W * VIDEO_H * 4;
	shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) return -1;
	if (ftruncate(shm_fd, shm_size) == -1) return -2;
	AtmUI_buf = (uint16_t *)mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    	if (AtmUI_buf == MAP_FAILED) return -3;
    	return 0;
}

void WriteToKernel(int serial_fd) {
	char buffer[128];
	int fd = open(AUTUMN_IPC_PATH, O_RDONLY | O_NONBLOCK);
	if (fd >= 0) {
		ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
		if (n > 0) {
			buffer[n] = '\0';
			if (strncmp(buffer, "set_bright:", 11) == 0) {
				int val = atoi(buffer + 11);
				atmsys_set_brightness(val);
			}
			else if (strcmp(buffer, "flight_mode") == 0) {
				atmsys_flight(true);
			}
			
			else if (strcmp(buffer, "volume_down") == 0) {
				atmsys_volume_down();
			}

			else if (strcmp(buffer, "volume_up") == 0) {
				atmsys_volume_up();
			}

			else if (strcmp(buffer, "answer_call") == 0) {
				atmsys_modem_answer(serial_fd);
			}

			else if (strcmp(buffer, "decline_call") == 0) {
				atmsys_modem_reject(serial_fd);
			}

			else {
				printf("Invalid HAL parameter or command!");
				return;
			}

		}
	}
	close(fd);
}

int autumn_video_init_player(const char *filename) {
    if (!HAL_buf) HAL_buf = (unsigned char *)malloc(VIDEO_W * VIDEO_H * 3); 
    
    if (init_shared_memory() != 0) {
        return -1;
    }
    
    if (avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0) return -2;
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) return -3;
    
    video_stream_idx = -1;
    for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }
    if (video_stream_idx == -1) return -4;

    AVCodecParameters *pCodecPar = pFormatCtx->streams[video_stream_idx]->codecpar;
    const AVCodec *pCodec = avcodec_find_decoder(pCodecPar->codec_id);
    if (!pCodec) return -5;

    pCodecCtx = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecCtx, pCodecPar);
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) return -6;

    pFrame = av_frame_alloc();
    return 0;
}

void stop_vidplayer(void) {
	if (pFrame) {av_frame_free(&pFrame); pFrame = NULL; }
	if (pCodecCtx) { avcodec_free_context(&pCodecCtx); pCodecCtx = NULL; }
	if (pFormatCtx) { avformat_close_input(&pFormatCtx); pFormatCtx = NULL; }
	
	if (AtmUI_buf && AtmUI_buf != MAP_FAILED) {
		munmap(AtmUI_buf, VIDEO_W * VIDEO_H * 2);
		AtmUI_buf = NULL;
	}
	if (shm_fd != -1) {
		close(shm_fd);
		shm_unlink(SHM_NAME);
		shm_fd = -1;
	}
	video_stream_idx = -1;
}
int play_vid(void) {
	if (!pFormatCtx || !pCodecCtx || !pFrame || !HAL_buf || !AtmUI_buf) return -1;

	int has_frame = atmsys_decode_videopix_libc(pFormatCtx, pCodecCtx, video_stream_idx, pFrame, HAL_buf);
	if (has_frame == 1) {
		AtmDrv_G2D_Blit((uintptr_t)HAL_buf, (uintptr_t)AtmUI_buf, VIDEO_W, VIDEO_H, 0);
	}
	else {
		stop_vidplayer();
	}
}


void check_power_status(void) {
    char line[64];
    int req_val = -1;

	if (atmsys_pwrstat()) {
		pwr_counter++;
		if (pwr_counter >= LIMIT) {
			atmsys_pwroff();
		}
	}
	else {
		pwr_counter = 0;
	}
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

void update_driver_status(void) {
	struct input_event ev;
	bool moved = false;
	if (input_fd < 0) {
        	atmsys_indev_type();
		if (input_fd < 0) return;
    	}

    	while (read(input_fd, &ev, sizeof(ev)) > 0) {
        	if (ev.type == EV_ABS) {
            		if (ev.code == ABS_X) { touch.x = (ev.value * 480) / 32767; moved = true; }
            		if (ev.code == ABS_Y) { touch.y = (ev.value * 800) / 32767; moved = true; }
        	}
		
		else if (ev.type == EV_REL) {
			if (ev.code == REL_X) { touch.x += ev.value; moved = true; }
			if (ev.code == REL_Y) { touch.y += ev.value; moved = true; }
			
			if (touch.x < 0) touch.x = 0;
			if (touch.y < 0) touch.y = 0;
			if (touch.x >= 480) touch.x = 479;
			if (touch.y >= 800) touch.y = 799;
		}

        	else if (ev.type == EV_KEY && (ev.code == BTN_LEFT || ev.code == BTN_TOUCH)) {
            		touch.pressed = ev.value;
			moved = true;
        	}
    	}	
	if (moved) {
		FILE *fp_mouse = fopen(AUTUMN_MS_PATH, "w");
		if (fp_mouse != NULL) {
			fprintf(fp_mouse, "%d %d %d", touch.x, touch.y, touch.pressed);
        		fclose(fp_mouse);
		}
		else {
			printf("[AutumnCore]: Couldn't init mouse driver!");
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

	char op_name[64];
	atmsys_get_sim_operator_name(serial_fd, op_name, sizeof(op_name));
	FILE *fp_op = fopen("/tmp/autumnsys/connection/autumnoperator0", "w");
	if (fp_op) {
	fprintf(fp_op, "%s", op_name); 
        fclose(fp_op);
    }


}


int main(void) {
	mkdir_data();
	atmsys_safe_volume(65);
	atmsys_modemhdinit();
	int serial_fd = open("/dev/ttyS1", O_RDWR | O_NOCTTY);
	serial_raw(serial_fd);
	WriteToKernel(serial_fd);
	if (atmsys_camera_init() == 0) {
		kmsg("[AUTUMNOS]: Camera driver was initialized.");
	}
	else {
		kmsg("[AUTUMNOS]: An error occurred when camera driver initializing.");
	}

	if (AtmDrv_G2D_Init() == 0) {
		kmsg("AtmDrv_G2D: the driver was initialized.");
	}
	else { 
		kmsg("AtmDrv_G2D: initialize error");
	}

	int tick = 0;
	while (1) {
		update_driver_status();
		if (tick >= 10) {
			update_system_status(serial_fd);
			check_power_status();
			usleep(16666);
		}
		tick++;
	
		usleep(16666);
	}
	if (serial_fd >= 0) close(serial_fd);
	return 0;
}
