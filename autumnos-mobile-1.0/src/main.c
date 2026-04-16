#include "lvgl.h"
#include "lv_drivers-v8.3.0/display/fbdev.h"
#include "lv_drivers-v8.3.0/indev/evdev.h"
#include "ui.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include "AutumnAPI.h"
#include <stdbool.h>
extern float cpu_temperature;
bool michael_ee = false;
extern const lv_img_dsc_t img_cloudy_dock;
extern const lv_img_dsc_t img_sunny_dock;
extern const lv_img_dsc_t img_no_sim;
extern const lv_img_dsc_t img_sim_connected;
extern const lv_img_dsc_t img_unknown;
extern const lv_img_dsc_t img_unknown_dock;
extern const lv_img_dsc_t img_rainy_dock;
extern const lv_img_dsc_t img_rainy;
extern const lv_img_dsc_t img_sun;
extern const lv_img_dsc_t img_cloudy;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lvgl.h"

void AutumnUI_Play_Touch_Sound() {
	system("mpg123 /usr/share/touch.mp3 &");
}

static void AutumnUI_Detect_TouchEvent(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);
	
	if(code == LV_EVENT_PRESSED) {
		AutumnUI_Play_Touch_Sound();
	}
}

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
        char *sehir_tekrar = strtok(NULL, ",");
        char *ulke = strtok(NULL, ",");
        char *degree = strtok(NULL, ",");
        char *status = strtok(NULL, ",");


        if (city) {
            lv_label_set_text(objects.weathercity, city);
        }


        if (degree) {
            if (degree[0] == '+') degree++;
            char *c_pos = strstr(degree, "°C");
            if (c_pos) *c_pos = '\0'; 
            
            lv_label_set_text_fmt(objects.weatherdegree, "%s°C", degree);
        }


        if (status) {
			if (strstr(status, "Thunder") || strstr(status, "storm") || strstr(status, "lightning")) {
        		lv_label_set_text(objects.weatherstatus, LV_SYMBOL_CHARGE " Gök Gürültülü");
    		}

    		else if (strstr(status, "Snow") || strstr(status, "Sleet") || strstr(status, "Ice") || strstr(status, "snow")) {
        		lv_label_set_text(objects.weatherstatus, LV_SYMBOL_LIST " Karlı");
    		}

    		else if (strstr(status, "Mist") || strstr(status, "Fog") || strstr(status, "Haze") || strstr(status, "Smoke")) {
        		lv_label_set_text(objects.weatherstatus, LV_SYMBOL_REFRESH " Sisli");
    		}

    		else if (strstr(status, "Heavy rain") || strstr(status, "Shower") || strstr(status, "Torrential")) {
        		lv_label_set_text(objects.weatherstatus, LV_SYMBOL_DOWNLOAD " Sağanak Yağış");
   			}

    		else if (strstr(status, "Rain") || strstr(status, "Drizzle") || strstr(status, "rain") || strstr(status, "patchy")) {
        		lv_label_set_text(objects.weatherstatus, LV_SYMBOL_LIST " Yağmurlu");
				lv_img_set_src(objects.clockground, &img_rainy_dock);
				lv_img_set_src(objects.reflection, &img_rainy_dock);
				lv_img_set_src(objects.weatherstatusicon, &img_rainy);
    		}

    		else if (strstr(status, "Overcast") || strstr(status, "broken clouds")) {
        		lv_label_set_text(objects.weatherstatus, LV_SYMBOL_IMAGE " Kapalı");
    		}

    		else if (strstr(status, "Cloud") || strstr(status, "cloudy") || strstr(status, "scattered")) {
        		lv_label_set_text(objects.weatherstatus, LV_SYMBOL_IMAGE " Bulutlu");
				lv_img_set_src(objects.clockground, &img_cloudy_dock);
				lv_img_set_src(objects.reflection, &img_cloudy_dock);
				lv_img_set_src(objects.weatherstatusicon, &img_cloudy);
    		}

    		else if (strstr(status, "Clear") || strstr(status, "Sunny") || strstr(status, "clear sky")) {
        		lv_label_set_text(objects.weatherstatus, LV_SYMBOL_SETTINGS " Güneşli");
				lv_img_set_src(objects.clockground, &img_sunny_dock);
				lv_img_set_src(objects.reflection, &img_sunny_dock);
				lv_img_set_src(objects.weatherstatusicon, &img_sun);
    		}
        }
    } 
	else {

        lv_label_set_text(objects.weathercity, "Bilinmiyor");
        lv_label_set_text(objects.weatherdegree, "°C");
        lv_label_set_text(objects.weatherstatus, "Bağlantı Yok");
		lv_img_set_src(objects.clockground, &img_unknown_dock);
		lv_img_set_src(objects.reflection, &img_unknown_dock);
		if (michael_ee == true) {
			lv_img_set_src(objects.weatherstatusicon, &img_unknown);
			lv_obj_set_x(objects.weatherstatusicon,254);
			lv_obj_set_y(objects.weatherstatusicon,42);
			lv_img_set_zoom(objects.weatherstatusicon, 91);
		}
		else {
			lv_obj_add_flag(objects.weatherstatusicon, LV_OBJ_FLAG_HIDDEN);
			lv_obj_set_x(objects.weatherstatusicon,336);
            lv_obj_set_y(objects.weatherstatusicon, 130);
			lv_img_set_zoom(objects.weatherstatusicon, 255);
		}
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

	if(objects.lockscreenclocklabel != NULL) {
		lv_label_set_text(objects.lockscreenclocklabel, buffer);
	}

}


