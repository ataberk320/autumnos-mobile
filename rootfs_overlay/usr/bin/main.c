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

    // 1. Veriyi tek satırda çekiyoruz (Format: Şehir | Derece | Durum)
    fp = popen("wget -qO- http://wttr.in?format=%l,%t,%C", "r");
    
    if (fp == NULL) {
        lv_label_set_text(objects.weatherstatus, "Hata!");
        return;
    }

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0; // Satır sonunu temizle
        
        // Verileri "|" işaretine göre parçalayacağız
        char *sehir = strtok(buffer, ",");
        char *sehir_tekrar = strtok(NULL, ","); // "Istanbul, Istanbul" daki 2. Istanbul
        char *ulke = strtok(NULL, ",");
        char *derece = strtok(NULL, ",");
        char *durum = strtok(NULL, ",");

        // ŞEHİR
        if (sehir) {
            lv_label_set_text(objects.weathercity, sehir);
        }

        // DERECE (+ işaretini ve °C'yi temizliyoruz)
        if (derece) {
            if (derece[0] == '+') derece++; // Baştaki + işaretini atla
            // Eğer içinde °C varsa sadece sayıyı almak için temizleyelim
            char *c_pos = strstr(derece, "°C");
            if (c_pos) *c_pos = '\0'; 
            
            lv_label_set_text_fmt(objects.weatherdegree, "%s°C", derece);
        }

        // DURUM VE İKON
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
        // Eğer hiçbir şey okunamadıysa
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

    // EEZ Studio'dan gelen objenin adının doğru olduğundan emin ol
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

void clock_timer(lv_timer_t * timer) {
        update_clock();
        update_widget_clock();
}

void weather_timer_callback(lv_timer_t * timer) {
    update_autumn_weather();
}

int main(void) {

    // 1. LVGL Çekirdeğini Başlat
    lv_init();

    // 2. Linux Framebuffer Sürücüsünü Başlat (/dev/fb0)
    fbdev_init();

    // 3. Çift Tampon (Double Buffering) - 1GB RAM Avantajı
    // Her tamponu ekranın 1/4'ü kadar (200 satır) tutuyoruz. 
    // Bu, MQ Pro'nun DMA (Direct Memory Access) performansını artırır.
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf1[480 * 200]; 
    static lv_color_t buf2[480 * 200]; 
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, 480 * 200);

    // 4. Ekran Sürücüsü Yapılandırması
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = fbdev_flush; // LVGL'den Framebuffer'a veri taşıyan köprü
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = 480; // Dikey Genişlik
    disp_drv.ver_res = 800; // Dikey Yükseklik
    lv_disp_drv_register(&disp_drv);
    // 1. Önce donanımı (dokunmatik/mouse) başlatıyoruz
// 1. Donanımı başlatıyoruz
    ui_init();
    evdev_init();

    // 2. Değişkeni tanımlıyoruz
    static lv_indev_drv_t indev_drv;

    // 3. Değişkeni LVGL için sıfırlayıp hazırlıyoruz
    lv_indev_drv_init(&indev_drv);

    // 4. Özelliklerini atıyoruz
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;

    // 5. Kaydediyoruz ve dönen değeri 'mouse_indev' değişkenine atıyoruz!
    // (Az önce unuttuğumuz kritik kısım burasıydı)
    lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv);

    // 6. Kare imleç objesini oluşturuyoruz
    lv_obj_t * invisible_cursor = lv_obj_create(lv_layer_top());
    lv_obj_set_size(invisible_cursor, 4, 4);
    lv_obj_set_style_radius(invisible_cursor, 0, 0);
    lv_obj_set_style_border_width(invisible_cursor, 0, 0);
    lv_obj_set_style_pad_all(invisible_cursor, 0, 0);
    lv_obj_clear_flag(invisible_cursor, LV_OBJ_FLAG_CLICKABLE);

    // Test için istersen buraya renk satırını ekleyebilirsin:
    // lv_obj_set_style_bg_color(invisible_cursor, lv_color_make(255, 0, 0), 0);

    // 7. Objeyi imleç yapıyoruz
    lv_indev_set_cursor(mouse_indev, invisible_cursor);

    lv_timer_create(system_monitor_task, 1000, NULL);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    
    // Yandaki o gri sürükleme barlarını tamamen yok et
    lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_OFF);
    lv_timer_create(clock_timer, 5000, NULL);
    update_autumn_weather();
    lv_timer_create(weather_timer_callback, 300000, NULL);
    // 6. Ana Döngü (Zamanlama Kritik!)
    while(1) {
        lv_timer_handler(); // Grafik görevlerini yönetir
        usleep(15000);       // 5ms uyku (İşlemciyi serin tutar)
        lv_tick_inc(15);     // LVGL'ye 5ms geçtiğini bildir
    }
    return 0;
}

