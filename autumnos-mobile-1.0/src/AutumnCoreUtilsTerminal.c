#include <termios.h>
#include "AutumnIO.h"
#include "acoreutils.h"
#include "AutumnSyscall.h"
#include <stdlib.h> //system();
#include "AutumnCoreUtilsTerminal.h"
#include "AutumnAPI.h"
#include <fcntl.h>    // open() & O_RDWR
#include <unistd.h>   // close()
int cursor_x = 0;
int cursor_y = 0;
#define TERM_WIDTH 60
#define TERM_HEIGHT 24

void Serial_Disable_Echo(int fd) {
        if (fd < 0) return;
        struct termios toptions;

        if (tcgetattr(fd, &toptions) < 0) {
                return;
        }

        toptions.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON | ISIG);

        toptions.c_iflag &= ~(INPCK | ISTRIP | IGNCR | ICRNL | INLCR | IXON | IXOFF);
        toptions.c_oflag &= ~(OPOST);

        toptions.c_cflag &= ~PARENB;
        toptions.c_cflag &= ~CSTOPB;
        toptions.c_cflag &= ~CSIZE;
        toptions.c_cflag |= CS8;

        toptions.c_cc[VMIN] = 1;
        toptions.c_cc[VTIME] = 0;

        tcsetattr(fd, TCSANOW, &toptions);
}

void Terminal_Clr() {
	system("clear || printf '\033[2J\033[H'");
}

int Terminal_PutChr(char c) {
	AutumnSys_Syscall(64, 1, (long)&c, 1, 0, 0, 0);
}

void Terminal_PutInt(int n) {
	if (n == 0) {
		Terminal_PutChr('0');
		return;
	}
	char buf[10];
	int i = 0;
	while (n > 0) {
		buf[i++] = (n % 10) + '0';
		n /= 10;
	}
	while (i > 0) {
		Terminal_PutChr(buf[--i]);
	}
}

void Terminal_PutStr(const char *str) {
	int len = 0;
	while(*str) {
		if (*str == '\n') {
			cursor_y++;
			cursor_x = 0;
			char n = '\n';
			AutumnSys_Syscall(64, 1, (long)&n, 1, 0, 0, 0);
		}

		else if (*str == '\r') {
			cursor_x = 0;
			char r = '\r';
			AutumnSys_Syscall(64, 1, (long)&r, 1, 0, 0, 0);
		}

		else {
			AutumnSys_Syscall(64, 1, (long)str, 1, 0, 0, 0);
			cursor_x++;
		}
		str++;
	}
}


