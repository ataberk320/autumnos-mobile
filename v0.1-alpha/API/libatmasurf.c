#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/types.h>

#include "AutixSurf.h"
#include "FramebufferStruct.h"
#include "table.h"

#ifndef MAX_SURFACES
#define MAX_SURFACES 32
#endif

#ifndef AS_LAYER_SYSTEMUI
#define AS_LAYER_SYSTEMUI 0
#endif

#ifndef AS_LAYER_APP
#define AS_LAYER_APP 1
#endif

typedef struct {
    int id;
    int memfd;

    uint32_t *pixels;

    int x;
    int y;
    int w;
    int h;

    int z_order;
    int type;

    bool active;
} Layer;

typedef struct {
    int width;
    int height;

    uint32_t *comp_buf;

    Layer layers[MAX_SURFACES];
} AS_CTX_t;

static AS_CTX_t *g_ctx = NULL;
static int g_width = 1024;
static int g_height = 768;

static AS_CTX_t *AutixSurf_Initialize(int w, int h)
{
    AS_CTX_t *ctx = calloc(1, sizeof(AS_CTX_t));

    if (!ctx)
        return NULL;

    ctx->width = w;
    ctx->height = h;

    ctx->comp_buf = calloc(
        (size_t)w * (size_t)h,
        sizeof(uint32_t)
    );

    if (!ctx->comp_buf) {
        free(ctx);
        return NULL;
    }

    for (int i = 0; i < MAX_SURFACES; i++) {
        ctx->layers[i].id = i;
        ctx->layers[i].memfd = -1;
        ctx->layers[i].pixels = NULL;
        ctx->layers[i].active = false;
    }

    return ctx;
}

static void AutixSurf_Destroy(AS_CTX_t *ctx)
{
    if (!ctx)
        return;

    for (int i = 0; i < MAX_SURFACES; i++) {
        Layer *layer = &ctx->layers[i];

        if (!layer->active)
            continue;

        if (layer->pixels && layer->memfd >= 0) {
            size_t size =
                (size_t)layer->w *
                (size_t)layer->h *
                sizeof(uint32_t);

            munmap(layer->pixels, size);
        }

        if (layer->memfd >= 0)
            close(layer->memfd);

        layer->pixels = NULL;
        layer->memfd = -1;
        layer->active = false;
    }

    free(ctx->comp_buf);
    free(ctx);
}

static uint32_t AutixSurf_TitleGradient(int y, int height)
{
    if (height <= 1)
        return 0xFF58A858;

    uint8_t top_r = 0x58;
    uint8_t top_g = 0xA8;
    uint8_t top_b = 0x58;

    uint8_t bottom_r = 0x14;
    uint8_t bottom_g = 0x52;
    uint8_t bottom_b = 0x14;

    int r =
        top_r +
        ((bottom_r - top_r) * y) /
        (height - 1);

    int g =
        top_g +
        ((bottom_g - top_g) * y) /
        (height - 1);

    int b =
        top_b +
        ((bottom_b - top_b) * y) /
        (height - 1);

    return 0xFF000000 |
           ((uint32_t)r << 16) |
           ((uint32_t)g << 8) |
           (uint32_t)b;
}

static void AutixSurf_DrawLayer(
    AS_CTX_t *ctx,
    Layer *layer
)
{
    if (!ctx || !layer || !layer->active)
        return;

    if (!layer->pixels)
        return;

    for (int sy = 0; sy < layer->h; sy++) {

        int target_y = layer->y + sy;

        if (target_y < 0 || target_y >= ctx->height)
            continue;

        int source_x = 0;
        int target_x = layer->x;

        int width = layer->w;

        if (target_x < 0) {
            source_x = -target_x;
            width -= source_x;
            target_x = 0;
        }

        if (target_x + width > ctx->width)
            width = ctx->width - target_x;

        if (width <= 0)
            continue;

        uint32_t *src =
            &layer->pixels[
                (size_t)sy * (size_t)layer->w +
                source_x
            ];

        uint32_t *dst =
            &ctx->comp_buf[
                (size_t)target_y *
                (size_t)ctx->width +
                target_x
            ];

        memcpy(
            dst,
            src,
            (size_t)width * sizeof(uint32_t)
        );
    }

    if (layer->type != AS_LAYER_APP)
        return;

    int title_height = 16;

    if (title_height > layer->h)
        title_height = layer->h;

    for (int ty = 0; ty < title_height; ty++) {

        int target_y = layer->y + ty;

        if (target_y < 0 || target_y >= ctx->height)
            continue;

        uint32_t color =
            AutixSurf_TitleGradient(
                ty,
                title_height
            );

        int start_x = layer->x;
        int end_x = layer->x + layer->w;

        if (start_x < 0)
            start_x = 0;

        if (end_x > ctx->width)
            end_x = ctx->width;

        for (int tx = start_x; tx < end_x; tx++) {
            ctx->comp_buf[
                (size_t)target_y *
                (size_t)ctx->width +
                tx
            ] = color;
        }
    }
}

