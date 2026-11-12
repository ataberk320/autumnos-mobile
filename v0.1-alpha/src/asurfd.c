#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../include/HAL/FramebufferStruct.h"
#include "../include/libfbdev.h"
#include "../include/SymbolTable/table.h"
#include <dlfcn.h>
#include "../include/HAL/io.h" 
#include "../include/HAL/AutixSurf.h"

SYM_TAB *symb = NULL;
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
	}
    }
}

void _AutixSurf_Flush(AS_CTX_t *ctx, FbDev *fb) {
    if (!ctx || !fb || !fb->bk_bf) return;
    
    size_t scr_size = ctx->width * ctx->height * sizeof(uint32_t);
    memcpy(fb->bk_bf, ctx->comp_buf, scr_size);
    
    symb->gfx->RefreshScreen(fb);
}


int main(int argc, char *argv[]) {
	if (argc < 2) return -1;
    	unsigned long tab_addr = 0;
    	sscanf(argv[1], "%lu", &tab_addr);
    	symb = (SYM_TAB*)tab_addr;

	daemonize();

	FbDev screen;
	if (symb->gfx->InitFb(&screen, "/dev/dri/card0") < 0) {
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