int Terminal_StrCmp(const char *s1, const char *s2) {
	while (*s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	}
	return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

const char* Terminal_GetCmd(const char *path) {
	const char *last = path;
	while(*path) {
		if (*path == '/') last = path + 1;
		path++;
	}
	return last;
}

void Terminal_GetInp(char *buf, int max_len) {
	int i = 0;
	char c;
	while (i < max_len - 1) {
		AutumnSys_Syscall(63, 0, (long)&c, 1, 0, 0, 0);
		if (c == '\n' || c == '\r') break;
		if (c >= 32 && c < 127) {
			buf[i++] = c;
			Terminal_PutChr(c);
		}
	}
	buf[i] = '\0';
	Terminal_PutChr('\n');
}
#ifndef TERMINAL_AS_LIB
int main(int argc, char *argv[]) {
	if (argc < 1) return 1;
	const char *cmd = Terminal_GetCmd(argv[0]);
	if (Terminal_StrCmp(cmd, "CoreTerminal") == 0) {
		char input[128];
		Terminal_Clr();
		Terminal_PutStr("\033[1;33mW\033[0;34me\033[0;31ml\033[1;33mc\033[0;34mo\033[0;31mm\033[1;33me\033[0;34m \033[0;31mt\033[1;33mo\033[0;34m \033[0;31mA\033[1;33mu\033[0;34mt\033[0;31mu\033[1;33mm\033[0;34mn\033[0;31mO\033[1;33mS\033[0;34m \033[0;31mC\033[1;33mo\033[0;34mr\033[0;31me\033[1;33mU\033[0;34mt\033[0;31mi\033[1;33ml\033[0;34ms\033[0;31m \033[1;33mT\033[0;34me\033[0;31mr\033[1;33mm\033[0;34mi\033[0;31mn\033[1;33ma\033[0;34ml\033[0;31m!\033[0m\n");
		while(1) {
			int tty_fd = open("/dev/ttyS0", O_RDWR);
			struct termios oldt;
			if (tty_fd >= 0) {
				tcgetattr(tty_fd, &oldt);
				Serial_Disable_Echo(tty_fd);
				close(tty_fd);
			}
			Terminal_PutStr("\033[32mAutumnOS\033[34m@\033[33mUser\033[0m: /#");
			Terminal_GetInp(input, 128);

			tty_fd = open("/dev/ttyS0", O_RDWR);
			if (tty_fd >= 0) {
				tcdrain(tty_fd);
				tcsetattr(tty_fd, TCSANOW, &oldt);
				close(tty_fd);
			}
			if (input[0] == '\0') continue;
			char *cmd_part = input;
			char *arg_part = "";

			for (int i = 0; input[i] != '\0'; i++) {
				if (input[i] == ' ') {
					input[i] = '\0';
					arg_part = &input[i+1];
					while(*arg_part == ' ') arg_part++;
					break;
				}	
			}
			if (Terminal_StrCmp(input, "exec") == 0) {
				if (arg_part[0] == '\0') {
					Terminal_PutStr("Using: exec <example: pip install package>\n");
				}
				else {
					coreu_exec(arg_part);
				}
			}
			if (Terminal_StrCmp(cmd_part, "cat") == 0) {
                        	if (arg_part[0] == '\0') {	
					Terminal_PutStr("Using: cat <file.txt>\n");
				}
				else {
					coreu_catf(arg_part);
				}
        		}

        		else if (Terminal_StrCmp(input, "whoami") == 0) {
                		coreu_tellwhoami();
        		}

        		else if (Terminal_StrCmp(input, "mkdir") == 0) {
                		if (arg_part[0] == '\0') {
					Terminal_PutStr("Using: mkdir <folder>");
				}
				else {
					coreu_crdir(arg_part, 0755);
				}
        		}

        		else if (Terminal_StrCmp(input, "sudo") == 0) {
                		atmsu_root(argc > 1 ? argv[1] : "/bin/sh");
        		}
			
			else if (Terminal_StrCmp(input, "atch") == 0) {
				if (arg_part[0] == '\0') {
					Terminal_PutStr("Using: atouch <file.format>");
				}
				else {
					coreu_crfil(arg_part);
				}
			}

			else if (Terminal_StrCmp(input, "ls") == 0) {
				if (arg_part[0] == '\0') {
					Terminal_PutStr("Using: ls /your/path/to/file");
				}
				else {
					coreu_lsdir(arg_part);
				}
			}

			else if (Terminal_StrCmp(input, "cd") == 0) {
				if (arg_part[0] == '\0') {
					Terminal_PutStr("Using: cd /current/path/to/change");
				}
				else {
					coreu_goto(arg_part);
				}
			}

			else if (Terminal_StrCmp(input, "reboot") == 0) {
				AutumnAPI_Request_Reboot();
			}

			else if (Terminal_StrCmp(input, "shutdown") == 0) {
				AutumnAPI_Request_PowerOff();
			}

			else if (Terminal_StrCmp(input, "exit") == 0) {
				Terminal_PutStr("[CoreUtilsTerminal]: Ending current terminal session\n");
				Terminal_PutStr("Process '/usr/bin/CoreTerminal' exited by 0\n");
				break;
			}

			else if (Terminal_StrCmp(input, "echo") == 0) {
				if (arg_part[0] == '\0') {
					Terminal_PutStr("Using: echo autumn");
				}
				else {
					Terminal_PutStr(arg_part);
					Terminal_PutStr("\n");
				}
			}
        		else {
                		Terminal_PutStr("Command not found: ");
                		Terminal_PutStr(cmd_part);
				Terminal_PutStr("\n");
        		}
	  	}
	}
}
#endif
