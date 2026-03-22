#include "lvgl.h"
#include "lv_drivers-v8.3.0/display/fbdev.h"
#include "lv_drivers-v8.3.0/indev/evdev.h"
#include "ui.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include "acore.h"

extern float cpu_temperature;

void update_clock(lv_timer_t * timer) {
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[10];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);

    /
    	if(objects.clocklabel != NULL) {
        lv_label_set_text(objects.clocklabel, buffer);
	}
}

void system_monitor_task(lv_timer_t * timer) {
    //cpu_temperature = read_cpu_temperature();


   // if (cpu_temperature >= CRITICAL_TEMP_THRESHOLD) {


       // lv_label_set_text_fmt(objects.current_heat, "Mevcut sıcaklık: %.1f C", cpu_temperature);


       // if (lv_obj_has_flag(objects.heat_warning, LV_OBJ_FLAG_HIDDEN)) {
        //    lv_obj_clear_flag(objects.heat_warning, LV_OBJ_FLAG_HIDDEN);
      //      lv_obj_fade_in(objects.heat_warning, 500, 0);
    //    }
  //  }

   // else {
     //   if (!lv_obj_has_flag(objects.heat_warning, LV_OBJ_FLAG_HIDDEN)) {
       //     lv_obj_fade_out(objects.heat_warning, 500, 0);
        //    lv_obj_add_flag(objects.heat_warning, LV_OBJ_FLAG_HIDDEN);
        //}
  //  }
}


int main(void) {

 
    lv_init();


    fbdev_init();


    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf1[480 * 200]; 
    static lv_color_t buf2[480 * 200]; 
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, 480 * 200);


    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = fbdev_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = 480;
    disp_drv.ver_res = 800; 
    lv_disp_drv_register(&disp_drv);
    evdev_init();
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read; 
    lv_indev_drv_register(&indev_drv);

    ui_init();
    lv_timer_create(system_monitor_task, 1000, NULL);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    

    lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_OFF);
    lv_timer_create(update_clock, 1000, NULL);

    while(1) {
        lv_timer_handler();
        usleep(5000);
        lv_tick_inc(5); 
	
    }

    return 0;
}
