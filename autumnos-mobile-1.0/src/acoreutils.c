#define RAM_START 0x40000000
#include "acoreutils.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include "AutumnIO.h"
#include "AutumnSyscall.h"
static int kmsg_fd = -1;
int last_pulse = 0;

void kmsg_open() {
        kmsg_fd = open("/dev/kmsg", O_WRONLY);
}

void kmsg(const char *msg) {
        if (kmsg_fd >= 0 && msg) {
                write(kmsg_fd, msg, strlen(msg));
        }
}

void kpanic(const char *err) {
        asm volatile("csrci mstatus, 8");
        kmsg("[AUTUMNOS]: Critical error detected, system halted on while(1)");
        kmsg("Error message: ");
        kmsg(err);
        while(1) {
                asm volatile("wfi");
        }
}

void kmsgf(const char *fmt, ...) {
	va_list args;
        char buffer[256];
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        kmsg(buffer);
}

void kbridge(uintptr_t phys_addr) {
        if (phys_addr == 0) {
                kpanic("[AUTUMNOS]: NULL physical address!");
        }

        if (phys_addr < RAM_START) {
                kpanic("[AUTUMNOS]: This is an illegal address out of RAM!");
        }

        if (phys_addr & 0x3) {
                kmsgf("[AUTUMNOS]: Address 0x%lx is unaligned.");
        }
}

void kbeat(uintptr_t current_tick) {
        if (last_pulse != 0) {
                uintptr_t delta = current_tick - last_pulse;
                if (delta > 1000) {
                        kmsgf("[AUTUMNOS]: Lag detected %lu ticks since last beat.", delta);
                        }
        }
        last_pulse = current_tick;
}

void kmap(uintptr_t addr) {
        kmsgf("[AUTUMNOS: Analyzing Address: 0x%lx", addr);

        if (addr >= 0x40000000 && addr <= 0x48000000) {
                kmsgf("0x%lx is System RAM.", addr);
        }
        else if (addr >= 0x01C00000 && addr <= 0x1C00FFF) {
                kmsgf("0x%lx is G2D Engine Registers Area", addr);
        }
        else if (addr >= 0x02000000 && addr <= 0x02000FFF) {
                kmsgf("0x%lx is GPIO/PIO Controllers Area", addr);
        }
        else if (addr < 0x00010000) {
                kmsgf("0x%lx is Boot ROM area", addr);
        }
        else {
                kmsgf("Don't know where is 0x%lx, maybe it's a reserved space.", addr);
                kmsg("[AUTUMNOS]: Accessing unmapped hardware regions might cause a hang!");
        }
}

void coreu_catf(const char *path) {
	int fd = AutumnIO_Open(path, 0);
	if (fd < 0) {
		printf("[AutumnIO]: Cannot open file!");
		printf(path);
		return;
	}
	char buf[1024];
	int n;
	while ((n = AutumnIO_Read(fd, buf, sizeof(buf))) > 0) {
		AutumnIO_Write(1, buf, n);
	}

	AutumnIO_Close(fd);
}

void coreu_crdir(const char *path, int mode) {
	long ret = AutumnSys_Syscall(34, -100, (long)path, mode, 0, 0, 0);
	if (ret < 0) printf("[AutumnIO]: Cannot create directory.");
}

void coreu_copyf(const char *src, const char *dest) {
	int fd_src = AutumnIO_Open(src, 0);
	if (fd_src < 0) {
		printf("[AutumnIO]: No resources to copy.\n");
		return;
	}
	int fd_dest = AutumnIO_Open(dest, 65);
	if (fd_dest < 0) {
		AutumnIO_Close(fd_src);
		printf("[AutumnIO]: Invalid target to copy!\n");
		return;
	}

	char buffer[4096];
	int n;
	while((n = AutumnIO_Read(fd_src, buffer, 4096)) > 0) {
		AutumnIO_Write(fd_dest, buffer, n);
	}
	AutumnIO_Close(fd_src);
	AutumnIO_Close(fd_dest);
}

void coreu_remf(const char *path) {
	long ret = AutumnSys_Syscall(35, -100, (long)path, 0, 0, 0, 0);
	if (ret < 0) printf("Cannot delete file.\n");
}

void atmsu_root(const char *command) {
	const char *pass = "Meyvelerden_Mango_Oslardan_Guvenli";
	char input[32];
	printf("[AtmSu-Root]: Enter your password for access root permission:");
	int n = AutumnIO_Read(0, input, 31);
	if (n > 0 && input[n-1] == '\n') input[n-1] = '\0';
	
	int i = 0, match = 1;
	while (pass[i] || input[i]) {
		if (pass[i] != input[i]) {
			match = 0;
			break;
		}
		i++;
	}

	if (match) {
		AutumnSys_Syscall(SYS_SETUID, 0, 0, 0, 0, 0, 0);
		printf("You are root now!");
	}
	else {
		printf("Password are wrong, cannot access root permission!");
	}
}

void coreu_tellwhoami() {
	long uid = AutumnSys_Syscall(174, 0, 0, 0, 0, 0, 0);
	if (uid == 0) {
		printf("You are: root");
	}
	else if (uid == 1000) {
		printf("You are: autumnuser0 (Default user)");
	}
	else {
		printf("I don't know who you are...");
	}
}
