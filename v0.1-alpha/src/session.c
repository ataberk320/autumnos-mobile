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
#include <stdlib.h>
#include "timer.h"
#include "sigf.h"
#include <sys/stat.h>

bool is_alsa;
bool is_ui = false;
int loadanim_frame = 0;
FbDev screen;
GifFileType* load;
ChatTunnel* tunn = NULL;
int mouse_x = 100;
int mouse_y = 100;
int mouse_btn1 = 0;
extern GFX_API* gfx;
extern IMG_API* img;
extern CONNECTION_API* conn;
extern void* inputd_main(void* arg);
extern void* AutumnUI(void* arg);
pthread_mutex_t mouse_mutex = PTHREAD_MUTEX_INITIALIZER; //for prevent blocking

void _setup_Dir(void) {
	const char *dirs[] = {
		"/home/autumnuser0",
		"/home/autumnuser0/audio"
	};
	
	for (int i = 0; i < 2; i++) {
		chown(dirs[i], 1000, 1000); //only user access!
	}
}

void _userspace_StartSrv() {
	pthread_t tid;
	if (pthread_create(&tid, NULL, inputd_main, NULL) != 0) {
		perror("input-daemon");
		return;
	}
}

void _userspace_StartUI() {
	pthread_t uid;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 128 * 1024); //setting stack for prevent sigsegv

	if (pthread_create(&uid, &attr, AutumnUI, NULL) != 0) {
                perror("System UI");
		pthread_attr_destroy(&attr);
                return;
        }

	pthread_attr_destroy(&attr);
	pthread_detach(uid);
}

int main() {
	set_sig(); //sig handler
	if (access("/dev/snd/pcmC0D0p", F_OK) == 0) {
		is_alsa = true;
	}
	else {
		is_alsa = false;
	}
	
	ldinit(); //filling pointers (GFX_API etc.)
	
	tunn = AutumnAPI_Tunnel_Create("mouse_pipe");
        if (tunn == NULL) {
                perror("inputd - ChatTunnel");
                return 1;
        }

	if (gfx->InitFb(&screen, "/dev/dri/card0") != 0) {
        	perror("FbInit");
        	return 1;
    	}

	int ret = conn->On("eth0"); //change it as you wish
	if (ret < 0) {
		perror("Connection");
	}
	else {
		printf("[SYSTEM]: Internet connection enabled.");
	}

	load = img->LdGif("/usr/share/loading.gif");
        if (!load) {
                perror("LoadGif");
                return 1;
        }
	
        int load_count = img->CountGif(load);
	
        _userspace_StartSrv();
	_userspace_StartUI(); //defined as pthread so UI can use /usr/bin/session's 
	
	while (!is_ui) {
                gfx->Clear(&screen, 0x00000000); //DARK
                gfx->DrawGif(&screen, load, (480 - 256) / 2, (800 - 256) / 2, loadanim_frame); //boot animation
                gfx->RefreshScreen(&screen); // to write
                loadanim_frame++;
                
		if (loadanim_frame >= load_count) {
                        loadanim_frame = 0;
                }
        }	

    	while (1) {
		char m_buffer[64];

        	int ret = AutumnAPI_Tunnel_ReceiveFromFriend(tunn, m_buffer); //reading mouse data
		if (ret > 0) {
			if (strncmp(m_buffer, "TOUCH_EVENT", 11) == 0) {
				pthread_mutex_lock(&mouse_mutex);
				sscanf(m_buffer, "TOUCH_EVENT X:%d Y:%d T:%d", &mouse_x, &mouse_y, &mouse_btn1); //read coordinates and detect touch events
				pthread_mutex_unlock(&mouse_mutex);
                	}

            		if (mouse_x < 0) mouse_x = 0; //hardcoded value!!
            		if (mouse_x > 479) mouse_x = 479;
            		if (mouse_y < 0) mouse_y = 0;
            		if (mouse_y > 799) mouse_y = 799;
        	}
		usleep(1000);
    	}
    	return 0;
}
