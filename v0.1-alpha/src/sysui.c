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
#include "libatmchtn.h"
#include <pthread.h>
#include "button.h"
#include <stdlib.h> //getenv
#include "timer.h"

void* naina(void* arg);
bool thread_started;
extern bool is_ui;
bool virus = false;
int current_frame = 0;
int bird_frame = 0;
ButtonWidget btn;
AutumnImage* bg;
TimerWidget tm;
GifFileType* my_gif;
GifFileType* bird;
extern FbDev screen;
extern int mouse_x;
extern int mouse_y;
extern int mouse_btn1;
extern GFX_API* gfx;
extern IMG_API* img;
extern WIDGET_API* wid;
extern MP3_PLAYER_API* mp3p;
extern void _userspace_SetUserAccess();
extern void _setup_Dir();

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

void AutumnUI_LoadResources() {
        bg = img->LdImg("/usr/share/test.png");
        if (!bg) {
                perror("LoadImage");
                return;
        }
        gfx->DrawImage(&screen, bg, 0, 0);
        
	my_gif = img->LdGif("/usr/share/wp.gif");
        bird = img->LdGif("/usr/share/test.gif");
        if (!my_gif || !bird) {
                perror("LoadGif");
		return;
        }
	
	int ret = mp3p->LoadMp3("/home/autumnuser0/touch.mp3");
        if (ret < 0) {
                perror("LoadMp3");
		return;
        }

}

void AutumnUI_Prepare() {
	wid->SetBtnEnv(&btn, 250, 20, 180, 50, 0xFFD3D3D3, 0xFFE6A23C, "Apps");
}


void* AutumnUI(void* arg) {
	_setup_Dir();
	AutumnUI_LoadResources();
	AutumnUI_Prepare();
	FT_Library lib;
        FT_Face face;
	int frame_count = img->CountGif(my_gif);
        int bird_count = img->CountGif(bird);
	if (UI_SetFont(&lib, &face) == 0) {
		printf("UI: Font successfully loaded");
        }

        else {
                perror("UI");
        }

	ChatTunnel* tunn = AutumnAPI_Tunnel_Connect("mouse_pipe");
    	if (!tunn) {
        	perror("Tunnel");
        	return NULL;
    	}

	is_ui = true;

	while (1) {
		mp3p->Play();
		gfx->DrawGif(&screen, my_gif, 0, 0, current_frame); 
		mp3p->GetFileTime(&tm.elapsed, &tm.total);

        	unsigned int indicator_color = mouse_btn1 ? 0xFF00FF00 : 0xFFFFFFFF;

        	char coord_text[64];

        	snprintf(coord_text, sizeof(coord_text), "X: %d  Y: %d %s", mouse_x, mouse_y, mouse_btn1 ? "[TOUCHED]" : "");
        	gfx->Text(&screen, face, coord_text, 10, 20, 0xFFFFFFFF);
        	gfx->Text(&screen, face, "No GSM device", 10, 40, 0xFFFFFFFF);
        	wid->SpawnButton(&screen, face, &btn);
		mp3p->GetFileTime(&tm.elapsed, &tm.total);
                wid->SpawnElTimer(&screen, face, &tm);
		if (wid->IsTouchEvent(&btn, mouse_x, mouse_y, mouse_btn1)) {
			virus = true;
        	}
	
    			
		gfx->Text(&screen, face, "A", mouse_x, mouse_y, indicator_color);
		gfx->RefreshScreen(&screen);

		if (virus) {
		}

        	current_frame++;
        	if (current_frame >= frame_count) {
        		current_frame = 0;
        	}
                
		bird_frame++;
        	if (bird_frame >= bird_count) {
        		bird_frame = 0;
        	}
        
		usleep(16666);
	}
}
