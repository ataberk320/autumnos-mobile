#include "table.h"
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>  // <-- 1. Bunu kesin ekle (malloc/free/exit için)
#include <stdio.h>


extern bool is_alsa;

__attribute__((visibility("default"))) GFX_API* gfx = NULL;
__attribute__((visibility("default"))) IMG_API* img = NULL;
__attribute__((visibility("default"))) WIDGET_API* wid = NULL;
__attribute__((visibility("default"))) MOUSE_HAL* mshal = NULL;
__attribute__((visibility("default"))) SOUND_HAL* snd = NULL;
__attribute__((visibility("default"))) FBDEV_HAL* fbd = NULL;
__attribute__((visibility("default"))) MP3_PLAYER_API* mp3p = NULL;

static void *fb_handle = NULL;
static void *io_handle = NULL;
static void *mshal_handle = NULL;
static void *wid_handle = NULL;
static void *snd_handle = NULL;
static void *ply_handle = NULL;
static void *screen_handle = NULL;

void check_align(void* ptr, const char* name) {
	if (ptr == NULL) {
		fprintf(stderr, "Error while loading %s\n", name);
		exit(1);
	}

	if (((uintptr_t)ptr % 8) != 0) {
		fprintf(stderr, "%s alignment is corrupted - address: %p", name, ptr);
		abort();
	}
}

void cleanup() {
        if (io_handle) dlclose(io_handle);
        if (fb_handle) dlclose(fb_handle);
	if (mshal_handle) dlclose(mshal_handle);
	if (wid_handle) dlclose(wid_handle);
	if (snd_handle) dlclose(snd_handle);
	if (ply_handle) dlclose(ply_handle);
	if (screen_handle) dlclose(screen_handle);

        if (gfx) { free(gfx); gfx = NULL; }
        if (img) { free(img); img = NULL; }
	if (mshal) { free(mshal); mshal = NULL; }
	if (wid) { free(wid); wid = NULL; }
	if (snd) { free(snd); snd = NULL; }
	if (mp3p) { free(mp3p); mp3p = NULL; }
	if (fbd) { free(fbd); fbd = NULL; }
}

void ins_lib() {
	fb_handle = dlopen("/usr/lib/api/ui/libfbdev.so", RTLD_GLOBAL | RTLD_NOW);
        if (!fb_handle) {
                printf("libfbdev.so - Could not load library: %s\n", dlerror());
                cleanup();
		exit(1);
        }

	io_handle = dlopen("/usr/lib/api/io/libatmio.so", RTLD_GLOBAL | RTLD_NOW);
        if (!io_handle) {
                printf("libatmio.so - Could not load library: %s\n", dlerror());
                cleanup();
		exit(1);
        }

	mshal_handle = dlopen("/usr/lib/hal/mouse/libmshal.so", RTLD_GLOBAL | RTLD_NOW);
        if (!mshal_handle) {
                printf("libmshal.so - Could not load library: %s\n", dlerror());
                cleanup();
		exit(1);
        }

	screen_handle = dlopen("/usr/lib/hal/screen/libfbdhal.so", RTLD_GLOBAL | RTLD_NOW);
        if (!screen_handle) {
                printf("libfbdhal.so - Could not load library: %s\n", dlerror());
                cleanup();
                exit(1);
        }

	wid_handle = dlopen("/usr/lib/api/ui/libwidget.so", RTLD_GLOBAL | RTLD_NOW);
        if (!wid_handle) {
                printf("libwidget.so - Could not load library: %s\n", dlerror());
                cleanup();
                exit(1);
        }
	
	snd_handle = dlopen("/usr/lib/hal/sound/libsndhal.so", RTLD_GLOBAL | RTLD_NOW);
	if (!snd_handle) {
                printf("libsndhal.so - Could not load library: %s\n", dlerror());
                cleanup();
                exit(1);
        }

	ply_handle = dlopen("/usr/lib/api/media/libatmsndec.so", RTLD_GLOBAL | RTLD_NOW);
        if (!ply_handle) {
                printf("libatmsndec.so - Could not load library: %s\n", dlerror());
                cleanup();
                exit(1);
        }
}

void modalloc() {
	gfx = (GFX_API*)calloc(1, sizeof(GFX_API));
        img = (IMG_API*)calloc(1, sizeof(IMG_API));
	mshal = (MOUSE_HAL*)calloc(1, sizeof(MOUSE_HAL));
	wid = (WIDGET_API*)calloc(1, sizeof(WIDGET_API));
	snd = (SOUND_HAL*)calloc(1, sizeof(SOUND_HAL));
	fbd = (FBDEV_HAL*)calloc(1, sizeof(FBDEV_HAL));
	mp3p = (MP3_PLAYER_API*)calloc(1, sizeof(MP3_PLAYER_API));
}

