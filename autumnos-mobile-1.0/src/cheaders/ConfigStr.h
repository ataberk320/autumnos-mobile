#ifndef CONF_H
#define CONF_H

#define AutumnUI_Config_Path "/etc/autumn_conf/sysui.cfg"

typedef enum {
        ATM_LANG_TR,
        ATM_LANG_EN,
        ATM_LANG_RU,
        ATM_LANG_KOR,
        ATM_LANG_JP,
        ATM_LANG_FR,
        ATM_LANG_ITA,
        ATM_LANG_CHI,
        ATM_LANG_DF
} Autumn_LangList;

enum {
        WALLPAPER_1,
        WALLPAPER_2,
        WALLPAPER_3,
        WALLPAPER_4,
        WALLPAPER_OTHERSOURCE
} Autumn_WallpaperList;

typedef struct {
	int current_language;
	int current_wallpaper;
	char custom_path[128];
} AutumnConfig;

extern AutumnConfig settings;

void AutumnUI_Save_SysConfig();
void AutumnUI_Load_Settings_To_SysConfig();

#endif
