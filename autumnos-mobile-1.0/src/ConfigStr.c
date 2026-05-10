#include "ConfigStr.h"
#include <stdio.h>
#include <string.h>

AutumnConfig settings;

void AutumnUI_Save_SysConfig() {
    FILE *f = fopen(AutumnUI_Config_Path, "wb");
    if (f) {
        fwrite(&settings, sizeof(AutumnConfig), 1, f);
        fclose(f);
    }
}

void AutumnUI_Load_Settings_To_SysConfig() {
    FILE *f = fopen(AutumnUI_Config_Path, "rb");
    if (f) {
        fread(&settings, sizeof(AutumnConfig), 1, f);
        fclose(f);
    } else {
        settings.current_language = ATM_LANG_TR;
        settings.current_wallpaper = WALLPAPER_1;
        memset(settings.custom_path, 0, 128);
        AutumnUI_Save_SysConfig();
    }
}
