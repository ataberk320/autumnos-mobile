// SPDX-License-Identifier: GPL-2.0-only
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/panic_notifier.h>
#include <linux/kexec.h>

#define RSOD_ADDR  0x84000000
#define RSOD_MSG_MAX 256
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Autix (ataberk320)");
MODULE_DESCRIPTION("Red Screen Of Death (RSOD) Support driver.");
MODULE_VERSION("0.1");

struct shared_excinfo {
	uint32_t magic;
	uint32_t errcod;
	char message[256];
};

static int RedFrog_Catch(struct notifier_block *this, unsigned long event, void *ptr) {
	volatile struct shared_excinfo *shared = (volatile struct shared_excinfo *)RSOD_ADDR;
    
    	shared->magic = 0x524F4F43; // "ROOC" (Red Frog)
    	shared->errcod = 0xDEADBEEF; 

    	if (ptr) {
        	const char *panic_msg = (const char *)ptr;
        	int i = 0;
        	while (panic_msg[i] != '\0' && i < RSOD_MSG_MAX - 1) {
            		shared->message[i] = panic_msg[i];
            		i++;
        	}
        	shared->message[i] = '\0';
    	} 
	else {
        	const char *default_msg = "Unknown reason.";
        	int i = 0;
        	while (default_msg[i] != '\0') {
            		shared->message[i] = default_msg[i];
            		i++;
        	}
        	shared->message[i] = '\0';
    	}

    	crash_kexec(NULL); //edit of unsupported function.

    	return NOTIFY_DONE;
}

static struct notifier_block rsod_bl = {
	.notifier_call = RedFrog_Catch,
	.priority = INT_MAX,
};

static int __init RedFrog_Idle(void) {
	atomic_notifier_chain_register(&panic_notifier_list, &rsod_bl);
    	return 0;
}

static void __exit RedFrog_DSleep(void) {
	atomic_notifier_chain_unregister(&panic_notifier_list, &rsod_bl);
}

module_init(RedFrog_Idle);
module_exit(RedFrog_DSleep);
