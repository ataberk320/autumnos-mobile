#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>


#include "g2d_driver_i.h" 

int main() {

    int g2d_fd = open("/dev/g2d", O_RDWR);
    if (g2d_fd < 0) {
        return -1;
    }

    g2d_blt blit_para;
    memset(&blit_para, 0, sizeof(g2d_blt));

    // Kaynak Görüntü (Source)
    blit_para.src_image.addr[0] = 0x45000000;
    blit_para.src_image.w       = 800;
    blit_para.src_image.h       = 480;
    blit_para.src_image.format  = G2D_FORMAT_ARGB8888;


    blit_para.dst_image.addr[0] = 0x46000000;
    blit_para.dst_image.w       = 800;
    blit_para.dst_image.h       = 480;
    blit_para.dst_image.format  = G2D_FORMAT_ARGB8888;


    blit_para.src_rect.x = 0; blit_para.src_rect.y = 0;
    blit_para.src_rect.w = 800; blit_para.src_rect.h = 480;
    blit_para.dst_x = 0; blit_para.dst_y = 0;

    if (ioctl(g2d_fd, G2D_CMD_BITBLT, &blit_para) < 0) {

    } else {

        ioctl(g2d_fd, G2D_CMD_WAIT_FINISH, NULL);
    }

    close(g2d_fd);
    return 0;
}
