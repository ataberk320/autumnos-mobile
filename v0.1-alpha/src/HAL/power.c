#include <sys/reboot.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static int hal_readsupply(const char *node, char *val) {
	char path[128];
	sprintf(path, "/sys/class/power_supply/battery0/%s", node);
	
	FILE *fp = fopen(path, "r");
	if (!fp) return -1;
	
	fgets(val, 32, fp);
	fclose(fp);
	return 0;
}

int hal_getcpc(void) {
	char buf[32];
	if (hal_readsupply("capacity", buf) == 0) {
		return atoi(buf);
	}
	return -1;
}


