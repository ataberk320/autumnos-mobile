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

    // EEZ Studio'dan gelen objenin adının doğru olduğundan emin ol
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
    evdev_init();
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read; // /dev/input/eventX üzerinden okuma yapar
    lv_indev_drv_register(&indev_drv);
    // 5. EEZ Studio Tasarımını Yükle
    ui_init();
    lv_timer_create(system_monitor_task, 1000, NULL);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    
    // Yandaki o gri sürükleme barlarını tamamen yok et
    lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_OFF);
    lv_timer_create(update_clock, 1000, NULL);
    // 6. Ana Döngü (Zamanlama Kritik!)
    while(1) {
        lv_timer_handler(); // Grafik görevlerini yönetir
        usleep(5000);       // 5ms uyku (İşlemciyi serin tutar)
        lv_tick_inc(5);     // LVGL'ye 5ms geçtiğini bildirir
	
    }

    return 0;
}
