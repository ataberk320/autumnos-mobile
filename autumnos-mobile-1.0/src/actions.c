#include "actions.h"
#include "ui.h"
#include "lvgl.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static lv_point_t start_point;

void action_unlock_only(lv_event_t * e) {
	lv_obj_t *screen = objects.lockscreen;
	lv_indev_t * indev = lv_indev_get_act();
	if(!indev) return;
	lv_point_t curr_point;
	lv_indev_get_point(indev, &curr_point);
	lv_event_code_t code = lv_event_get_code(e);
	if(code == LV_EVENT_PRESSED) start_point = curr_point;
	else if(code == LV_EVENT_PRESSING) {
        long int dx = curr_point.x - start_point.x;
        long int dy = curr_point.y - start_point.y;
        uint32_t dist = sqrt(dx*dx + dy*dy);
        int opacity = 255 - (dist / 2); 
        if(opacity < 0) opacity = 0;
        lv_obj_set_style_opa(screen, opacity, 0);
    }
	else if(code == LV_EVENT_RELEASED) {
        long int dx = curr_point.x - start_point.x;
        long int dy = curr_point.y - start_point.y;
        uint32_t dist = sqrt(dx*dx + dy*dy);
        if(dist > 200) {
            lv_obj_add_flag(screen, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_opa(screen, 255, 0);
        } 
        else {
            lv_obj_set_style_opa(screen, 255, 0);
        }
	}
}

void action_start_process(lv_event_t * e){
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * obj = lv_event_get_target(e);
	if(code == LV_EVENT_CLICKED) {
		if (obj == objects.phone_icon) {
			system("/usr/bin/com.autumnos.phone &");
		}
		else if(obj == objects.messages_icon) {
			system("echo 'messages process test!' > /dev/kmsg");
		}
		else if(obj == objects.camera_icon) {
			system("echo 'camera process test!' > /dev/kmsg");
		}
		else if(obj == objects.calendar_icon) {
			system("echo 'calendar process test!' > /dev/kmsg");
		}
	}
}

void action_search_on_internet(lv_event_t * e) {
	lv_obj_t * searchbar = lv_event_get_user_data(e);
	const char * query = lv_textarea_get_text(searchbar);
	
	if(strlen(query) > 0) {
		char command[512];
		snprintf(command, sizeof(command), "links http://lite.duckduckgo.com/lite/?q=%s &", query);
		system(command);
	}
}

void action_show_weather_clock_widget_settings(lv_event_t * e) {
	lv_obj_clear_flag(objects.weatherclockwidgetsettings, LV_OBJ_FLAG_HIDDEN);
	lv_obj_fade_in(objects.weatherclockwidgetsettings, 300, 0);
}
