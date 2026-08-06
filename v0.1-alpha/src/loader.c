#include "table.h"
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>


extern bool is_alsa;

__attribute__((visibility("default"))) GFX_API* gfx = NULL;
__attribute__((visibility("default"))) IMG_API* img = NULL;
__attribute__((visibility("default"))) WIDGET_API* wid = NULL;
__attribute__((visibility("default"))) MOUSE_HAL* mshal = NULL;
__attribute__((visibility("default"))) SOUND_HAL* snd = NULL;
__attribute__((visibility("default"))) FBDEV_HAL* fbd = NULL;
__attribute__((visibility("default"))) MP3_PLAYER_API* mp3p = NULL;
__attribute__((visibility("default"))) SYSPOWER_HAL* pwr = NULL;
__attribute__((visibility("default"))) ETHERNET_HAL* eth = NULL;
__attribute__((visibility("default"))) CONNECTION_API* conn = NULL;
__attribute__((visibility("default"))) UART_HAL* uart = NULL;
__attribute__((visibility("default"))) MODEM_HAL* modem = NULL;
__attribute__((visibility("default"))) ADV_WIDGET_API* uiw = NULL;
__attribute__((visibility("default"))) PALETTE_API* plt = NULL;

static void *fb_handle = NULL;
static void *io_handle = NULL;
static void *mshal_handle = NULL;
static void *wid_handle = NULL;
static void *snd_handle = NULL;
static void *ply_handle = NULL;
static void *screen_handle = NULL;
static void *power_handle = NULL;
static void *ethernet_handle = NULL;
static void *connection_handle = NULL;
static void *uart_handle = NULL;
static void *modem_handle = NULL;
static void *adv_widg_handle = NULL;
static void *plt_handle = NULL;

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
	if (power_handle) dlclose(power_handle);
	if (ethernet_handle) dlclose(ethernet_handle);
	if (connection_handle) dlclose(connection_handle);
	if (uart_handle) dlclose(uart_handle);
	if (modem_handle) dlclose(modem_handle);
	if (adv_widg_handle) dlclose(adv_widg_handle);
	if (plt_handle) dlclose(plt_handle);

        if (gfx) { free(gfx); gfx = NULL; }
        if (img) { free(img); img = NULL; }
	if (mshal) { free(mshal); mshal = NULL; }
	if (wid) { free(wid); wid = NULL; }
	if (snd) { free(snd); snd = NULL; }
	if (mp3p) { free(mp3p); mp3p = NULL; }
	if (fbd) { free(fbd); fbd = NULL; }
	if (pwr) { free(pwr); pwr = NULL; }
	if (eth) { free(eth); eth = NULL; }
	if (conn) { free(conn); conn = NULL; }
	if (uart) { free(uart); uart = NULL; }
	if (modem) { free(modem); modem = NULL; }
	if (uiw) { free(modem); uiw = NULL; }
	if (plt) { free(plt); plt = NULL; }
}

