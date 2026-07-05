#include <signal.h>
#include <setjmp.h>
#include <stddef.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h> 
#include <linux/fb.h> 
#include <stdint.h>
#include <stdio.h>
#include "sigf.h"

sigjmp_buf jump_buffer;

const char* get_sig_name(int sig, char* buffer) {
    const char* name;
    switch(sig) {
        case SIGSEGV: name = "SEGMENTATION FAULT"; break;
        case SIGFPE:  name = "MATH ERROR";       break;
        case SIGILL:  name = "ILLEGAL INSTR";    break;
        case SIGBUS:  name = "BUS ERROR";        break;
        case SIGABRT: name = "ABORTED";          break;
        default:      name = "UNKNOWN SIGNAL";   break;
    }

    int i = 0;
    while (name[i] != '\0') { buffer[i] = name[i]; i++; }
    buffer[i++] = ' '; buffer[i++] = '(';

    if (sig < 10) {
        buffer[i++] = sig + '0';
    }
    else {
        buffer[i++] = (sig / 10) + '0';
        buffer[i++] = (sig % 10) + '0';
    }
    buffer[i++] = ')';
    buffer[i] = '\0';

    return buffer;
}



void atmsig_handle(int sig, siginfo_t *si, void *unused) {
	static int crash_count = 0;
    	crash_count++;
        
	printf("Critical error handled: Signal %d\n", sig, crash_count);

	if (crash_count >= 10) {
        	printf("Too many crashes. Halting process.\n");
        	sync();
        	_exit(1);
    	}

        switch(sig) {
                case SIGSEGV: printf("Reason: Segmentation fault\n"); break;
                case SIGFPE: printf("Reason: Invalid math operation\n"); break;
                case SIGILL: printf("Reason: Illegal instruction\n"); break;
                case SIGBUS: printf("Reason: Bus error\n"); break;
                default: printf("Reason: Unknown critical signal.\n"); break;
        }


        siglongjmp(jump_buffer, sig);
}

int set_sig() {
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sigemptyset(&sa.sa_mask);
        sa.sa_sigaction = atmsig_handle;

        sigaction(SIGSEGV, &sa, NULL);
        sigaction(SIGFPE,  &sa, NULL);
        sigaction(SIGILL,  &sa, NULL);
        sigaction(SIGBUS,  &sa, NULL);
        sigaction(SIGABRT, &sa, NULL);
}
