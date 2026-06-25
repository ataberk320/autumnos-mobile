#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

typedef enum {
	GPIO_MODE_INPUT = 0,
	GPIO_MODE_OUTPUT = 1
} GPIOMode;

typedef struct {
    void (*init)(void);
    void (*set_mode)(uint32_t pin, GPIOMode mode);
    void (*write)(uint32_t pin, int value);
    int  (*read)(uint32_t pin);
} GPIO_Driver;

extern GPIO_Driver* io;

#endif
