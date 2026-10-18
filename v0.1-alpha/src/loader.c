#include "../include/alog.h"
#include <errno.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <gif_lib.h>
#include "../include/libfbdev.h"
#include "../include/item_ui/AutumnImage.h"
#include <ft2build.h>
#include "../include/SymbolTable/table.h"
#include <stdbool.h>
#include "../include/libatmchtn.h"
#include <pthread.h>
#include "../include/item_ui/button.h"
#include <stdlib.h>
#include "../include/item_ui/timer.h"
#include "../include/sigf.h"
#include <sys/stat.h>
#include <sys/utsname.h>
#include "../include/HAL/io.h"
#include "../include/HAL/autumn_ioctl.h"

static void *resolve_symbol(void *handle, const char *lib_name, const char *symbol_name) {
    if (!handle || !symbol_name) return NULL;

    dlerror();
    void *symbol = dlsym(handle, symbol_name);
    const char *err = dlerror();
    if (err != NULL) {
        fprintf(stderr, "Could not resolve %s from %s: %s\n", symbol_name, lib_name, err);
        return NULL;
    }

    return symbol;
}

static void bind_runtime_apis(void) {
    static bool bound = false;
    if (bound) return;
    bound = true;

    void *core_handle = dlopen("/usr/lib/api/libatmcore.so", RTLD_NOW | RTLD_GLOBAL);
    void *mouse_handle = dlopen("/usr/lib/hal/libmshal.so", RTLD_NOW | RTLD_GLOBAL);
    void *sound_handle = dlopen("/usr/lib/hal/libsndhal.so", RTLD_NOW | RTLD_GLOBAL);
    void *eth_handle = dlopen("/usr/lib/hal/libethal.so", RTLD_NOW | RTLD_GLOBAL);
    void *modem_handle = dlopen("/usr/lib/hal/libsimhal.so", RTLD_NOW | RTLD_GLOBAL);
    void *uart_handle = dlopen("/usr/lib/hal/libuarthal.so", RTLD_NOW | RTLD_GLOBAL);
    void *scr_handle = dlopen("/usr/lib/hal/libfbdhal.so", RTLD_NOW | RTLD_GLOBAL);

    if (!symb) return;

    // Tek tablo üzerinden tüm fonksiyon pointer'larını doğrudan bağlıyoruz
    symb->Create = (Create_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_Tunnel_Create");
    symb->Receive = (Receive_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_Tunnel_ReceiveFromFriend");
    symb->Send = (Send_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_Tunnel_Send_Msg");

    symb->TotalRAM = (GetTotalRAM_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_GetTotRAM_MB");
    symb->FreeRAM = (GetFreeRAM_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_GetFrRam_MB");
    symb->GetProc = (GetProcess_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_GetProc");
    symb->CheckStat = (CheckProcessStat_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_CheckProcess");

    symb->Action = (SystemAction_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_Reboot");
    symb->EmergencyShutdown = (EmergencyButton_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_PowerOff");
    symb->BatPercent = (GetPSupplyPercent_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_GetBatPercent");

    symb->DrawLine = (DrawLine_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_DrawLine");
    symb->DrawImage = (DrawImage_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_DrawImg");
    symb->DrawTriangle = (DrawTriangle_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_DrawTriangle");
    symb->DrawButton = (DrawButton_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_DrawButton");
    symb->InitFb = (InitFb_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_Init");
    symb->RefreshScreen = (RefreshScreen_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_FbRefresh");
    symb->EyeShield = (EyeSh_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_EyeShield");
    symb->DrawGif = (DrawGif_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_DrawGifAni");
    symb->Clear = (Clear_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_ClearFb");
    symb->Text = (Text_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_DrawString");
    symb->DrawRectangle = (DrawRectangle_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_DrawRect");
    symb->DrawWindow = (DrawWindow_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_DrawWindowBody");
    symb->GetStrWidth = (GetStringWidth_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_GetStringWidth");
    symb->DrawTextbox = (DrawTextbox_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_DrawTextbox");

    symb->ApplyGrad = (GetGradientColor_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_GetGradientColor");
    symb->Blend = (AlphaBlend_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_AlBlend");
    symb->AddShadow = (AddShadow_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_AddShadow");

    symb->LdGif = (LdGif_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_LoadGif");
    symb->LdImg = (LdImg_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_LoadImg");
    symb->CountGif = (CountGif_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_GetFrameCount");

    symb->OpenMouse = (MouseOpen_t)resolve_symbol(mouse_handle, "/usr/lib/hal/libmshal.so", "hal_msdevinit");
    symb->ReadMouse = (MouseRead_t)resolve_symbol(mouse_handle, "/usr/lib/hal/libmshal.so", "hal_msdevread");
    symb->CloseMouse = (MouseClose_t)resolve_symbol(mouse_handle, "/usr/lib/hal/libmshal.so", "hal_msdevclose");

    symb->InitSound = (SoundInit_t)resolve_symbol(sound_handle, "/usr/lib/hal/libsndhal.so", "hal_sndinit");
    symb->WritePCM = (SoundWrite_t)resolve_symbol(sound_handle, "/usr/lib/hal/libsndhal.so", "hal_sndwrite");
    symb->CloseSound = (SoundClose_t)resolve_symbol(sound_handle, "/usr/lib/hal/libsndhal.so", "hal_sndcls");
    symb->CompSndInit = (CompatibleSubsInit_t)resolve_symbol(sound_handle, "/usr/lib/hal/libsndhal.so", "hal_compsndinit");
    symb->CompSndWrite = (CompatibleSubsWrite_t)resolve_symbol(sound_handle, "/usr/lib/hal/libsndhal.so", "hal_compsndwrite");
    symb->CompSndCls = (CompatibleSubsCls_t)resolve_symbol(sound_handle, "/usr/lib/hal/libsndhal.so", "hal_compsndcls");
    symb->SetVol = (SoundVol_t)resolve_symbol(sound_handle, "/usr/lib/hal/libsndhal.so", "hal_setvol");

    symb->RefreshFbCard = (ScreenDev_Refresh_t)resolve_symbol(scr_handle, "/usr/lib/hal/libfbdhal.so", "hal_fbrefresh");
    symb->ResetFbCard = (ScreenDev_Reset_t)resolve_symbol(scr_handle, "/usr/lib/hal/libfbdhal.so", "hal_fbreset");
    symb->InitFbCard = (ScreenInit_t)resolve_symbol(scr_handle, "/usr/lib/hal/libfbdhal.so", "hal_fbinit");
    symb->FreeFbMem = (ScreenFree_t)resolve_symbol(scr_handle, "/usr/lib/hal/libfbdhal.so", "hal_fbfree");
    symb->FbFlip = (PageFlip_t)resolve_symbol(scr_handle, "/usr/lib/hal/libfbdhal.so", "hal_fbflip");

    symb->SetBtnEnv = (Button_Set_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_SetBtnEnv");
    symb->SpawnButton = (CreateButton_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_SpawnButton");
    symb->IsTouchEvent = (ChkTouchEv_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_IsTouchEvent");
    symb->SpawnElTimer = (CreateElapsedTimer_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_SpawnElTimer");

    symb->MessageBox = (DrawMsgBox_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_DrawWindow");
    symb->FadeIn = (Anim_FadeIn_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_Anim_FadeIn");
    symb->FadeOut = (Anim_FadeOut_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_Anim_FadeOut");
    symb->SetFont = (SetFont_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_SetFont");

    symb->LoadMp3 = (LoadFile_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_LoadMp3");
    symb->GetFileTime = (PlayTime_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_PlayTime");
    symb->Play = (PlayAudio_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_PlayAudio");
    symb->Stop = (Stop_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_StopAudio");
    symb->Pause = (Pause_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_Pause");
    symb->Resume = (Resume_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_Resume");
    symb->Replay = (Replay_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_Replay");
    symb->CleanPcmMemory = (PCMCleanup_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_CleanPcmMemory");
    symb->Volume = (SetVolume_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_SetVolume");
    symb->Status = (GetPlayingStatus_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_GetPlayingStatus");

    symb->EthOn = (EthOn_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_EthOn");
    symb->EthOff = (EthOff_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_EthOff");

    symb->SpawnLayer = (SpawnLayer_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_SpawnLayer");

    symb->TLS = (EnableTLS_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_TLSSec");
    symb->ConfigureHostname = (HostnameConf_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_HostnameConf");

    symb->CreateRequest = (CreateRequest_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_JSONCreateRequest");
    symb->ParseResp = (ParseResponse_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_JSONParseResp");

    symb->InterfaceSetup = (EthernetIntCtl_t)resolve_symbol(eth_handle, "/usr/lib/hal/libethal.so", "hal_ethinterfacectl");
    symb->SetGateway = (EthernetSetGateway_t)resolve_symbol(eth_handle, "/usr/lib/hal/libethal.so", "hal_setgateway");

    symb->ModemTest = (ModemTest_t)resolve_symbol(modem_handle, "/usr/lib/hal/libsimhal.so", "hal_modemok");
    symb->ModemInit = (ModemInit_t)resolve_symbol(modem_handle, "/usr/lib/hal/libsimhal.so", "hal_modeminit");
    symb->SendMsg = (SendMsg_t)resolve_symbol(modem_handle, "/usr/lib/hal/libsimhal.so", "hal_sendsms");
    symb->Dial = (Dial_t)resolve_symbol(modem_handle, "/usr/lib/hal/libsimhal.so", "hal_modemcall");
    symb->Answer = (Answer_t)resolve_symbol(modem_handle, "/usr/lib/hal/libsimhal.so", "hal_callanswer");
    symb->Decline = (Decline_t)resolve_symbol(modem_handle, "/usr/lib/hal/libsimhal.so", "hal_callreject");
    symb->IsPin = (IsPin_t)resolve_symbol(modem_handle, "/usr/lib/hal/libsimhal.so", "hal_modemcheckpin");

    symb->UartConnect = (UartInit_t)resolve_symbol(uart_handle, "/usr/lib/hal/libuarthal.so", "hal_uartconnect");
    symb->UartSend = (UartSendCmd_t)resolve_symbol(uart_handle, "/usr/lib/hal/libuarthal.so", "hal_uartsend");
    symb->UartRead = (UartReadBuf_t)resolve_symbol(uart_handle, "/usr/lib/hal/libuarthal.so", "hal_uartrd");

    symb->Reboot = (Reboot_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_Reboot");
    symb->Shutdown = (PowerOff_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_PowerOff");
    symb->Sleep = (SleepMode_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_SleepMode");
    symb->GetBatPercent = (GetBPercent_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_GetBatPercent");

    symb->ConvertToUTF = (UnicodeTF8_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_UnicodeTF8");
    symb->HarfbuzzShape = (ShapeCompx_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_ShapeCompx");

    symb->ListRF = (RfkillList_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_RfkillList");
    symb->SetBlocked = (RfkillSetBlk_t)resolve_symbol(core_handle, "/usr/lib/api/libatmcore.so", "AutumnAPI_RfkillSetBlk");
}

extern bool is_alsa;

__attribute__((visibility("default"))) SYM_TAB* symb = NULL;

#define MAX_HANDLES 128
static void* loaded_handles[MAX_HANDLES];
static int handle_count = 0;
void ins_lib(int tab_fd, const char *base_path);

void ins_lib(int tab_fd, const char *base_path) {
    DIR *dir = opendir(base_path);
    if (!dir) {
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, entry->d_name);

        struct stat st;
        int is_dir = 0;
        int is_regl = 0;

        if (entry->d_type == DT_DIR) is_dir = 1;
        else if (entry->d_type == DT_REG) is_regl = 1;
        else {
            if (stat(full_path, &st) == 0) {
                if (S_ISDIR(st.st_mode)) is_dir = 1;
                else if (S_ISREG(st.st_mode)) is_regl = 1;
            }
        }
        
        if (is_dir) {
            ins_lib(tab_fd, full_path);
        }
        else if (is_regl) {
            size_t len = strlen(entry->d_name);
            if (len > 3 && strcmp(entry->d_name + len - 3, ".so") == 0) {
                void *handle = dlopen(full_path, RTLD_GLOBAL | RTLD_NOW);
                if (!handle) {
                    printf("loader: Error: Could not load [%s]: %s\n", full_path, dlerror());
                }
                else {
                    printf("loader: Notify: Successfully loaded: %s\n", full_path);
                    if (handle_count < MAX_HANDLES) {
                        loaded_handles[handle_count++] = handle;
                    }
                }
            }
        }
    }
    closedir(dir);
}

void cleanup() {
    for (int i = handle_count - 1; i >= 0; i--) {
        if (loaded_handles[i]) {
            dlclose(loaded_handles[i]);
            loaded_handles[i] = NULL;
        }
    }
    handle_count = 0;

    if (symb) {
        memset(symb, 0, sizeof(SYM_TAB));
    }
}
        
void ldinit() {
    memset(loaded_handles, 0, sizeof(loaded_handles));
    handle_count = 0;

    int tab_fd = _AutumnSys_ioOpen("/dev/tabldev", A_RDWR, 0);
    if (tab_fd < 0) {
        a_perror("loader -> tabldev: Error");
        exit(1);
    }

    extern SYM_TAB *sym;
    symb = sym;

    if (!symb) {
        printf("loader: Error: NULL Global Symbol Table. Allocating now.\n");
        symb = (SYM_TAB*)calloc(1, sizeof(SYM_TAB));
    }

    printf("loader: Notify: Scanning for shared objects...\n");

    ins_lib(tab_fd, "/usr/lib/hal");
    ins_lib(tab_fd, "/usr/lib/api");

    printf("loader: Notify: Binding runtime symbols...\n");
    
    bind_runtime_apis();

    unsigned long glob_tab_ptr = (unsigned long)symb;
    if (ioctl(tab_fd, IOCTBLREGSRV, glob_tab_ptr) == 0) {
        printf("SYM_TAB Address mapped to Tabloader device: %p\n", (void*)glob_tab_ptr);
    }
    else {
        perror("loader: Error: ");
    }
}
