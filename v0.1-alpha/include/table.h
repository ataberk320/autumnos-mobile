#ifndef API_H
#define API_H

#include "libfbdev.h"
#include <gif_lib.h>
#include "AutumnImage.h"
#include "libatmchtn.h"
#include <linux/input.h>
#include "button.h"
#include "sound.h"
#include "timer.h"
#include "window.h"
#include <hb.h>
#include <openssl/ssl.h>

typedef void (*DrawLine_t)(FbDev*, int, int, int, int, uint32_t);
typedef void (*DrawImage_t)(FbDev*, AutumnImage*, int, int);
typedef void (*DrawButton_t)(FbDev*, FT_Face, int, int, int, int, int, uint32_t, const char*, uint32_t, AutumnImage*);
typedef int (*InitFb_t)(FbDev*, const char*);
typedef int (*RefreshScreen_t)(FbDev*);
typedef bool (*EyeSh_t)(FbDev*);
typedef void (*DrawGif_t)(FbDev*, GifFileType*, int, int, int);
typedef int (*CountGif_t)(GifFileType*);
typedef void (*Clear_t)(FbDev*, uint32_t);
typedef void (*Text_t)(FbDev*, FT_Face, const char*, int, int, uint32_t);
typedef void (*DrawRectangle_t)(FbDev*, int, int, int, int, uint32_t, int);
typedef uint32_t (*GetGradientColor_t)(uint32_t, int, int, int);
typedef void (*DrawWindow_t)(FbDev*, int, int, int, int, int, uint32_t, uint32_t);
typedef int (*GetStringWidth_t)(FT_Face, const char*);
typedef GifFileType* (*LdGif_t)(const char*);
typedef AutumnImage* (*LdImg_t)(const char*);
typedef uint32_t (*utf8_t)(const char** s);
typedef ChatTunnel* (*Create_t)(const char*, int);
typedef bool (*Receive_t)(ChatTunnel *, char *, int);
typedef bool (*Send_t)(ChatTunnel *, const char *, int);
typedef void (*GetMod_t)();
typedef int (*MouseOpen_t)(void);
typedef int (*MouseRead_t)(int, struct input_event*);
typedef void (*MouseClose_t)(int);
typedef void (*Button_Set_t)(ButtonWidget*, int, int, int, int, unsigned int, unsigned int, const char*);
typedef void (*CreateButton_t)(FbDev*, FT_Face, ButtonWidget*);
typedef int (*SoundInit_t)(shw*, int, int);
typedef int (*SoundWrite_t)(shw*, const short*, int);
typedef void (*SoundVol_t)(shw*, int); 
typedef void (*SoundClose_t)(shw*);
typedef int (*LoadFile_t)(const char*);
typedef int (*PlayAudio_t)(void);
typedef int (*Pause_t)(void);
typedef int (*Resume_t)(void);
typedef int (*Replay_t)(void);
typedef void (*PCMCleanup_t)(void); 
typedef void (*SetVolume_t)(int);
typedef void (*PlayTime_t)(int*, int*);
typedef int (*GetPlayingStatus_t)(void);
typedef void (*CreateElapsedTimer_t)(FbDev*, FT_Face, TimerWidget*);
typedef int (*ChkTouchEv_t)(ButtonWidget*, int, int, int);
typedef int (*Stop_t)(void);
typedef void (*ScreenDev_Refresh_t)(DRMStruct*);
typedef void (*ScreenDev_Reset_t)(DRMStruct*, int);
typedef int (*ScreenInit_t)(DRMStruct*, const char*);
typedef void (*ScreenFree_t)(int, void*, size_t);
typedef void (*PageFlip_t)(DRMStruct*);
typedef int (*EthernetIntCtl_t)(const char *, unsigned int, int);
typedef void (*EthernetSetGateway_t)(int, const char *, const char *);
typedef void (*Reboot_t)(void);
typedef void (*PowerOff_t)(void);
typedef void (*EmergencyButton_t)(void);
typedef int (*GetPSupplyPercent_t)(void);
typedef int (*EthOn_t)(const char *);
typedef int (*EthOff_t)(const char *);
typedef void (*UartSendCmd_t)(const char *);
typedef int (*UartReadBuf_t)(char*, int);
typedef int (*UartInit_t)(const char* dev);
typedef int (*ModemTest_t)();
typedef void (*ModemInit_t)();
typedef void (*SendMsg_t)(const char*, const char*);
typedef void (*Dial_t)(const char*);
typedef void (*Answer_t)();
typedef void (*Decline_t)();
typedef int (*IsPin_t)();
typedef void (*DrawMsgBox_t)(FbDev *, Window *, FT_Face, int);
typedef void (*DrawTextbox_t)(FbDev*, FT_Face, int, int, int, int, int, const char*, const char*, uint32_t);
typedef uint32_t (*UnicodeTF8_t)(const char**, hb_font_t *);
typedef void (*ShapeCompx_t)(hb_font_t *, const char *);
typedef uint32_t (*AlphaBlend_t)(FbDev *, int, int, uint32_t, int);
typedef void (*Anim_FadeIn_t)(void*, void*, int);
typedef void (*Anim_FadeOut_t)(void*, void*, int);
typedef int (*SpawnLayer_t)(int *, uint32_t **, int, int, int, int, int);
typedef int (*EnableTLS_t)();
typedef void (*HostnameConf_t)(SSL *, const char *);
typedef char* (*CreateRequest_t)(const char *, const char *, int);
typedef int (*ParseResponse_t)(const char *, char *, int);


