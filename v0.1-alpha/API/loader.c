#include "table.h"
#include <dlfcn.h>

GFX_API* gfx = NULL;
IMG_API* img = NULL;
ENCODE_API* enc = NULL;
CHAT_TUN_API* tun = NULL;

static void *fb_handle = NULL;
static void *io_handle = NULL;
static void *nls_handle = NULL;
static void *tun_handle = NULL;

void get_mod() {
	fb_handle = dlopen("/usr/lib/libfbdev.so", RTLD_GLOBAL | RTLD_LAZY);
        if (!fb_handle) {
                printf("libfbdev.so - Could not load library: %s\n", dlerror());
                return;
        }

        gfx = (GFX_API*)malloc(sizeof(GFX_API));
	img = (IMG_API*)malloc(sizeof(IMG_API));
	tun = (CHAT_TUN_API*)malloc(sizeof(CHAT_TUN_API));
        gfx->DrawImage = (DrawImage_t)dlsym(fb_handle, "AutumnAPI_DrawImg");
        gfx->DrawButton = (DrawButton_t)dlsym(fb_handle, "AutumnAPI_DrawButton");
        gfx->InitFb = (InitFb_t)dlsym(fb_handle, "AutumnAPI_Init");
        gfx->RefreshScreen = (RefreshScreen_t)dlsym(fb_handle, "AutumnAPI_FbRefresh");
        gfx->DrawGif = (DrawGif_t)dlsym(fb_handle, "AutumnAPI_DrawGifAni");
        gfx->Clear = (Clear_t)dlsym(fb_handle, "AutumnAPI_ClearFb");
        gfx->Text = (Text_t)dlsym(fb_handle, "AutumnAPI_DrawString");
        gfx->DrawRectangle = (DrawRectangle_t)dlsym(fb_handle, "AutumnAPI_DrawRect");

        if (!gfx->DrawImage || !gfx->DrawButton || !gfx->InitFb || !gfx->RefreshScreen ||
            !gfx->DrawGif || !gfx->Clear || !gfx->Text || !gfx->DrawRectangle) {
                perror("dlsym - libfbdev.so");
		goto cleanup;
                return;
        }

        io_handle = dlopen("/usr/lib/libatmio.so", RTLD_GLOBAL | RTLD_LAZY);
        if (!io_handle) {
                printf("libatmio.so - Could not load library: %s\n", dlerror());
		goto cleanup;
                return;
        }
        img->LdGif = (LdGif_t)dlsym(io_handle, "AutumnAPI_LoadGif");
        img->LdImg = (LdImg_t)dlsym(io_handle, "AutumnAPI_LoadImg");
        img->CountGif = (CountGif_t)dlsym(io_handle, "AutumnAPI_GetFrameCount");
		
        if (!img->LdGif || !img->LdImg || !img->CountGif) {
                perror("dlsym - libatmio.so");
		goto cleanup;
                return;
        }
	
	tun_handle = dlopen("/usr/lib/libatmchtn.so", RTLD_GLOBAL | RTLD_LAZY);
        if (!tun_handle) {
                printf("libatmchtn.so - Could not load library: %s\n", dlerror());
		goto cleanup;
		return;
        }
        tun->Create = (Create_t)dlsym(tun_handle, "AutumnAPI_Tunnel_Create");
        tun->Receive = (Receive_t)dlsym(tun_handle, "AutumnAPI_Tunnel_ReceiveFromFriend");
        tun->Send = (Send_t)dlsym(tun_handle, "AutumnAPI_Tunnel_Send_Msg");

        if (!tun->Create || !tun->Receive || !tun->Send) {
                perror("dlsym - libatmio.so");
		goto cleanup;
                return;
        }
cleanup:
    if (tun_handle) dlclose(tun_handle);
    if (io_handle) dlclose(io_handle);
    if (fb_handle) dlclose(fb_handle);
    
    if (gfx) { free(gfx); gfx = NULL; }
    if (img) { free(img); img = NULL; }
    if (tun) { free(tun); tun = NULL; }
}
        
