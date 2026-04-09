#include "lvgl.h"
#include "lv_drivers-v8.3.0/display/fbdev.h"
#include "lv_drivers-v8.3.0/indev/evdev.h"
//For DRM acceleration
// #include "lv_drivers-v8.3.0/display/drm.h"
#include "ui.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
//UI connected with API
#include "AutumnAPI.h"
extern const lv_img_dsc_t img_cloudy_dock;
extern const lv_img_dsc_t img_sunny_dock;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void update_autumn_weather(void) {
    FILE *fp;
    char buffer[128];

    
    fp = popen("wget -qO- http://wttr.in?format=%l,%t,%C", "r");
    
    if (fp == NULL) {
        lv_label_set_text(objects.weatherstatus, "Hata!");
        return;
    }

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        
        
        char *city = strtok(buffer, ",");
        char *unnecessary_city_buffer = strtok(NULL, ","); 
        char *unnecessary_country_buffer = strtok(NULL, ",");
        char *degree = strtok(NULL, ",");
        char *status = strtok(NULL, ",");

        // ŞEHİR
        if (sehir) {
            lv_label_set_text(objects.weathercity, city);
        }

        
        if (derece) {
            if (derece[0] == '+') derece++;
            
            char *c_pos = strstr(derece, "°C");
            if (c_pos) *c_pos = '\0'; 
            
            lv_label_set_text_fmt(objects.weatherdegree, "%s°C", degree;
        }

        
        if (status) {
            if (strstr(status, "Clear") || strstr(status, "Sunny")) {
                lv_label_set_text(objects.weatherstatus, LV_SYMBOL_IMAGE " Güneşli");
            } else if (strstr(status, "Cloud") || strstr(status, "Overcast") || strstr(status, "cloudy")) {
                lv_label_set_text(objects.weatherstatus, LV_SYMBOL_IMAGE " Bulutlu");
            } else if (strstr(status, "Rain") || strstr(status, "Drizzle")) {
                lv_label_set_text(objects.weatherstatus, LV_SYMBOL_IMAGE " Yağmurlu");
            } else {
                lv_label_set_text_fmt(objects.weatherstatus, LV_SYMBOL_IMAGE " %s", status;
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

void update_date_label(lv_obj_t * date_label, lv_obj_t * day_label) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	const char *aylar[] = {
		"Ocak",
		"Şubat",
		"Mart",
		"Nisan",
		"Mayıs",
		"Haziran"
		"Temmuz",
		"Ağustos",
		"Eylül",
		"Ekim",
		"Kasım",
		"Aralık"
	};

	const char *gunler[] = {
		"Pazar",
		"Pazartesi",
		"Salı",
		"Çarşamba",
		"Perşembe",
		"Cuma",
		"Cumartesi"
	};
	lv_label_set_text_fmt(date_label, "%d %s %d", tm.tm_mday, aylar[tm.tm_mon], tm.tm_year + 1900);
	lv_label_set_text(day_label, gunler[tm.tm_wday]);
}

void system_monitor_task(lv_timer_t * timer) {
	int bat = AutumnAPI_Read_Battery_Level();

	if (bat < 0 || bat > 100) {
		lv_bar_set_value(objects.batterybar, 0, LV_ANIM_ON);
		lv_label_set_text(objects.batterylevellabel, "%N/A");
	}
	else {
		lv_bar_set_value(objects.batterybar, bat, LV_ANIM_ON);
		lv_label_set_text_fmt(objects.batterylevellabel, "%d%%", bat);
	}

	static int last_sim_status = -1;
    int sim_status = AutumnAPI_SIM_Status();
    if (sim_status != last_sim_status) {
                switch(sim_status) {
                        case 1:
                                lv_img_set_src(objects.sim_status, objects.sim_connected);
                                break;

                        case 2:
                                lv_img_set_src(objects.sim_status, objects.no_sim);
                                break;

                        default:
                                lv_img_set_src(objects.sim_status, objects.no_sim);
                                break;

                }
    }
}

void clock_timer(lv_timer_t * timer) {
        update_clock();
        update_widget_clock();
}

void weather_timer_callback(lv_timer_t * timer) {
    update_autumn_weather();
}

void ui_date_timer(lv_timer_t * timer) {
	update_date_label(objects.datelabel, objects.daylabel);
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
	//For DRM (optional)
	//disp_drv.flush_cb = drm_flush;
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
    lv_timer_create(system_monitor_task, 500, NULL);
	lv_timer_t * date_timer = lv_timer_create(ui_date_timer, 1000, NULL);
    while(1) {
        lv_timer_handler();
        usleep(15000);
        lv_tick_inc(15);
    }
    return 0;
}

