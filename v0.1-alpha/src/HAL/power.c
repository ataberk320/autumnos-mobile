#include <sys/reboot.h>
#include <fcntl.h>
#include <unistd.h>

void hal_reboot(void) {
        sync();
        reboot(RB_AUTOBOOT);
}

void hal_pwroff(void) {
        sync();
        reboot(RB_POWER_OFF);
}

void hal_emergency_pwroff(void) {
	int fd = open("/proc/sysrq-trigger", O_WRONLY);
	if (fd >= 0) {
		write(fd, "o", 1);
		close(fd);
	}
        _exit(1);
}
