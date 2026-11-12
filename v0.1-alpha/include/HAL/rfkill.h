#ifndef ATMRFK_H
#define ATMRFK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	ATMRFK_ALL = 0,
	ATMRFK_WLAN,
	ATMRFK_BT,
	ATMRFK_UWB,
	ATMRFK_WIMAX,
	ATMRFK_WWAN,
	ATMRFK_GPS,
	ATMRTK_FM,
	ATMRFK_NFC,
} rfkillType;

typedef struct {
	uint32_t index;
	rfkillType type;
	bool soft_blocked;
	bool hard_blocked;
} rfkillDev;

int hal_rfkilldevlist(rfkillDev *devs, size_t capacity, size_t *count);
int hal_rfkillsetblk(uint32_t index, bool blocked);

#ifdef __cplusplus
}
#endif
#endif