void AutixSurf_Flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
    static int shm_fd = -1; 
    static uint32_t *shm_ptr = NULL;

    if (shm_fd < 0) {
        shm_fd = shm_open("/autumn_ui", O_RDWR | O_CREAT, 0666);
        if (shm_fd >= 0) {
            shm_ptr = (uint32_t*)mmap(0, 480 * 800 * 4, PROT_WRITE, MAP_SHARED, shm_fd, 0);
        }
    }

   if (shm_ptr != MAP_FAILED && shm_ptr != NULL) {
        const int screen_width = 480;
        const int screen_height = 800;

        for (int y = area->y1; y <= area->y2; y++) {
            if (y < 0 || y >= screen_height) continue;
            uint32_t *dest_row = &shm_ptr[y * screen_width + area->x1];
            int width = (area->x2 - area->x1 + 1);
            if (area->x1 + width > screen_width) {
                width = screen_width - area->x1;
            }
            memcpy(dest_row, color_p, width * sizeof(uint32_t));
            color_p += width;
        }
    }

    lv_disp_flush_ready(disp_drv);
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

void update_date_label(lv_obj_t * date_label, lv_obj_t * day_label, lv_obj_t * lockscreen_datelabel, lv_obj_t * lockscreen_daylabel) {
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
	lv_label_set_text_fmt(lockscreen_datelabel, "%d %s %d", tm.tm_mday, aylar[tm.tm_mon], tm.tm_year + 1900);
	lv_label_set_text(lockscreen_daylabel, gunler[tm.tm_wday]);
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
				lv_img_set_src(objects.sim_status, &img_sim_connected);
				break;
		
			case 2: 
				lv_img_set_src(objects.sim_status, &img_no_sim);
				break;
		
			default:
				lv_img_set_src(objects.sim_status, &img_no_sim);
				break;

		}
	last_sim_status = sim_status;
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
	update_date_label(objects.datelabel, objects.daylabel, objects.lockscreendatelabel, objects.lockscreendaylabel);
}

int main(void) {
    lv_init();
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf1[480 * 200]; 
    static lv_color_t buf2[480 * 200]; 
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, 480 * 200);
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = AutixSurf_Flush;
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
    lv_obj_t * invisible_cursor = lv_obj_create(lv_layer_top());
    lv_obj_set_size(invisible_cursor, 4, 4);
    lv_obj_set_style_radius(invisible_cursor, 0, 0);
    lv_obj_set_style_border_width(invisible_cursor, 0, 0);
    lv_obj_set_style_pad_all(invisible_cursor, 0, 0);
    lv_obj_clear_flag(invisible_cursor, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(invisible_cursor, lv_color_make(255, 0, 0), 0);
    lv_indev_set_cursor(mouse_indev, invisible_cursor);
    lv_obj_add_event_cb(lv_scr_act(), AutumnUI_Detect_TouchEvent, LV_EVENT_ALL, NULL);
    lv_timer_create(system_monitor_task, 1000, NULL);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(objects.lockscreendaylabel, 207, 607);
    lv_obj_set_style_align(objects.lockscreendaylabel, LV_ALIGN_CENTER, 0);
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

