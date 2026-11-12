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
#include <sys/stat.h>
#include "FramebufferStruct.h"
#include "libfbdev.h"
#include "table.h"
#include <dlfcn.h>
#include "io.h" 
#include "AutixSurf.h"

static SYM_TAB *sym = NULL;
static void *fb_handle = NULL;

void daemonize(void) {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    if (setsid() < 0) exit(EXIT_FAILURE);

    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
    chdir("/");

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    _AutumnSys_ioOpen("/dev/null", A_RDONLY, 0);
    _AutumnSys_ioOpen("/dev/null", A_WRONLY, 0);
    _AutumnSys_ioOpen("/dev/null", A_RDWR, 0);
}

AS_CTX_t* _AutixSurf_Initialize(int w, int h) {

	AS_CTX_t *ctx = (AS_CTX_t*)malloc(sizeof(AS_CTX_t));
	if (!ctx) return NULL;

	ctx->width = w; //w parameter as Context width
	ctx->height = h; //h parameter as Context height
	ctx->comp_buf = (uint32_t*)calloc(w * h, sizeof(uint32_t)); //allocate composion buffer

	for (int i = 0; i < MAX_SURFACES; i++) { //placeholder
		ctx->layers[i].active = false; 
		ctx->layers[i].pixels = NULL;
		ctx->layers[i].memfd = -1;
	}
	
	return ctx; //return to allocated ConTeXt
}

int _AutixSurf_SpawnLayer(AS_CTX_t *ctx, int x, int y, int w, int h, int z_order, int type) {
	for (int i = 0; i < MAX_SURFACES; i++) {
		if (!ctx->layers[i].active) {
			size_t size = w * h * sizeof(uint32_t);

			int fd = memfd_create("as_layer", MFD_CLOEXEC | MFD_ALLOW_SEALING); //i prefer memfd for performance instead of shm
			if (fd < 0) return -1;

			if (ftruncate(fd, size) < 0) { 
				close(fd);
				return -1;
			}

			uint32_t *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //mapping the memfd to memory
            		if (mapped == MAP_FAILED) {
                		close(fd);
                		return -1;
            		}
			
			memset(mapped, 0, size);
	
			ctx->layers[i].id = i;
            		ctx->layers[i].memfd = fd;
            		ctx->layers[i].pixels = mapped;
            		ctx->layers[i].x = x;
            		ctx->layers[i].y = y;
            		ctx->layers[i].w = w;
            		ctx->layers[i].h = h;
            		ctx->layers[i].z_order = z_order;
			ctx->layers[i].type = type;
            		ctx->layers[i].active = true;
            		return i;
        	}
    	}
    	return -1;
}

int _AutixSurf_SpawnWayLandSupportedLayer(AS_CTX_t *ctx, int external_memfd, int x, int y, int w, int h, int z_order) {
	for (int i = 0; i < MAX_SURFACES; i++) {
		if (!ctx->layers[i].active) {
			size_t size = w * h * sizeof(uint32_t);

			uint32_t *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, external_memfd, 0);
            		if (mapped == MAP_FAILED) {
                		return -1;
            		}
	
			ctx->layers[i].id = i;
            		ctx->layers[i].memfd = external_memfd;
            		ctx->layers[i].pixels = mapped;
            		ctx->layers[i].x = x;
            		ctx->layers[i].y = y;
            		ctx->layers[i].w = w;
            		ctx->layers[i].h = h;
            		ctx->layers[i].z_order = z_order;
            		ctx->layers[i].active = true;
            		return i;
        	}
    	}
    	return -1;
}

static uint32_t AutixSurf_TitleGradient(int y, int height)
{
    uint8_t top_r = 0x58;
    uint8_t top_g = 0xA8;
    uint8_t top_b = 0x58;

    uint8_t bottom_r = 0x14;
    uint8_t bottom_g = 0x52;
    uint8_t bottom_b = 0x14;

    int r = top_r + ((bottom_r - top_r) * y) / (height - 1);
    int g = top_g + ((bottom_g - top_g) * y) / (height - 1);
    int b = top_b + ((bottom_b - top_b) * y) / (height - 1);

    return 0xFF000000 |
           ((uint32_t)r << 16) |
           ((uint32_t)g << 8) |
           (uint32_t)b;
}

void AutumnSys_AutixSurf_Compose(AS_CTX_t *ctx)
{
    memset(
        ctx->comp_buf,
        0,
        ctx->width * ctx->height * sizeof(uint32_t)
    );

    for (int z = 0; z < 8; z++) {
        for (int i = 0; i < MAX_SURFACES; i++) {
            Layer *layer = &ctx->layers[i];

            if (!layer->active || layer->z_order != z)
                continue;

            for (int ly = 0; ly < layer->h; ly++) {
                int target_y = layer->y + ly;

                if (target_y < 0 || target_y >= ctx->height)
                    continue;

                for (int lx = 0; lx < layer->w; lx++) {
                    int target_x = layer->x + lx;

                    if (target_x < 0 || target_x >= ctx->width)
                        continue;

                    int src_idx =
                        ly * layer->w + lx;

                    int dest_idx =
                        target_y * ctx->width + target_x;

                    uint32_t pixel =
                        layer->pixels[src_idx];

                    if (pixel != 0)
                        ctx->comp_buf[dest_idx] = pixel;
                }
            }

            if (layer->type == AS_LAYER_APP) {
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

                    for (int tx = 0; tx < layer->w; tx++) {
                        int target_x = layer->x + tx;

                        if (target_x < 0 || target_x >= ctx->width)
                            continue;

                        int dest_idx =
                            target_y * ctx->width + target_x;

                        ctx->comp_buf[dest_idx] = color;
                    }
                }
            }
        }
    }
}

void _AutixSurf_Flush(AS_CTX_t *ctx, FbDev *fb) {
    if (!ctx || !fb || !fb->bk_bf) return;
    
    size_t scr_size = ctx->width * ctx->height * sizeof(uint32_t);
    memcpy(fb->bk_bf, ctx->comp_buf, scr_size);
    
    sym->gfx->RefreshScreen(fb);
}


int main(int argc, char *argv[]) {
	if (argc < 2) return -1;
    	unsigned long tab_addr = 0;
    	sscanf(argv[1], "%lu", &tab_addr);
    	sym = (SYM_TAB*)tab_addr;

	daemonize();

	FbDev screen;
	if (sym->gfx->InitFb(&screen, "/dev/dri/card0") < 0) {
		return -1;
	}

	AS_CTX_t *surf = _AutixSurf_Initialize(screen.w, screen.h);
	if (!surf) {
		return -1;
	}

	while (1) {
		AutumnSys_AutixSurf_Compose(surf);
        
        _AutixSurf_Flush(surf, &screen);

        usleep(16000); // ~60 FPS
    }

    return 0;
}
