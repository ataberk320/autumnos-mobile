#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/panic_notifier.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/string.h>
#include <linux/fb.h>
#include <linux/panic.h>
#include "kp_banner.h"
#include "f8x8.h"

#define IMG_WIDTH     124   
#define IMG_HEIGHT    124   

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Autix (ataberk320)");
MODULE_DESCRIPTION("AutumnOS Panic Screen Driver");
MODULE_VERSION("0.1");

static struct fb_info *global_fb_info = NULL;

static u32 afmtcolor(struct fb_info *info, u8 r, u8 g, u8 b, u8 a) {
    u32 color = 0;
    
    color |= ((u32)r >> (8 - info->var.red.length)) << info->var.red.offset;
    color |= ((u32)g >> (8 - info->var.green.length)) << info->var.green.offset;
    color |= ((u32)b >> (8 - info->var.blue.length)) << info->var.blue.offset;
    
    if (info->var.transp.length) {
        color |= ((u32)a >> (8 - info->var.transp.length)) << info->var.transp.offset;
    }
    
    return color;
}

static void aputc(u32 *fb, int screen_w, int screen_h, char c, int x, int y, u8 r, u8 g, u8 b) {
    const uint8_t *glyph = f8x8[(int)c];
    int i, j;
    int scr_w = info->var.xres;
    int scr_h = info->var.yres;

    u32 color = afmtcolor(info, r, g, b, 0xFF);
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            if (glyph[i] & (1 << (7 - j))) {
                if ((y + i) < screen_h && (x + j) < screen_w) {
                    __raw_writel(color, &fb[(y + i) * screen_w + (x + j)]);
                }
            }
        }
    }
}

static void aputs(u32 *fb, int screen_w, int screen_h, const char *str, int x, int y, u8 r, u8 g, u8 b) {
    while (*str) {
        aputc(fb, screen_w, screen_h, *str++, x, y, r, g, b);
        x += 8; 
    }
}

static int RedFrog_Catch(struct notifier_block *this, unsigned long event, void *ptr) {
    struct fb_info *info = global_fb_info;
    u32 *screen_buf = NULL;
    int screen_w = 480;
    int screen_h = 320;
    
    char panic_msg_buf[128] = "Kernel Hardware / Software Exception Detected.";

    if (!info || !info->screen_base) {
	pr_emerg("RedFrog: Error: No active framebuffer linked!\n");
	return NOTIFY_DONE;
    }

    if (ptr) {
	strscpy(panic_msg_buf, (const char *)ptr, sizeof(panic_msg_buf);
    }


    screen_buf = (u32 __force *)info->screen_base;
    screen_w = info->var.xres;
    screen_h = info->var.yres;


    if (screen_buf) {
        int y, i;
        const u32 *img_pixels;
        
        int padding_x = (screen_w - IMG_WIDTH) / 2;
        int padding_y = 30;
        int text_x, text_y;

	u32 black = format_color(info, 0, 0, 0, 0xFF);

        for (i = 0; i < screen_w * screen_h; i++) {
            __raw_writel(black, &screen_buf[i]);
        }

        img_pixels = (const u32 *)rsod_img;
        
        for (y = 0; y < IMG_HEIGHT && (padding_y + y) < screen_h; y++) {
            int screen_idx = (padding_y + y) * screen_w + padding_x;
            int img_idx = y * IMG_WIDTH;
            int x;
            
            for (x = 0; x < IMG_WIDTH && (padding_x + x) < screen_w; x++) {
                u32 argb = img_pixels[img_idx + x];
                u32 a = (argb >> 24) & 0xFF;
                u32 r = (argb >> 16) & 0xFF;
                u32 g = (argb >> 8) & 0xFF;
                u32 b = argb & 0xFF;
                u32 img_pix = afmtcolor(info, r, g, b, a);
                
                __raw_writel(img_pix, &screen_buf[screen_idx + x]);
            }
        }

        text_x = (screen_w - (32 * 8)) / 2; 
        if (text_x < 10) text_x = 10;
        
        text_y = padding_y + IMG_HEIGHT + 25;

        aputs(screen_buf, screen_w, screen_h, "PANIC: AutumnOS System Exception", text_x, text_y, 255, 255, 255);

        int msg_len = strlen(panic_msg_buf);
        int msg_x = (screen_w - ((msg_len + 5) * 8)) / 2;
        if (msg_x < 10) msg_x = 10;

        char log_b[128];
        snprintf(log_b, sizeof(log_b), "LOG: %s", panic_msg_buf);
        
	aputs(info, screen_buf, log_b, msg_x, text_y + 20, 255, 0, 0);

        
        pr_emerg("panicLOG: %s\n", panic_msg_buf);
    }

    mdelay(5000);
    machine_restart(NULL);

    return NOTIFY_DONE;
}

static int fb_notifier_callback(struct notifier_block *self, unsigned long event, void *data) {
    struct fb_event *evdata = data;
    
    if (event == FB_EVENT_FB_REGISTERED) {
        if (evdata && evdata->info) {
            global_fb_info = evdata->info; 
            pr_info("RedFrog: Active Framebuffer linked successfully. Res: %dx%d\n", 
                    global_fb_info->var.xres, global_fb_info->var.yres);
        }
    }
    return NOTIFY_OK;
}

static struct notifier_block rsod_bl = {
    .notifier_call = RedFrog_Catch,
    .priority = INT_MAX,
};

static struct notifier_block fb_notif = {
    .notifier_call = fb_notifier_callback,
};

static int __init RedFrog_Idle(void) {
    fb_register_client(&fb_notif);
    atomic_notifier_chain_register(&panic_notifier_list, &rsod_bl);
    
    return 0;
}

late_initcall(RedFrog_Idle);
