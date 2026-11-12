#ifndef AS_H
#define AS_H

#define MAX_SURFACES 16
#define SYSUI 0
#define APP 1

typedef struct {
        int id;
        int memfd;
        uint32_t *pixels;
        int x, y, w, h;
        int z_order;
	int type;
        bool active;
} Layer;

typedef struct {
        int width;
        int height;
        Layer layers[MAX_SURFACES];
        uint32_t *comp_buf;
} AS_CTX_t;

int _AutixSurf_SpawnLayer(AS_CTX_t *ctx, int x, int y, int w, int h, int z_order, int type);
int _AutixSurf_SpawnWayLandSupportedLayer(AS_CTX_t *ctx, int external_memfd, int x, int y, int w, int h, int z_order);

#endif
