#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "table.h"
#include "libatmchtn.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <linux/input.h>

extern int mouse_x;
extern int mouse_y;
extern int mouse_btn1;
extern int mouse_btn2;
extern MOUSE_HAL* mshal; //added missing pointer struct
extern ChatTunnel* tunn;

void* inputd_main(void* arg) {
    	while (tunn == NULL) {
        	usleep(1000); 
        	printf("waiting for tunnel...\n"); //for prevent to sigsegv
    	}

    	int fd = mshal->OpenMouse();
    	if (fd < 0) {
        	perror("inputd - OpenMouse");
        	return NULL;
    	}

    	struct input_event ev;
    
		static int current_x = 240;  
		static int current_y = 400;
		static int current_touch = 0;

		char buffer[128];

		while (1) {
    		if (mshal->ReadMouse(fd, &ev) == sizeof(struct input_event)) {

        		if (ev.type == EV_ABS) {
            			if (ev.code == ABS_X) {
                			current_x = (ev.value * 480) / 32767; //hardcoded values will be edited!
            			}
            			else if (ev.code == ABS_Y) {
                			current_y = (ev.value * 800) / 32767;
            			}
        		}
        		else if (ev.type == EV_KEY) { 
            			if (ev.code == BTN_TOUCH || ev.code == BTN_LEFT) {
                			current_touch = ev.value;
            			}
        		}

        		if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
            			sprintf(buffer, "TOUCH_EVENT X:%d Y:%d T:%d", current_x, current_y, current_touch); //writing data to tunnel in a single block for prevent blocking 
            			AutumnAPI_Tunnel_Send_Msg(tunn, buffer); //writing to tunnel on session
        		}
			}
		}
    	close(fd);
    	return NULL;
}
