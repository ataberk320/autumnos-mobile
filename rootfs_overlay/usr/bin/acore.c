#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "acore.h"
float cpu_temperature = 0.0f;
char current_operator[64] = "SIM Kart Yok"; 
bool is_connected = false;
bool sim_present = false;
int sim800_signal_level = 0;
int uart_fd = -1;
void uart_init() {
    uart_fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fd == -1) return;

    struct termios options;
    tcgetattr(uart_fd, &options);
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    options.c_cflag |= (CLOCAL | CREAD | CS8);
    options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    options.c_oflag &= ~OPOST;
    tcsetattr(uart_fd, TCSANOW, &options);
}


int send_at_command(const char* cmd, char* buffer, int buf_size) {
    write(uart_fd, cmd, strlen(cmd));
    write(uart_fd, "\r\n", 2);
    usleep(500000);
    int n = read(uart_fd, buffer, buf_size - 1);
    if (n > 0) buffer[n] = '\0';
    return n;
}

void read_sim800_data() {
    char buffer[256];

    // SIM Kart Varlık Kontrolü (AT+CPIN?)
    if (send_at_command("AT+CPIN?", buffer, sizeof(buffer)) > 0) {
        sim_present = (strstr(buffer, "READY") != NULL);
    } else {
        sim_present = false;
    }

    if (send_at_command("AT+COPS?", buffer, sizeof(buffer)) > 0) {
        char* ptr = strchr(buffer, '"');
        if (ptr) {
            char* end_ptr = strchr(ptr + 1, '"');
            if (end_ptr) {
                int len = end_ptr - (ptr + 1);
                strncpy(current_operator, ptr + 1, len);
                current_operator[len] = '\0';
            }
        }
    }

    if (send_at_command("AT+CSQ", buffer, sizeof(buffer)) > 0) {
        int rssi;
        if (sscanf(buffer, "\r\n+CSQ: %d,", &rssi) == 1) {
            sim800_signal_level = (rssi * 100) / 31;
            is_connected = (rssi != 99);
        }
    }
}


int read_battery_capacity() {
    FILE *file = fopen("/sys/class/power_supply/bat0/capacity", "r");
    int cap = 0;
    if (file) {
        if (fscanf(file, "%d", &cap) == 1) {
            fclose(file);
            return cap;
        }
        fclose(file);
    }
    return -1;
}


float read_cpu_temperature() {
    FILE *file = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    int temp = 0;
    if (file) {
        if (fscanf(file, "%d", &temp) == 1) {
            fclose(file);
            return temp / 1000.0f;
        }
        fclose(file);
    }
    return 0.0f;
}