void ins_lib() {
	fb_handle = dlopen("/usr/lib/api/ui/libfbdev.so", RTLD_GLOBAL | RTLD_NOW);
        if (!fb_handle) {
                printf("libfbdev.so - Could not load library: %s\n", dlerror());
                cleanup();
		exit(1);
        }

	adv_widg_handle = dlopen("/usr/lib/api/ui/libatmui.so", RTLD_GLOBAL | RTLD_NOW);
        if (!adv_widg_handle) {
                printf("libatmui.so - Could not load library: %s\n", dlerror());
                cleanup();
                exit(1);
        }

	io_handle = dlopen("/usr/lib/api/io/libatmio.so", RTLD_GLOBAL | RTLD_NOW);
        if (!io_handle) {
                printf("libatmio.so - Could not load library: %s\n", dlerror());
                cleanup();
		exit(1);
        }

	modem_handle = dlopen("/usr/lib/hal/ethernet/libsimhal.so", RTLD_GLOBAL | RTLD_NOW);
        if (!modem_handle) {
                printf("libsimhal.so - Could not load library: %s\n", dlerror());
                cleanup();
                exit(1);
        }

	uart_handle = dlopen("/usr/lib/hal/con/libuarthal.so", RTLD_GLOBAL | RTLD_NOW);
        if (!uart_handle) {
                printf("libuarthal.so - Could not load library: %s\n", dlerror());
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

	power_handle = dlopen("/usr/lib/hal/system/libpwrhal.so", RTLD_GLOBAL | RTLD_NOW);
        if (!power_handle) {
                printf("libpwrhal.so - Could not load library: %s\n", dlerror());
                cleanup();
                exit(1);
        }

	ethernet_handle = dlopen("/usr/lib/hal/ethernet/libethal.so", RTLD_GLOBAL | RTLD_NOW);
        if (!ethernet_handle) {
                printf("libethal.so - Could not load library: %s\n", dlerror());
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

	connection_handle = dlopen("/usr/lib/api/ui/libatmeth.so", RTLD_GLOBAL | RTLD_NOW);
        if (!connection_handle) {
                printf("libatmeth.so - Could not load library: %s\n", dlerror());
                cleanup();
                exit(1);
        }

	plt_handle = dlopen("/usr/lib/api/ui/libpalette.so", RTLD_GLOBAL | RTLD_NOW);
        if (!plt_handle) {
                printf("libpalette.so - Could not load library: %s\n", dlerror());
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
	eth = (ETHERNET_HAL*)calloc(1, sizeof(ETHERNET_HAL));
	pwr = (SYSPOWER_HAL*)calloc(1, sizeof(SYSPOWER_HAL));
	conn = (CONNECTION_API*)calloc(1, sizeof(CONNECTION_API));
	uart = (UART_HAL*)calloc(1, sizeof(UART_HAL));
	modem = (MODEM_HAL*)calloc(1, sizeof(MODEM_HAL));
	uiw = (ADV_WIDGET_API*)calloc(1, sizeof(ADV_WIDGET_API));
	plt = (PALETTE_API*)calloc(1, sizeof(PALETTE_API));
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
	        snd->CompSndInit = (CompatibleSubsInit_t)dlsym(snd_handle, "hal_compsndinit");
        snd->CompSndWrite = (CompatibleSubsWrite_t)dlsym(snd_handle, "hal_compsndwrite");
        snd->CompSndCls = (CompatibleSubsCls_t)dlsym(snd_handle, "hal_compsndcls");
        snd->SetVol = (SoundVol_t)dlsym(snd_handle, "hal_setvol");
        check_align(snd, "SOUND_HAL");

        if (!snd->InitSound || !snd->WritePCM || !snd->CloseSound || !snd->SetVol || !snd->CompSndInit || !snd->CompSndWrite || !snd->CompSndCls) {
                cleanup();
                exit(1);
        } //added modern sound subsystem functions
	

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

	pwr->Reboot = (Reboot_t)dlsym(power_handle, "hal_reboot");
	pwr->Shutdown = (PowerOff_t)dlsym(power_handle, "hal_pwroff");
	pwr->EmergencyShutdown = (EmergencyButton_t)dlsym(power_handle, "hal_emergency_pwroff");
	pwr->BatPercent = (GetPSupplyPercent_t)dlsym(power_handle, "hal_getcpc");
	check_align(pwr, "SYSPOWER_HAL");

	if (!pwr->Reboot || !pwr->Shutdown || !pwr->EmergencyShutdown || !pwr->BatPercent) {
                cleanup();
                exit(1);
        }

	eth->InterfaceSetup = (EthernetIntCtl_t)dlsym(ethernet_handle, "hal_ethinterfacectl");
	eth->SetGateway = (EthernetSetGateway_t)dlsym(ethernet_handle, "hal_setgateway");

	if (!eth->InterfaceSetup || !eth->SetGateway) {
                cleanup();
                exit(1);
        }

	uart->Connect = (UartInit_t)dlsym(uart_handle, "hal_uartconnect");
	uart->Send = (UartSendCmd_t)dlsym(uart_handle, "hal_uartsend");
        uart->Read = (UartReadBuf_t)dlsym(uart_handle, "hal_uartrd");

	if (!uart->Connect || !uart->Send || !uart->Read) {
                cleanup();
                exit(1);
        }
	
	modem->Test = (ModemTest_t)dlsym(modem_handle, "hal_modemok");
	modem->Init = (ModemInit_t)dlsym(modem_handle, "hal_modeminit");
	modem->SendM = (SendMsg_t)dlsym(modem_handle, "hal_sendsms");
	modem->Dial = (Dial_t)dlsym(modem_handle, "hal_modemcall");
	modem->Answer = (Answer_t)dlsym(modem_handle, "hal_callanswer");
	modem->Decline = (Decline_t)dlsym(modem_handle, "hal_callreject");
	modem->PinCheck = (IsPin_t)dlsym(modem_handle, "hal_modemcheckpin");
	
	if (!modem->Test || !modem->Init || !modem->SendM || !modem->Dial || !modem->Answer || !modem->Decline || !modem->PinCheck) {
                cleanup();
                exit(1);
        }
}

void get_mod() {
        gfx->DrawImage = (DrawImage_t)dlsym(fb_handle, "AutumnAPI_DrawImg");
        gfx->DrawButton = (DrawButton_t)dlsym(fb_handle, "AutumnAPI_DrawButton");
        gfx->InitFb = (InitFb_t)dlsym(fb_handle, "AutumnAPI_Init");
        gfx->RefreshScreen = (RefreshScreen_t)dlsym(fb_handle, "AutumnAPI_FbRefresh");
	gfx->EyeShield = (EyeSh_t)dlsym(fb_handle, "AutumnAPI_EyeShield");
        gfx->DrawGif = (DrawGif_t)dlsym(fb_handle, "AutumnAPI_DrawGifAni");
        gfx->Clear = (Clear_t)dlsym(fb_handle, "AutumnAPI_ClearFb");
        gfx->Text = (Text_t)dlsym(fb_handle, "AutumnAPI_DrawString");
        gfx->DrawRectangle = (DrawRectangle_t)dlsym(fb_handle, "AutumnAPI_DrawRect");
	gfx->DrawLine = (DrawLine_t)dlsym(fb_handle, "AutumnAPI_DrawLine");
	gfx->GetStrWidth = (GetStringWidth_t)dlsym(fb_handle, "AutumnAPI_GetStringWidth");
	gfx->DrawWindow = (DrawWindow_t)dlsym(fb_handle, "AutumnAPI_DrawWindowBody");
	gfx->DrawTextbox = (DrawTextbox_t)dlsym(fb_handle, "AutumnAPI_DrawTextbox");
	check_align(gfx, "GFX_API");
	
        if (!gfx->DrawImage || !gfx->DrawButton || !gfx->InitFb || !gfx->RefreshScreen ||
            !gfx->DrawGif || !gfx->Clear || !gfx->Text || !gfx->DrawRectangle || !gfx->DrawLine || !gfx->DrawWindow) {
                perror("dlsym - libfbdev.so");
                cleanup();
		exit(1);
        }

	plt->ApplyGrad = (GetGradientColor_t)dlsym(plt_handle, "AutumnAPI_GetGradientColor");
	if (!plt->ApplyGrad) {
                perror("dlsym - libpalette.so");
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

	conn->On = (EthOn_t)dlsym(connection_handle, "AutumnAPI_EthOn");
	conn->Off = (EthOff_t)dlsym(connection_handle, "AutumnAPI_EthOff");
	
	if (!conn->On || !conn->Off) {
                perror("dlsym - libatmeth.so");
                cleanup();
                exit(1);
        }
	
	uiw->MessageBox = (DrawMsgBox_t)dlsym(adv_widg_handle, "AutumnAPI_DrawWindow");

	return;
}
        
void ldinit() {
        ins_lib();
        modalloc();
        get_hal();
        get_mod();
}
