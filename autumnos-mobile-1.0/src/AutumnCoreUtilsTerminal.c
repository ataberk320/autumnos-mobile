#include "AutumnIO.h"
#include "acoreutils.h"
#include "AutumnSyscall.h"
#include <stdlib.h> //system();
int cursor_x = 0;
int cursor_y = 0;
#define TERM_WIDTH 60
#define TERM_HEIGHT 24
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
		if (cursor_x >= TERM_WIDTH) {
			cursor_x = 0;
			cursor_y++;
			char n = '\n';
			AutumnSys_Syscall(64, 1, (long)&n, 1, 0, 0, 0);
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
		
		buf[i++] = c;
		Terminal_PutStr(&c);
	}
	buf[i] = '\0';
	Terminal_PutStr("\n");
}

int main(int argc, char *argv[]) {
	if (argc < 1) return 1;
	const char *cmd = Terminal_GetCmd(argv[0]);
	if (Terminal_StrCmp(cmd, "CoreTerminal") == 0) {
		char input[128];
		Terminal_Clr();
		Terminal_PutStr("Welcome to AutumnOS CoreUtils Terminal!\n");
		while(1) {
			Terminal_PutStr("\033[32mAutumnOS\033[34m@\033[33mUser\033[0m: /#");
			Terminal_GetInp(input, 128);
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
					Terminal_PutStr("Folder name is not defined");
				}
				else {
					coreu_crdir(argv[1], 0755);
				}
        		}

        		else if (Terminal_StrCmp(input, "sudo") == 0) {
                		atmsu_root(argc > 1 ? argv[1] : "/bin/sh");
        		}

        		else {
                		Terminal_PutStr("Command not found:\n");
                		Terminal_PutStr(cmd_part);
        		}
	  	}
	}
}
