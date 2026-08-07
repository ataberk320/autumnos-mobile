#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include "io.h"

int _mp4_RdCallback(int64_t offset, void *buffer, size_t size, void *token) {
    int fd = *(int *)token;
    if (_AutumnSys_ioLseek(fd, offset, SEEK_SET) < 0) return 0;
    ssize_t r = _AutumnSys_ioRead(fd, buffer, size);
    return (r > 0) ? (int)r : 0;
}

