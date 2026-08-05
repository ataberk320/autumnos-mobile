#define _GNU_SOURCE
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>
#include <gif_lib.h>
#include <stdint.h>
#include <png.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "AutumnImage.h"
#include "io.h" //added missing headers
#include <string.h>

//AutumnIO implemented PNG loader callback.
extern void _png_RdCallback(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead);

//added high level file functions
int AutumnAPI_FCp(const char* src_p, const char* dst_p) {
        int src_fd = _AutumnSys_ioOpen(src_p, 0, 0);
        if (src_fd < 0) return -1;

        int dst_fd = _AutumnSys_ioOpen(dst_p, 01101, 0644);
        if (dst_fd < 0) {
                _AutumnSys_ioClose(src_fd);
                return -1;
        }

        char buffer[4096];
        ssize_t bytes_read, bytes_written;

        while ((bytes_read = _AutumnSys_ioRead(src_fd, buffer, sizeof(buffer))) > 0) {
                char *ptr = buffer;
                while (bytes_read > 0) {
                        bytes_written = _AutumnSys_ioWrite(dst_fd, ptr, bytes_read);
                        if (bytes_written < 0) {
                                _AutumnSys_ioClose(src_fd);
                                _AutumnSys_ioClose(dst_fd);
                                return -1;
                        }
                        bytes_read -= bytes_written;
                        ptr += bytes_written;
                }
        }

        _AutumnSys_ioClose(src_fd);
        _AutumnSys_ioClose(dst_fd);
        return 0;
}

int AutumnAPI_FRename(const char* old_n, const char* new_n) {
        return _AutumnSys_ioRename(old_n, new_n);
}
int AutumnAPI_FRename(const char* old_n, const char* new_n) {
        return _AutumnSys_ioRename(old_n, new_n);
}

int AutumnAPI_FMv(const char* src_p, const char* dst_p) {
        char full_dst_p[512];
        const char* fname = strrchr(src_p, '/');
        if (!fname) fname = src_p;
        else fname++;

        snprintf(full_dst_p, sizeof(full_dst_p), "%s/%s", dst_p, fname);
        return _AutumnSys_ioRename(src_p, full_dst_p);
}

int AutumnAPI_Delete(const char* path) {
    return _AutumnSys_ioUnlink(path);
}

GifFileType* AutumnAPI_LoadGif(const char* path) {
        int error;
        GifFileType* gif = DGifOpenFileName(path, &error);
        if (!gif) return NULL;
        if (DGifSlurp(gif) != GIF_OK) {
                DGifCloseFile(gif, &error);
                return NULL;
        }
        return gif;
}

AutumnImage* AutumnAPI_LoadImg(const char* path) {
        
AutumnImage* AutumnAPI_LoadImg(const char* path) {
    int fp = _AutumnSys_ioOpen(path, 0, 0); //AutumnIO implementation
    if (!fp) return NULL;
    //error controls
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        _AutumnSys_ioClose(fp);
        return NULL;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, NULL, NULL);
        _AutumnSys_ioClose(fp);
        return NULL;
    }
    //binding our callback :)
    png_set_read_fn(png, (void*)(intptr_t)fp, _png_RdCallback);
    png_init_io(png, fp);
    png_read_info(png, info);

    png_set_expand(png);
    png_set_gray_to_rgb(png);
    png_set_palette_to_rgb(png);
    png_set_tRNS_to_alpha(png);
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    png_read_update_info(png, info);

    AutumnImage* img = malloc(sizeof(AutumnImage));
    img->w = png_get_image_width(png, info);
    img->h = png_get_image_height(png, info);
    img->buffer = malloc(img->w * img->h * sizeof(uint32_t));

    png_bytep* rows = malloc(sizeof(png_bytep) * img->h);
    for(int i = 0; i < img->h; i++) {
        rows[i] = (png_bytep)&img->buffer[i * img->w];
    }
    png_read_image(png, rows);

    free(rows);
    png_destroy_read_struct(&png, &info, NULL);
    _AutumnSys_ioClose(fp);

    return img;
}

void AutumnAPI_FreeGif(GifFileType* gif) {
    if (gif) {
        int error;
        DGifCloseFile(gif, &error);
    }
}

int AutumnAPI_GetFrameCount(GifFileType* gif) {
    if (!gif) return 0;
    return gif->ImageCount;
}
