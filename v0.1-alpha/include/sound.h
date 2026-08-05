#ifndef SHW_H
#define SHW_H

#include <stdint.h>

typedef struct {
    int fd;
    int channels;
    int sample_rate;
} __attribute__((aligned(8))) shw;


#endif
