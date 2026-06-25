#include "syscall.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/asound.h>
#include <unistd.h>

#define ATM_VOLUP 0x01
#define ATM_VOLDOWN 0x02

long hal_linux_syscall(long num, long a0, long a1, long a2, long a3, long a4, long a5) {
	long ret;
	register long r8 asm("x8") = num;
	register long r0 asm("x0") = a0;
	register long r1 asm("x1") = a1;
	register long r2 asm("x2") = a2;
	register long r3 asm("x3") = a3;
	register long r4 asm("x4") = a4;
	register long r5 asm("x5") = a5;

	asm volatile (
		"svc #0\n"
		: "+r"(r0)
		: "r"(r8), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5)
		: "memory"
	);
	return r0;
}

void hal_connectionstatus(uint8_t rf_type, bool block) {
        int rfk_fd = open("/dev/rfkill", O_WRONLY | O_NONBLOCK);
        if (rfk_fd < 0) {
                printf("[AUTUMNOS]: No rfkill device found. Network unavailable!");
                return;
        }
        struct rfkill_event event;
        memset(&event, 0, sizeof(event));
        event.op = RFKILL_OP_CHANGE_ALL;
        event.type = rf_type;
        event.soft = block ? 1 : 0;
        write(rfk_fd, &event, sizeof(event));
        close(rfk_fd);
}

void hal_volumesetting(int dir) {
	int fd = open("/dev/snd/controlC0", O_RDWR);
	if (fd < 0) return;
	
	struct snd_ctl_elem_value vol_val;
	memset(&vol_val, 0, sizeof(vol_val));
	
	vol_val.id.numid = 1;

	if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_READ, &vol_val) < 0) {
        	close(fd);
        	return;
    	}

    	if (dir == ATM_VOLUME_UP) {
        	vol_val.value.integer.value[0] += 5;
    	}
	else {
        	vol_val.value.integer.value[0] -= 5;
    	}

    	if (vol_val.value.integer.value[0] > 100) vol_val.value.integer.value[0] = 100;
    	if (vol_val.value.integer.value[0] < 0)   vol_val.value.integer.value[0] = 0;

	ioctl(fd, SNDRV_CTL_IOCTL_ELEM_WRITE, &vol_val);
	close(fd);
}
