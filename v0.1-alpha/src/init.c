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
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <setjmp.h>
#include <linux/fb.h>
#include "sigf.h"



void sys_mnt() {
	if (mount("proc", "/proc", "proc", 0, NULL) < 0 && errno != EBUSY) perror("init: Couldn't mount proc");
	if (mount("sysfs", "/sys", "sysfs", 0, NULL) < 0 && errno != EBUSY) perror("init: Couldn't mount sysfs");
	if (mount("devtmpfs", "/dev", "devtmpfs", 0, NULL) < 0 && errno != EBUSY) {
		perror("init");
	}
	else  {
		printf("init: devtmpfs mounted successfully!\n");
	}

    	if (mount("tmpfs", "/tmp", "tmpfs", 0, NULL) < 0 && errno != EBUSY) {
        	perror("init: Couldn't mount tmpfs");
    	}
	else {
		printf("init: tmpfs mounted successfully!\n");
	}
	
	mkdir("/dev/shm", 0755);
	if (mount("tmpfs", "/dev/shm", "tmpfs", 0, NULL) < 0 && errno != EBUSY) {
    		perror("init: Couldn't mount /dev/shm");
	}
	else {
		printf("init: '/dev/shm' mounted successfully!\n");
	}
}


int main() {
	sys_mnt();
	set_sig();

	for (int i = 0; i < 5; i++) {
		if (access("/dev/dri/card0", F_OK) == 0) break;
		sleep(1);
	}
	

	if (access("/dev/dri/card0", F_OK) == 0) {
		if (fork() == 0) {
			setsid();
			int con = open("/dev/console", O_RDWR);
			dup2(con, 1);
			dup2(con, 2);
            execl("/usr/bin/session", "AutumnOS UI Session Layer", NULL);
            _exit(1);
		}
	}	

	int tty = open("/dev/console", O_RDWR);

	pid_t shell_pid = fork();
    if (shell_pid == 0) {
        	setsid();
            ioctl(tty, TIOCSCTTY, 1);
            dup2(tty, 0);
            dup2(tty, 1);
            dup2(tty, 2);
            execl("/bin/mksh", "Terminal", NULL);
            _exit(1);
    }

    while (1) {
		int status;
		pid_t wpid = wait(&status);

		if (wpid > 0 && WIFSIGNALED(status)) {
			printf("Process %d crashed with signal %d\n", wpid, WTERMSIG(status));
		}
    }
    	return 0;
}
