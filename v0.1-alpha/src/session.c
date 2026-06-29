#include <errno.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <gif_lib.h>
#include "libfbdev.h"
#include "AutumnImage.h"
#include <ft2build.h>
#include "table.h"
#include <stdbool.h>

extern GFX_API *gfx;
extern IMG_API *img;

int UI_SetFont(FT_Library *library, FT_Face *face) {
	if (FT_Init_FreeType(library)) {
		perror("FreeType");
		return -1;
	}

	if (FT_New_Face(*library, "/usr/share/fonts/DroidSans.ttf", 0, face)) {
    		perror("DroidSans.ttf");
    		return -1;
	}

	FT_Set_Pixel_Sizes(*face, 0, 16);
	return 0;
}

int main() {
	FT_Library lib;
	FT_Face face;
	get_mod();
	FbDev screen;
	if (gfx->InitFb(&screen, "/dev/fb0") != 0) {
        	perror("UI");
        	return -1;
    	}
	if (UI_SetFont(&lib, &face) == 0) {	

	}
	else {
		perror("UI");
	}

	

	AutumnImage* bg = img->LdImg("/usr/share/test.png");
	if (!bg) {
        	perror("LoadImage");
        	return 1;
    	}
	gfx->DrawImage(&screen, bg, 0, 0);
	GifFileType* my_gif = img->LdGif("/usr/share/wp.gif");
        GifFileType* bird = img->LdGif("/usr/share/test.gif");
    	if (!my_gif) {
        	perror("LoadGif");
        	return -1;
    	}
    
    	int frame_count = img->CountGif(my_gif);
	int bird_count = img->CountGif(bird);
	while (1) { 
		for (int i = 0; i < frame_count; i++) {

        		gfx->DrawGif(&screen, my_gif, 0, 0, i);

			gfx->Text(&screen, face, "Ennerci", 10, 40, 0xFFFFFFFF);
        		gfx->DrawButton(&screen, face, 200, 150, 180, 50, 15, 0xFFD3D3D3, "bedava para", 0x00000000);
			gfx->RefreshScreen(&screen);
        
        		usleep(16666);
		}
	}
    	return 0;
}
