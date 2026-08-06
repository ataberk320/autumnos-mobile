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
#include <dirent.h>

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
//for destroy hardcoded values
static int chkdrm(char *buf, size_t max_l) {
        DIR *dir = opendir("/dev/dri");
        if (!dir) return 0;

        struct dirent *entry;
        int found = 0;

        while ((entry = readdir(dir)) != NULL) {
                if (strncmp(entry->d_name, "card", 4) == 0) {
                snprintf(buf, max_l, "/dev/dri/%s", entry->d_name);
                found = 1;
                break;
                }
        }
    closedir(dir);
    return found;
}

static int chk_ethint(char *buf, size_t max_l) {
        DIR *dir = opendir("/sys/class/net");
        if (!dir) return 0;

        struct dirent *entry;
        int found = 0;

        while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, "lo") != 0) {
                        snprintf(buf, max_l, "%s", entry->d_name);
                        found = 1;
                        break;
                }
        }

        closedir(dir);
        return found;
}

static int chk_sndev(char *buf, size_t max_l) {
        struct stat st;
        if (stat("/dev/dsp", &st) == 0) {
                snprintf(buf, max_l, "/dev/dsp");
                return 1;
        }

        DIR *dir = opendir("/dev/snd");
        if (!dir) return 0;

        struct dirent *entry;
        int found = 0;

        while ((entry = readdir(dir)) != NULL) {
                if (strncmp(entry->d_name, "pcm", 3) == 0) {
                        snprintf(buf, max_l, "/dev/snd/%s", entry->d_name);
                        found = 1;
                        break;
                }
        }
        closedir(dir);
        return found;
}
//for destroy hardcoded values
int main() {
	sys_mnt(); //mounting system paths
	set_sig();
	        char drm_targ[64];
        char netint_targ[32];
        char snd_targ[64];
        if (!chk_ethint(netint_targ, sizeof(netint_targ))) {
                snprintf(netint_targ, sizeof(netint_targ), "eth0");
        }

        if (!chk_sndev(snd_targ, sizeof(snd_targ))) {
                snprintf(snd_targ, sizeof(snd_targ), "%s", "/dev/snd/pcmC0D0p");
        }

        if (chkdrm(drm_targ, sizeof(drm_targ))) {
                if (fork() == 0) {
                        setsid();
                        int con = open("/dev/console", O_RDWR);
                        if (con >= 0) {
                                dup2(con, 1);
                                dup2(con, 2);
                                close(con);
                        }
                        execl("/bin/session", "AutumnOS UI Session Layer", drm_targ, netint_targ, snd_targ, NULL); //FIXME: sending drivers with arguments
                        perror("init - session");
                        _exit(1);
                }
        }
        else {
                printf("Fallback to mksh.\n");
        }

	int tty = open("/dev/console", O_RDWR); //our console device

	pid_t shell_pid = fork();
    if (shell_pid == 0) {
        	setsid();
            ioctl(tty, TIOCSCTTY, 1);
            dup2(tty, 0); //stdin
            dup2(tty, 1); //stdout
            dup2(tty, 2); //stderr
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
