#ifndef ACORE_H
#define ACORE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Sistem Sabitleri
#define CRITICAL_TEMP_THRESHOLD 60.0f
#define LOW_BATTERY_THRESHOLD 15

// Global Değişkenler
extern float cpu_temperature;
extern char current_operator[64]; // Boyutu 64 yapalım, garanti olsun
extern bool is_connected;
extern bool sim_present;
extern int uart_fd;
extern int sim800_signal_level;
// Fonksiyonlar
void uart_init(void);
void read_sim800_data(void);
int read_battery_capacity(void);
float read_cpu_temperature(void);

#endif#ifndef ACORE_H
#define ACORE_H


