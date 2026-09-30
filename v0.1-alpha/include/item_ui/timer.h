#ifndef TIMER_H
#define TIMER_H

typedef struct {
	int x, y;
	unsigned int color;
	int elapsed;
	int total;
	char label[16];
} TimerWidget;

#endif
