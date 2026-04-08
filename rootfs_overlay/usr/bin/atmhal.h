#ifndef ATMHAL_H
#define ATMHAL_H
#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
void atmsys_set_brightness(uint8_t level);

void atmsys_volume_up(void) ;

void atmsys_volume_down(void);

void atmsys_safe_volume(uint8_t volume);

void playaudio(const char *path);

int atmsys_camera_init(void);

void atmsys_convert_videofrm(AVFrame *pFrame, AVCodecContext *pCodecCtx, unsigned char *out_buffer, int target_width, int target_height);

void *atmsys_play_video(void *arg);

//Power options
void atmsys_reboot(void);

void atmsys_pwroff(void);

bool atmsys_pwrstat(void);

int atmsys_battery_perc(void);

//Connection functions
void atmsys_flight(bool enable);

void atmsys_modemhdinit(void);

int atmsys_modeminit(const char* port);

void atmsys_modem_software_init(int fd);

//Performance and storage status (Memory stat)
long atmsys_uptime(void);

long atmsys_get_free_RAM(void);

long atmsys_get_used_RAM(void);

long atmsys_get_free_disk_space(const char *path);
#ifdef __cplusplus
}
#endif
#endif