static int AutixSurf_GetLayerCount(AS_CTX_t *ctx)
{
    int count = 0;

    if (!ctx)
        return 0;

    for (int i = 0; i < MAX_SURFACES; i++) {
        if (ctx->layers[i].active)
            count++;
    }

    return count;
}

static void AutixSurf_Compose(AS_CTX_t *ctx)
{
    if (!ctx)
        return;

    memset(
        ctx->comp_buf,
        0,
        (size_t)ctx->width *
        (size_t)ctx->height *
        sizeof(uint32_t)
    );

    for (int z = 0; z < MAX_SURFACES; z++) {

        for (int i = 0; i < MAX_SURFACES; i++) {

            Layer *layer = &ctx->layers[i];

            if (!layer->active)
                continue;

            if (layer->z_order != z)
                continue;

            AutixSurf_DrawLayer(ctx, layer);
        }
    }
}

int AutumnAPI_SpawnLayer(
    int *memfd_out,
    uint32_t **pixels_out,
    int x,
    int y,
    int w,
    int h,
    int z_order,
    int type
)
{
    if (!memfd_out || !pixels_out)
        return -1;

    if (w <= 0 || h <= 0)
        return -1;

    if (!g_ctx) {
        g_ctx =
            AutixSurf_Initialize(
                g_width,
                g_height
            );

        if (!g_ctx)
            return -1;
    }

    for (int i = 0; i < MAX_SURFACES; i++) {

        Layer *layer = &g_ctx->layers[i];

        if (layer->active)
            continue;

        size_t size =
            (size_t)w *
            (size_t)h *
            sizeof(uint32_t);

        int fd =
            memfd_create(
                "as_layer",
                MFD_CLOEXEC | MFD_ALLOW_SEALING
            );

        if (fd < 0)
            return -1;

        if (ftruncate(fd, (off_t)size) < 0) {
            close(fd);
            return -1;
        }

        uint32_t *mapped =
            mmap(
                NULL,
                size,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                fd,
                0
            );

        if (mapped == MAP_FAILED) {
            close(fd);
            return -1;
        }

        memset(mapped, 0, size);

        layer->id = i;
        layer->memfd = fd;
        layer->pixels = mapped;

        layer->x = x;
        layer->y = y;
        layer->w = w;
        layer->h = h;

        layer->z_order = z_order;
        layer->type = type;

        layer->active = true;

        *memfd_out = fd;
        *pixels_out = mapped;

        return i;
    }

    return -1;
}

int AutumnAPI_DestroyLayer(int layer_id)
{
    if (!g_ctx)
        return -1;

    if (layer_id < 0 || layer_id >= MAX_SURFACES)
        return -1;

    Layer *layer =
        &g_ctx->layers[layer_id];

    if (!layer->active)
        return -1;

    size_t size =
        (size_t)layer->w *
        (size_t)layer->h *
        sizeof(uint32_t);

    if (layer->pixels)
        munmap(layer->pixels, size);

    if (layer->memfd >= 0)
        close(layer->memfd);

    memset(layer, 0, sizeof(Layer));

    layer->id = layer_id;
    layer->memfd = -1;

    return 0;
}

uint32_t *AutumnAPI_GetCompositeBuffer(
    int *width,
    int *height
)
{
    if (!g_ctx)
        return NULL;

    if (width)
        *width = g_ctx->width;

    if (height)
        *height = g_ctx->height;

    AutixSurf_Compose(g_ctx);

    return g_ctx->comp_buf;
}

int AutumnAPI_SetDisplaySize(int w, int h)
{
    if (w <= 0 || h <= 0)
        return -1;

    if (g_ctx)
        return -1;

    g_width = w;
    g_height = h;

    return 0;
}

int AutumnAPI_GetLayerCount(void)
{
    return AutixSurf_GetLayerCount(g_ctx);
}
