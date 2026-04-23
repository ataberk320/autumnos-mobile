#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>

#include "sunxi-g2d.h"
#define BLIT_SCREEN 1

int main(int argc, char** argv) {
    int g2d_fd = open("/dev/g2d", O_RDWR);
    if (g2d_fd < 0) {
        return -1;
    }

    int request = 0;
    g2d_blt config;
    memset(&config, 0, sizeof(g2d_blt));
    while (1) {
        if (request == BLIT_SCREEN) {
            if (ioctl(g2d_fd, G2D_CMD_BITBLT, &config) < 0) {
                perror("IOCTL error!");
            }
            request = 0; 
        }

        usleep(100000); 
    }

    close(g2d_fd);
    return 0;
}
