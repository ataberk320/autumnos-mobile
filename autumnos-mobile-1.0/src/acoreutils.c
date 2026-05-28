#define RAM_START 0x40000000
#include "cheaders/acoreutils.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include "cheaders/AutumnIO.h"
#include "cheaders/AutumnSyscall.h"
#include "cheaders/AutumnCoreUtilsTerminal.h"
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

void coreu_exec(const char *cmd) {
	if (cmd == NULL || strlen(cmd) == 0) return;
	char final_cmd[512];
	snprintf(final_cmd, sizeof(final_cmd), "%s 2>&1", cmd);
	FILE *fp = popen(final_cmd, "r");
	if (fp == NULL) {
		Terminal_PutStr("Extern Linux command execution error!\n");
		return;
	}
	char out_buf[256];
	Terminal_PutStr("Response from Linux command:\n");
	while (fgets(out_buf, sizeof(out_buf), fp) != NULL) {
		Terminal_PutStr(out_buf);
	}
	pclose(fp);
}

void coreu_taskkill(int pid) {
    if (kill(pid, SIGTERM) == 0) {
        Terminal_PutStr("[ProcTerm]: SIGTERM sent to process.\n");
    } else {
        Terminal_PutStr("[ProcTerm]: Failed to terminate process.\n");
    }
}

void coreu_catf(const char *path) {
	int fd = AutumnIO_Open(path, 0);
	if (fd < 0) {
		Terminal_PutStr("[AutumnIO]: cannot open file!");
		Terminal_PutStr(path);
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
	if (ret < 0) Terminal_PutStr("[AutumnIO]: cannot create directory.\n");
}

void coreu_copyf(const char *src, const char *dest) {
	int fd_src = AutumnIO_Open(src, 0);
	if (fd_src < 0) {
		Terminal_PutStr("[AutumnIO]: no resources to copy.\n");
		return;
	}
	int fd_dest = AutumnIO_Open(dest, 65);
	if (fd_dest < 0) {
		AutumnIO_Close(fd_src);
		Terminal_PutStr("[AutumnIO]: invalid target to copy!\n");
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
	if (ret < 0) Terminal_PutStr("Cannot delete file.\n");
}

void atmsu_root(const char *command) {
	const char *pass = "1234";
	char input[32];
	Terminal_PutStr("[AtmSu-Root]: Enter your password for access root permission:");
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
		Terminal_PutStr("You are root now!\n");
	}
	else {
		Terminal_PutStr("Password are wrong, cannot access root permission!\n");
	}
}


void coreu_rdsoc(int fd) {
    char buffer[1024];
    
    int n = read(fd, buffer, sizeof(buffer) - 1);
    
    if (n > 0) {
        buffer[n] = '\0';
        Terminal_PutStr("Received data from socket: ");
        Terminal_PutStr(buffer);
        Terminal_PutStr("\n");
    }
    else if (n == 0) {
        Terminal_PutStr("Socket connection closed by remote.\n");
    }
    else {
        Terminal_PutStr("Socket is unavailable.\n");
    }
}

void coreu_tellwhoami() {
	long uid = AutumnSys_Syscall(174, 0, 0, 0, 0, 0, 0);
	if (uid == 0) {
		Terminal_PutStr("You are: root\n");
	}
	else if (uid == 1000) {
		Terminal_PutStr("You are: autumnuser0 (Default user)\n");
	}
	else {
		Terminal_PutStr("I don't know who you are...\n");
	}
}

void coreu_lsdir(const char *path) {
	const char *target = (path[0] == '\0') ? "." : path;
	DIR *d = opendir(target);
	struct dirent *dir;
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if (dir->d_name[0] != '.') {
				Terminal_PutStr(dir->d_name);
				Terminal_PutStr("  ");
				}
			}
			closedir(d);
			Terminal_PutStr("\n");
	}
	else {
		Terminal_PutStr("[lsdir]: could not opened directory.\n");
	}
}