void get_hal() {
	mshal->OpenMouse = (MouseOpen_t)dlsym(mshal_handle, "hal_msdevinit");
	mshal->ReadMouse = (MouseRead_t)dlsym(mshal_handle, "hal_msdevread");
	mshal->CloseMouse = (MouseClose_t)dlsym(mshal_handle, "hal_msdevclose");
	check_align(mshal, "MOUSE_HAL");

	if (!mshal->OpenMouse || !mshal->ReadMouse || !mshal->CloseMouse) {
		cleanup();
		exit(1);
	}

	snd->InitSound = (SoundInit_t)dlsym(snd_handle, "hal_sndinit");
        snd->WritePCM = (SoundWrite_t)dlsym(snd_handle, "hal_sndwrite");
        snd->CloseSound = (SoundClose_t)dlsym(snd_handle, "hal_sndcls");
	snd->SetVol = (SoundVol_t)dlsym(snd_handle, "hal_setvol");
	check_align(snd, "SOUND_HAL");

	if (!snd->InitSound || !snd->WritePCM || !snd->CloseSound || !snd->SetVol) {
                cleanup();
                exit(1);
        }

	fbd->RefreshFbCard = (ScreenDev_Refresh_t)dlsym(screen_handle, "hal_fbrefresh");
        fbd->ResetFbCard = (ScreenDev_Reset_t)dlsym(screen_handle, "hal_fbreset");
        fbd->InitFbCard = (ScreenInit_t)dlsym(screen_handle, "hal_fbinit");
	fbd->FreeFbMem = (ScreenFree_t)dlsym(screen_handle, "hal_fbexit");
	fbd->FbFlip = (PageFlip_t)dlsym(screen_handle, "hal_drmpgflip");
	check_align(fbd, "FBDEV_HAL");
	if (!fbd->RefreshFbCard || !fbd->ResetFbCard || !fbd->InitFbCard || !fbd->FreeFbMem || !fbd->FbFlip) {
                cleanup();
                exit(1);
        }
}

void get_mod() {
        gfx->DrawImage = (DrawImage_t)dlsym(fb_handle, "AutumnAPI_DrawImg");
        gfx->DrawButton = (DrawButton_t)dlsym(fb_handle, "AutumnAPI_DrawButton");
        gfx->InitFb = (InitFb_t)dlsym(fb_handle, "AutumnAPI_Init");
        gfx->RefreshScreen = (RefreshScreen_t)dlsym(fb_handle, "AutumnAPI_FbRefresh");
        gfx->DrawGif = (DrawGif_t)dlsym(fb_handle, "AutumnAPI_DrawGifAni");
        gfx->Clear = (Clear_t)dlsym(fb_handle, "AutumnAPI_ClearFb");
        gfx->Text = (Text_t)dlsym(fb_handle, "AutumnAPI_DrawString");
        gfx->DrawRectangle = (DrawRectangle_t)dlsym(fb_handle, "AutumnAPI_DrawRect");
	gfx->DrawLine = (DrawLine_t)dlsym(fb_handle, "AutumnAPI_DrawLine");
	check_align(gfx, "GFX_API");
	
        if (!gfx->DrawImage || !gfx->DrawButton || !gfx->InitFb || !gfx->RefreshScreen ||
            !gfx->DrawGif || !gfx->Clear || !gfx->Text || !gfx->DrawRectangle || !gfx->DrawLine) {
                perror("dlsym - libfbdev.so");
                cleanup();
		exit(1);
        }

        img->LdGif = (LdGif_t)dlsym(io_handle, "AutumnAPI_LoadGif");
        img->LdImg = (LdImg_t)dlsym(io_handle, "AutumnAPI_LoadImg");
        img->CountGif = (CountGif_t)dlsym(io_handle, "AutumnAPI_GetFrameCount");
	check_align(img, "IMG_API");

        if (!img->LdGif || !img->LdImg || !img->CountGif) {
                perror("dlsym - libatmio.so");
		cleanup();
		exit(1);
        }

	wid->SetBtnEnv = (Button_Set_t)dlsym(wid_handle, "Button_Set");
        wid->SpawnButton = (CreateButton_t)dlsym(wid_handle, "CreateButton");
	wid->SpawnElTimer = (CreateElapsedTimer_t)dlsym(wid_handle, "CreateElapsedTimer");
	wid->IsTouchEvent = (ChkTouchEv_t)dlsym(wid_handle, "ChkTouchEv");
	check_align(wid, "WIDGET_API");

	if (!wid->SetBtnEnv || !wid->SpawnButton || !wid->SpawnElTimer || !wid->IsTouchEvent) {
                perror("dlsym - libwidget.so");
                cleanup();
                exit(1);
        }

	mp3p->LoadMp3 = (LoadFile_t)dlsym(ply_handle, "AutumnAPI_LoadMp3");
	mp3p->GetFileTime = (PlayTime_t)dlsym(ply_handle, "AutumnAPI_PlayTime");
	mp3p->Play = (PlayAudio_t)dlsym(ply_handle, "AutumnAPI_PlayAudio");
	mp3p->Stop = (Stop_t)dlsym(ply_handle, "AutumnAPI_StopAudio");
	mp3p->Pause = (Pause_t)dlsym(ply_handle, "AutumnAPI_Pause");
	mp3p->Status = (GetPlayingStatus_t)dlsym(ply_handle, "AutumnAPI_GetPlayingStatus");
	mp3p->Resume = (Resume_t)dlsym(ply_handle, "AutumnAPI_Resume");
	mp3p->Replay = (Replay_t)dlsym(ply_handle, "AutumnAPI_Replay");
	mp3p->CleanPcmMemory = (PCMCleanup_t)dlsym(ply_handle, "AutumnAPI_PCMCleanup");
	mp3p->Volume = (SetVolume_t)dlsym(ply_handle, "AutumnAPI_SetVolume");
	check_align(mp3p, "MP3_PLAYER_API");

	if (!mp3p->LoadMp3 || !mp3p->GetFileTime || !mp3p->Play || !mp3p->Pause || !mp3p->Status || !mp3p->Resume || !mp3p->Replay || !mp3p->CleanPcmMemory || !mp3p->Volume) {
                cleanup();
                exit(1);
        }
	return;
}
        
void ldinit() {
        ins_lib();
        modalloc();
        get_hal();
        get_mod();
}
