#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <termios.h>
#include <sys/stat.h>
#include "../include/HAL/io.h"

#define PSW_FILE "/etc/atmpasswd"

void getminput(const char *prompt, char *buf, size_t max_len) {
	struct termios oldt, newt;
	int i = 0;
	char ch;

	printf("%s", prompt);
	fflush(stdout);
	
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	while (1) {
        	ch = getchar();
        	if (ch == '\n' || ch == '\r') {
            		buf[i] = '\0';
            		break;
        	}
		else if (ch == 127 || ch == '\b') {
            		if (i > 0) {
                		i--;
                		printf("\b \b");
                		fflush(stdout);
            		}
        	}
		else if (i < max_len - 1) {
            		buf[i++] = ch;
            		printf("*");
            		fflush(stdout);
        	}
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
}


void setpswd() {
	char p1[64], p2[64];
	printf("autumn-sudo: Notify: No system password found, please create a new password:\n");
	
	getminput("New password: ", p1, sizeof(p1));
	getminput("Confirm password: ", p2, sizeof(p2));

	if (strcmp(p1, p2) != 0 || strlen(p1) == 0) {
        	fprintf(stderr, "autumn-sudo: Error: Password does not match or empty!\n");
        	exit(1);
    	}

	int fd = _AutumnSys_ioOpen(PSW_FILE, A_WRONLY | A_CRT | A_TRUNC, A_IRUSR | A_IWUSR);
	if (fd < 0) {
		printf("autumn-sudo: Error: Cannot create password data file.\n");
		exit(1);
	}
	
	_AutumnSys_ioWrite(fd, p1, strlen(p1));
	_AutumnSys_ioClose(fd);

	const char *success = "autumn-sudo: Success: Password saved successfully!\n\n";
	write(STDOUT_FILENO, success, strlen(success));
}

int verify() {
	int fd = _AutumnSys_ioOpen(PSW_FILE, A_RDONLY, 0);
	if (fd < 0) return 0;

	char pass[64] = {0};
	ssize_t bytes_read = _AutumnSys_ioRead(fd, pass, sizeof(pass) - 1);
	_AutumnSys_ioClose(fd);
	
	if (bytes_read > 0) {
		pass[strcspn(pass, "\r\n")] = 0;
	}

	char input_p[64];
	getminput("Enter your password to reach root access: ", input_p, sizeof(input_p));

	for (volatile int i = 0; i < 1000000; i++);

	if (strcmp(input_p, pass) == 0) {
		return 1;
	}
	
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc == 2 && strcmp(argv[1], "-k") == 0) {
		uid_t ruid = getuid();
		if (setreuid(ruid, ruid) == 0) {
			const char *msg = "You are user now.";
			write(STDOUT_FILENO, msg, strlen(msg));
			return 0;
		}
		else {
			printf("autumn-sudo: Error: Could not switch to user mode!\n");
			return 1;
		}

	}

	if (argc < 2) {
		const char *usage = "Usage: autumn-sudo -k or autumn-sudo";
		write(STDERR_FILENO, usage, strlen(usage));
		return 1;
	}

	uid_t ruid = getuid();

	if (ruid != 0) {
		int test = _AutumnSys_ioOpen(PSW_FILE, A_RDONLY, 0);
		if (test < 0) {
			setpswd();
		}
		else {
			_AutumnSys_ioClose(test);
		}

		if (!verify()) {
			const char *err = "autumn-sudo: Error: Wrong password!\n";
			write(STDERR_FILENO, err, strlen(err));
			return 1;
		}
	}

	if (setreuid(0, 0) != 0) {
		printf("autumn-sudo: Error: Could not switch to root mode!\n");
		return 1;
	}

	execvp(argv[1], &argv[1]);

    	printf("autumn-sudo: Error: invalid execvp.");
    	return 1;
}
