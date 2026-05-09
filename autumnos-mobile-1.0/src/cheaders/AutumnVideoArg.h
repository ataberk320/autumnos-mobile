#ifndef VIDEO_ARG_H
#define VIDEO_ARG_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
        char *source;
        unsigned char *out_buffer;
        int start_sec;
} VideoThreadArgs;
#ifdef __cplusplus
}
#endif

#endif
