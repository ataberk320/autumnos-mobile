#include "lvgl.h"
#include "lv_drivers-v8.3.0/display/fbdev.h"
#include "lv_drivers-v8.3.0/indev/evdev.h"
#include "ui.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include "acore.h"

extern float cpu_temperature;
extern const lv_img_dsc_t img_cloudy_dock;
extern const lv_img_dsc_t img_sunny_dock;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lvgl.h"


void update_autumn_weather(void) {
    FILE *fp;
    char buffer[128];

    
    fp = popen("wget -qO- http://wttr.in?format=%l,%t,%C", "r");
    
    if (fp == NULL) {
        lv_label_set_text(objects.weatherstatus, "Hata!");
        return;
    }

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0; // Satır sonunu temizle
        
        
        char *sehir = strtok(buffer, ",");
        char *sehir_tekrar = strtok(NULL, ","); 
        char *ulke = strtok(NULL, ",");
        char *derece = strtok(NULL, ",");
        char *durum = strtok(NULL, ",");

        // ŞEHİR
        if (sehir) {
            lv_label_set_text(objects.weathercity, sehir);
        }

        
        if (derece) {
            if (derece[0] == '+') derece++;
            
            char *c_pos = strstr(derece, "°C");
            if (c_pos) *c_pos = '\0'; 
            
            lv_label_set_text_fmt(objects.weatherdegree, "%s°C", derece);
        }

        
        if (durum) {
            if (strstr(durum, "Clear") || strstr(durum, "Sunny")) {
                lv_label_set_text(objects.weatherstatus, LV_SYMBOL_IMAGE " Güneşli");
		lv_img_set_src(objects.clockground, &img_sunny_dock);
            } else if (strstr(durum, "Cloud") || strstr(durum, "Overcast") || strstr(durum, "cloudy")) {
                lv_label_set_text(objects.weatherstatus, LV_SYMBOL_IMAGE " Bulutlu");
		lv_img_set_src(objects.clockground, &img_cloudy_dock);
            } else if (strstr(durum, "Rain") || strstr(durum, "Drizzle")) {
                lv_label_set_text(objects.weatherstatus, LV_SYMBOL_IMAGE " Yağmurlu");
            } else {
                lv_label_set_text_fmt(objects.weatherstatus, LV_SYMBOL_IMAGE " %s", durum);
            }
        }
    } else {
        
        lv_label_set_text(objects.weathercity, "Bilinmiyor");
        lv_label_set_text(objects.weatherdegree, "°C");
        lv_label_set_text(objects.weatherstatus, "Bağlantı Yok");
    }
    pclose(fp);
}

void update_clock(void) {
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[10];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);

    
    	if(objects.clocklabel != NULL) {
        lv_label_set_text(objects.clocklabel, buffer);
	}
}

void update_widget_clock(void) {
	time_t rawtime;
	struct tm * timeinfo;
	char hh_buffer[3];
	char mm_buffer[3];
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	snprintf(hh_buffer, sizeof(hh_buffer), "%02d", timeinfo->tm_hour);
        snprintf(mm_buffer, sizeof(mm_buffer), "%02d", timeinfo->tm_min);

    
    	lv_label_set_text(objects.hh, hh_buffer);
        lv_label_set_text(objects.mm, mm_buffer);
}


void clock_timer(lv_timer_t * timer) {
        update_clock();
        update_widget_clock();
}

void weather_timer_callback(lv_timer_t * timer) {
    update_autumn_weather();
}

int main(void) {


    lv_init();


    fbdev_init();
	//drm_init();
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf1[480 * 200]; 
    static lv_color_t buf2[480 * 200]; 
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, 480 * 200);
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = fbdev_flush;
	//disp_drv.flush_cb = fbdev_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = 480;
	disp_drv.ver_res = 800;
    lv_disp_drv_register(&disp_drv);
    ui_init();
    evdev_init();
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;
    lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv);
    //************for test on QEMU***********
	//lv_obj_t * invisible_cursor = lv_obj_create(lv_layer_top());
	//lv_obj_set_size(invisible_cursor, 4, 4);
    //lv_obj_set_style_radius(invisible_cursor, 0, 0);
    //lv_obj_set_style_border_width(invisible_cursor, 0, 0);
    //lv_obj_set_style_pad_all(invisible_cursor, 0, 0);
    //lv_obj_clear_flag(invisible_cursor, LV_OBJ_FLAG_CLICKABLE);
    //lv_obj_set_style_bg_color(invisible_cursor, lv_color_make(255, 0, 0), 0);
    //lv_indev_set_cursor(mouse_indev, invisible_cursor);
    lv_timer_create(system_monitor_task, 1000, NULL);
    lv_timer_create(clock_timer, 5000, NULL);
    update_autumn_weather();
    lv_timer_create(weather_timer_callback, 300000, NULL);
    
    while(1) {
        lv_timer_handler();
        usleep(15000);
        lv_tick_inc(15);
    }
    return 0;
}

