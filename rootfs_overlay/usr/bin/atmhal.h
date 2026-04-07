#ifndef ATMHAL_H
#define ATMHAL_H
#include <stdint.h>
#include <stdbool.h>
void atmsys_set_brightness(uint8_t level);

void atmsys_volume_up(void) ;

void atmsys_volume_down(void);

void atmsys_safe_volume(uint8_t volume);

void playaudio(const char *path);

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

#endif
