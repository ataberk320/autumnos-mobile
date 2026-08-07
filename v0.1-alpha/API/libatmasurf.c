#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include "FramebufferStruct.h"


typedef struct {
	int id;
	int memfd;
	uint32_t *pixels;
	int x, y, w, h;
	int z_order;
	bool active;
} Layer;

int AutumnAPI_SpawnLayer(int *memfd_out, uint32_t **pixels_out, int x, int y, int w, int h, int z_order) {
    size_t size = w * h * sizeof(uint32_t);

    int fd = memfd_create("as_layer", MFD_CLOEXEC | MFD_ALLOW_SEALING);
    if (fd < 0) return -1;

    if (ftruncate(fd, size) < 0) { 
        close(fd);
        return -1;
    }

    uint32_t *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        close(fd);
        return -1;
    }
    
    memset(mapped, 0, size);

    *memfd_out = fd;
    *pixels_out = mapped;
    return 0;
}
