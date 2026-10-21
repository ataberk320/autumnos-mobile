#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "../include/HAL/alog.h"
#include <sys/reboot.h>
#include <linux/reboot.h>
#ifndef __NR_kexec_file_load
#define __NR_kexec_file_load 294
#endif

#define KEXEC_ON_CRASH  0x00000001

void usage(const char *prg) {
	printf("Using: %s [-options] <image>\n", prg);
    	printf("Options:\n");
    	printf("  -l    Load a kernel.\n");
    	printf("  -p    Load a crash kernel.\n");
    	printf("  -e    Execute loaded kernel.\n");
}

int main (int argc, char *argv[]) {
	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'e') {
        	a_printf("Jumping to loaded kernel...\n");
        	sync();
        	if (reboot(LINUX_REBOOT_CMD_KEXEC) < 0) {
            		a_perror("LINUX_REBOOT_CMD_KEXEC");
            		return 1;
        	}
        	return 0;
    	}
	
	if (argc < 3) {
		usage(argv[0]);
		return 1;
	}

	char mode = '\0';
	if (argv[1][0] == '-') {
		mode = argv[1][1];
	}

	if (mode != 'l' && mode != 'p') {
        	printf("Unknown command: -%c\n", mode);
        	usage(argv[0]);
        	return 1;
    	}

    	const char *kernel_path = argv[2];
    	int fd = open(kernel_path, O_RDONLY);
    	if (fd < 0) {
        	perror("Kernel file");
        	return 1;
    	}

	unsigned long flags = (mode == 'p') ? KEXEC_ON_CRASH : 0;
	const char *cmdline = "root=/dev/mmcblk0p2 console=ttyS0,115200";
	
	long ret = syscall(__NR_kexec_file_load, fd, 0, (unsigned long)__builtin_strlen(cmdline), cmdline, flags);
    	close(fd);

    	if (ret < 0) {
        	a_perror("kexec_file_load");
        	return 1;
    	}

    	printf("autumn-kexec: Operation completed successfully: (Mod: -%c, Flags: 0x%lx).\n", mode, flags);
    	return 0;
}


