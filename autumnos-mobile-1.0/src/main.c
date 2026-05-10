#include "lvgl.h"
#include "ui.h"
#include <unistd.h>
#include <pthread.h>
#include "AutumnAPI.h"
#include <stdbool.h>
#include "acoreutils.h"
#include "AutumnMouseArg.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
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
extern const lv_font_t ui_font_default_18;
extern const lv_font_t ui_font_default_title;
extern const lv_font_t ui_font_default_16;
extern const lv_font_t ui_font_default_title;
extern const lv_font_t ui_font_noto_sans_ch_16;
static int mouse_pipe_fd = -1;
const lv_font_t * ui_18 = &ui_font_default_18;
const lv_font_t * ui_16 = &ui_font_default_16;
const lv_font_t * ui_22 = &ui_font_default_title;
volatile sig_atomic_t ANR_DETECTED = 0;
static autumn_touchpad_t last_mouse_data = {0, 0, 0};

void Touch_Shield() {
	lv_obj_clear_flag(objects.temperaturelock, LV_OBJ_FLAG_HIDDEN);
}



void AutumnUI_Overheating_Lock() {
	FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
	if (fp == NULL) {
		return;
	}

	int temp;
	fscanf(fp, "%d", &temp);
	fclose(fp);
	if (temp > 200000) {
                system("echo 1 > /sys/class/graphics/fbcon/cursor_blink 2>/dev/null");
                kmsg("Corrupted or invalid temperature data detected! System is not stable.");
                system("echo c > /proc/sysrq-trigger");
        }

	else if (temp > 90000) {
                kmsg("Dangerous data received! Forcing to power off without GUI!");
                AutumnAPI_Emergency_PowerOff();
	}
	
	else if (temp >= 45000) {
		Touch_Shield();
	}

	else {
		if (!lv_obj_has_flag(objects.temperaturelock, LV_OBJ_FLAG_HIDDEN)) {
			lv_obj_add_flag(objects.temperaturelock, LV_OBJ_FLAG_HIDDEN);
			AutumnAPI_Show_Toast("Sıcaklık kilidi kaldırıldı, cihazınız normal ısıya döndü.");
		}
	}
	
}

void Prepare_Bare_Idle_Console() {
        system("clear || printf '\033[2J\033[H'");
        system("echo 1 > /sys/class/graphics/fbcon/cursor_blink 2>/dev/null");
}

void AutumnUI_Bare_Idle_Mode() {
        kmsg("AutumnUI idle mode is active.");
}

void AutumnUI_Crash_Handler() {
                Prepare_Bare_Idle_Console();
                kmsg("Unfortunately, AutumnUI has stopped!");
                AutumnUI_Bare_Idle_Mode();
                kill(getpid(), SIGKILL);
}

void AutumnUI_Restart() {
        int result = system("/usr/bin/sysui -platform offscreen &");
        if (result == -1) {
                AutumnUI_Crash_Handler();
        }
        else {
                exit(0);
        }
}

void AutumnUI_NonCrash_Stop() {
        AutumnUI_Restart();
}


void AutumnUI_ANR_HANDLER(int sig) {
	if (sig == SIGUSR1) {
		ANR_DETECTED = 1;
	}


}
void UI_Lifetime() {
	if (ANR_DETECTED == 1) {
		AutumnUI_NonCrash_Stop();
	}
}

void AutumnUI_Play_Touch_Sound() {
	system("mpg123 /usr/share/touch.mp3 > /dev/null 2>&1 &");
}


 
static void AutumnUI_Detect_TouchEvent(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);
	
	if (code == LV_EVENT_PRESSED) {
		AutumnUI_Play_Touch_Sound();
	}
}

void AutumnUI_Set_UI_Status() {
	FILE *fpconf = fopen ("/etc/autumn_conf/sysui.cfg", "rb");
	if  (fpconf == NULL) {
		return;
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

void AutumnUI_PrintF_Log_Callback(const char *buf) {
	printf("[LVERR] %s", buf);
	fflush(stdout);
}

void AutumnUI_CanvasDrv() {
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
}

void AutumnUI_Read_MouseDrv(lv_indev_drv_t * indev_drv, lv_indev_data_t * data) {
    char buffer[64];
    ssize_t n;
    
    while ((n = read(mouse_pipe_fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        int temp_x, temp_y, temp_pr;
        
        if (sscanf(buffer, "%d %d %d", &temp_x, &temp_y, &temp_pr) == 3) {
            last_mouse_data.x = temp_x;
            last_mouse_data.y = temp_y;
            last_mouse_data.pressed = temp_pr;
        }
    }

    data->point.x = (lv_coord_t)last_mouse_data.x;
    data->point.y = (lv_coord_t)last_mouse_data.y;
    data->state = (last_mouse_data.pressed) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

void AutumnUI_IndevDrv() {
	mouse_pipe_fd = AutumnAPI_Set_MsPipe();
	static lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = AutumnUI_Read_MouseDrv;
	lv_indev_drv_register(&indev_drv);
}

int main(void) {
    int pwrst = AutumnAPI_Read_Power_Button_Status();
    if (pwrst == 1) {
		
    }
    else if (pwrst == 2) {
	AutumnAPI_Request_PowerOff();
    }
    struct sigaction sa;
    sa.sa_handler = AutumnUI_ANR_HANDLER;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
    lv_init();
    lv_log_register_print_cb(AutumnUI_PrintF_Log_Callback);
    AutumnUI_CanvasDrv();
    ui_init();
    AutumnUI_IndevDrv();
    lv_obj_add_event_cb(lv_scr_act(), AutumnUI_Detect_TouchEvent, LV_EVENT_ALL, NULL);
    lv_timer_create(system_monitor_task, 1000, NULL);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(objects.lockscreendaylabel, 207, 607);
    lv_timer_create(clock_timer, 5000, NULL);
    update_autumn_weather();
    lv_timer_create(weather_timer_callback, 300000, NULL);
    lv_timer_t * date_timer = lv_timer_create(ui_date_timer, 1000, NULL);

    while(1) {
	UI_Lifetime();
        lv_timer_handler();
        usleep(15000);
        lv_tick_inc(15);
    }
    return 0;
}

