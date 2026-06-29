#include <gif_lib.h>
#include <stdint.h>
#include <png.h>
#include <stdlib.h>
#include "AutumnImage.h"

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
    FILE* fp = fopen(path, "rb");
    if (!fp) return NULL;

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
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
    fclose(fp);

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
