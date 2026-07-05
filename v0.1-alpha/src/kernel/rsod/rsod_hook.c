#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/panic_notifier.h>
#include <linux/init.h>
#include <linux/io.h>
#include <asm/setup.h>
#include <linux/module.h>
#include <linux/string.h>
#include "rsod_bin.h"
#include <linux/console.h>
#include <linux/fb.h>

static int rsod(struct notifier_block *nb, unsigned long code, void *data)
{
    extern struct fb_info *registered_fb[FB_MAX];
    struct fb_info *info = registered_fb[0];

    if (info && info->screen_base) {
        uint32_t *fb = (uint32_t *)info->screen_base;
        int pixels = info->var.xres * info->var.yres;
        
	int i;
        for (i = 0; i < pixels; i++) {
            fb[i] = 0x00FF0000; 
        }
    }
    return NOTIFY_DONE;
}

static struct notifier_block rsod_notif = {
	.notifier_call = rsod,
};

static int __init init_rsod(void) {
	    void *target = (void *)0x80500000;
    	memcpy(target, rsod_bin, rsod_bin_len);
	    atomic_notifier_chain_register(&panic_notifier_list, &rsod_notif);
    	pr_info("AutumnOS rSOD hook actived.\n");
    	return 0;
}

module_init(init_rsod);