void coreu_mnt(const char *source, const char *target, const char *fs_type) {
    long ret = AutumnSys_Syscall(165, (long)source, (long)target, (long)fs_type, 0, 0, 0);
    
    if (ret == 0) {
        Terminal_PutStr("[AutumnIO]: Device mounted successfully.\n");
    } else {
        Terminal_PutStr("[AutumnIO]: Mount syscall failed!\n");
    }
}

void coreu_umnt(const char *target) {
    long ret = AutumnSys_Syscall(166, (long)target, 0, 0, 0, 0, 0);
    
    if (ret == 0) {
        Terminal_PutStr("[AutumnIO]: Device unmounted.\n");
    } else {
        Terminal_PutStr("[AutumnIO]: Umount syscall failed!\n");
    }
}

void coreu_crfil(const char *filename) {
	if (filename == NULL || filename[0] == '\0') {
		Terminal_PutStr("[AutumnIO]: filename not defined\n");
		return;
	}

	int fd = AutumnIO_Open(filename, 65);
	if (fd < 0) {
		Terminal_PutStr("[AutumnIO]: could not create file\n");
	}
	else {
		AutumnIO_Close(fd);
	}
}

int coreu_goto(const char *path) {
	if (path == NULL || path[0] == '\0') {
		return chdir("/");
	}
	if (chdir(path) != 0) {
		return -1;
	}
	else {
		Terminal_PutStr("[AutumnIO]: You are in ");
		Terminal_PutStr(path);
		Terminal_PutStr("\n");
	}
	return 0;
}
void coreu_dd(const char *src, const char *dest) {
	int fd_src = AutumnIO_Open(src, 0);
	int fd_dest = AutumnIO_Open(dest, 65);
	
	if (fd_src < 0 || fd_dest < 0) {
		if (fd_src >= 0) AutumnIO_Close(fd_src);
		return;
	}

	char block[512];
	int bytes_read;
	unsigned long total_bytes = 0;
	
	Terminal_PutStr("[dd]: writing blocks to destination\n");
	
	while (bytes_read = AutumnIO_Read(fd_src, block, bytes_read)) {
		AutumnIO_Write(fd_dest, block, bytes_read);
		total_bytes += bytes_read;
	}

	AutumnIO_Close(fd_src);
	AutumnIO_Close(fd_dest);
	
	Terminal_PutStr("[dd]: process writing blocks to destination is successfully completed\n");
}

void coreu_pullandgrep(const char *path, const char *str) {
	int fd = AutumnIO_Open(path, 0);
	if (fd < 0) {
		Terminal_PutStr("[grep]: invalid destination!\n");
		return;
	}

	char line[256];
	char ch;
	int idx = 0;
	
	while (AutumnIO_Read(fd, &ch, 1) > 0) {
		if (ch != '\n' && idx < 255) {
			line[idx++] = ch;
		}
		else {
			line[idx] = '\0';
			if (strstr(line, str) != NULL) {
				Terminal_PutStr(line);
				Terminal_PutStr("\n");
			}
			idx = 0;
		}
	}
	AutumnIO_Close(fd);
}

void coreu_find(const char *path, const char *filename) {
	const char *target = (path[0] == '\0') ? "." : path;
	DIR *d = opendir(target);
	struct dirent *dir;

	if (!d) {
		Terminal_PutStr("[find]: could not open the path!\n");
		return;
	}

	while ((dir = readdir(d)) != NULL) {
		if (dir->d_name[0] == '.' && (dir->d_name[1] == '\0' || (dir->d_name[1] == '.' && dir->d_name[2] == '\0'))) {
			continue;
		}

		if (strcmp(dir->d_name, filename) == 0) {
			Terminal_PutStr("Found result: ");
			Terminal_PutStr(target);
			Terminal_PutStr("/");
			Terminal_PutStr(dir->d_name);
			Terminal_PutStr("\n");
		}
	}
	closedir(d);
}

