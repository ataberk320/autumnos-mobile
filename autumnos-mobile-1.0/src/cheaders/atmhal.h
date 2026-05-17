#ifndef ATMHAL_H
#define ATMHAL_H
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define TEST_BIT(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)
#include "AutumnMouseArg.h"
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

int atmsys_decode_videopix_libc(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, int videoStream, AVFrame *pFrame, unsigned char *out_buffer);

void atmsys_play_video(const char *source, unsigned char *final_out_buffer, int start_second);

//Driver initialization 
void atmsys_get_touch(autumn_touchpad_t *touch);

//Power options
void atmsys_reboot(void);

void atmsys_pwroff(void);

void atmsys_emergency_pwroff(void);

int atmsys_pwrstat(void);

void atmsys_indev_init(const char *suggested_path);

void atmsys_indev_type(void);

int atmsys_battery_perc(void);

//Connection functions
void atmsys_flight(bool enable);

void atmsys_bt(bool enable);

void atmsys_wifi(bool enable);

bool atmsys_wifi_stat(void);

bool atmsys_bt_stat(void);

void atmsys_modemhdinit(void);

int atmsys_modeminit(const char* port);

void atmsys_modem_software_init(int fd);

int atmsys_is_sim_inserted(int fd);

void atmsys_get_sim_operator_name(int fd, char *provider_name, size_t max_len);

void atmsys_modem_answer(int serial_fd);

void atmsys_modem_reject(int serial_fd);

//Performance and storage status (Memory stat)
long atmsys_uptime(void);

long atmsys_get_free_RAM(void);

long atmsys_get_used_RAM(void);

long atmsys_get_free_disk_space(const char *path);
#ifdef __cplusplus
}
#endif
#endif
