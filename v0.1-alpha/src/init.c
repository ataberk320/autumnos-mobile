#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/mount.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>

void sys_mnt() {
	if (mount("proc", "/proc", "proc", 0, NULL) < 0 && errno != EBUSY) perror("init: Couldn't mount proc");
	if (mount("sysfs", "/sys", "sysfs", 0, NULL) < 0 && errno != EBUSY) perror("init: Couldn't mount sysfs");
}

int main() {
	sys_mnt();
	pid_t pid = fork();
    	if (pid == 0) {
        	execl("/usr/bin/sysui", "sysui", NULL);
        
        	perror("sysui");
        	_exit(1);
    	}

    	while (1) {
        	sleep(60); 
    	}
    	return 0;
}
