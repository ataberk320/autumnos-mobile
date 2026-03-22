#ifndef ACORE_H
#define ACORE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


#define CRITICAL_TEMP_THRESHOLD 60.0f
#define LOW_BATTERY_THRESHOLD 15


extern float cpu_temperature;
extern char current_operator[64];
extern bool is_connected;
extern bool sim_present;
extern int uart_fd;
extern int sim800_signal_level;

void uart_init(void);
void read_sim800_data(void);
int read_battery_capacity(void);
float read_cpu_temperature(void);

#endif#ifndef ACORE_H
#define ACORE_H


