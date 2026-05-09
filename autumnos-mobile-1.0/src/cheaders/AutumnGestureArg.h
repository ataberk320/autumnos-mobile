#ifndef GESTURE_H
#define GESTURE_H

typedef enum {
	AUTUMN_G_NONE = 0,
	AUTUMN_G_S_TO_L,
	AUTUMN_G_A_TO_Y,
	AUTUMN_G_LONG_PRESS,
	AUTUMN_G_TAP
} AutumnGesture;

tyoedef struct {
	AutumnGesture type;
	uint32_t duration;
	uint8_t intensity;
} AutumnGestureEvent;

#endif