typedef struct {
	UartInit_t Connect;
	UartSendCmd_t Send;
	UartReadBuf_t Read;
} __attribute__((aligned(8))) UART_HAL;

typedef struct {
	ModemTest_t Test;
	ModemInit_t Init;
	SendMsg_t SendM;
	Dial_t Dial;
	Answer_t Answer;
	Decline_t Decline;
	IsPin_t PinCheck;
} __attribute__((aligned(8))) MODEM_HAL;

typedef struct {
	UnicodeTF8_t ConvertToUTF;
	ShapeCompx_t HarfbuzzShape;
} __attribute__((aligned(8))) NATIVELS_API;

typedef struct {
	Reboot_t Reboot;
	PowerOff_t Shutdown;
	EmergencyButton_t EmergencyShutdown;
	GetPSupplyPercent_t BatPercent;
} __attribute__((aligned(8))) SYSPOWER_HAL;

typedef struct {
	EthernetIntCtl_t InterfaceSetup;
	EthernetSetGateway_t SetGateway;
} __attribute__((aligned(8))) ETHERNET_HAL;

typedef struct {
	DrawLine_t DrawLine;
	DrawImage_t DrawImage;
	DrawButton_t DrawButton;
	InitFb_t InitFb;
	RefreshScreen_t RefreshScreen;
	EyeSh_t EyeShield;
	DrawGif_t DrawGif;
	Clear_t Clear;
	Text_t Text;
	DrawRectangle_t DrawRectangle;
	DrawWindow_t DrawWindow;
	GetStringWidth_t GetStrWidth;
	DrawTextbox_t DrawTextbox;
} __attribute__((aligned(8))) GFX_API;

typedef struct {
	GetGradientColor_t ApplyGrad;
	AlphaBlend_t Blend;
} __attribute__((aligned(8))) PALETTE_API;

typedef struct {
	Create_t Create;
	Receive_t Receive;
	Send_t Send;
} __attribute__((aligned(8))) CHAT_TUN_API;

typedef struct {
	LdGif_t LdGif;
	LdImg_t LdImg;
	CountGif_t CountGif;
} __attribute__((aligned(8))) IMG_API;

typedef struct {
	MouseOpen_t OpenMouse;
	MouseRead_t ReadMouse;
	MouseClose_t CloseMouse;
} __attribute__((aligned(8))) MOUSE_HAL;

typedef struct {
	SoundInit_t InitSound;
	SoundWrite_t WritePCM;
	SoundClose_t CloseSound;
	SoundVol_t SetVol;
} __attribute__((aligned(8))) SOUND_HAL;

typedef struct {
	ScreenDev_Refresh_t RefreshFbCard;
	ScreenDev_Reset_t ResetFbCard;
	ScreenInit_t InitFbCard;
	ScreenFree_t FreeFbMem;
	PageFlip_t FbFlip;
} __attribute__((aligned(8))) FBDEV_HAL;

typedef struct {
	Button_Set_t SetBtnEnv;
	CreateButton_t SpawnButton;
	ChkTouchEv_t IsTouchEvent;
	CreateElapsedTimer_t SpawnElTimer;
} __attribute__((aligned(8))) WIDGET_API;

typedef struct {
	DrawMsgBox_t MessageBox;
	Anim_FadeIn_t FadeIn;
	Anim_FadeOut_t FadeOut;
} __attribute__((aligned(8))) ADV_WIDGET_API;

typedef struct {
	LoadFile_t LoadMp3;
	PlayTime_t GetFileTime;
	PlayAudio_t Play;
	Stop_t Stop;
	Pause_t Pause;
	Resume_t Resume;
	Replay_t Replay;
	PCMCleanup_t CleanPcmMemory;
	SetVolume_t Volume;
	GetPlayingStatus_t Status;
} __attribute__((aligned(8))) MP3_PLAYER_API;

typedef struct {
        EthOn_t On;
        EthOff_t Off;
} __attribute__((aligned(8))) CONNECTION_API;

typedef struct {
	SpawnLayer_t SpawnLayer;
} __attribute__((aligned(8))) ASURF_API;

typedef struct {
	EnableTLS_t TLS;
	HostnameConf_t ConfigureHostname;
} __attribute__((aligned(8))) SSL_API;

typedef struct {
	CreateRequest_t CreateRequest;
	ParseResponse_t ParseResp;
} __attribute__((aligned(8))) JSON_API;

extern GFX_API* gfx;
extern IMG_API* img;
extern CHAT_TUN_API* tun;
extern MOUSE_HAL* mshal;
extern WIDGET_API* wid;
extern SOUND_HAL* snd;
extern FBDEV_HAL* fbd;
extern MP3_PLAYER_API* mp3p;
extern SYSPOWER_HAL* pwr;
extern ETHERNET_HAL* eth;
extern MODEM_HAL* modem;
extern UART_HAL* uart;
extern ADV_WIDGET_API* uiw;
extern PALETTE_API* plt;
extern NATIVELS_API* nls;
extern ASURF_API* asf;
extern SSL_API* ssl;
extern JSON_API* json;

void ldinit();

#endif
