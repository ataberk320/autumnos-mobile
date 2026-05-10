#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "acoreutils.h"

void procterm (pid_t pid, const char* process_name) {
	kmsgf("Unfortunately %s is stopped!", process_name);
	if (kill(pid, SIGTERM) == 0) {
		int timeout = 5;
		while (timeout > 0 && kill(pid, 0) == 0) {
			usleep(100000);
			timeout--;
		}
	}
	
	if (kill(pid, 0) == 0) {
		kmsgf("Unfortunately %s is fatally stopped!", process_name);
		kill(pid, SIGKILL);
	}
	else { }
}

void handle(int sig, siginfo_t *info, void *context) {
	pid_t sender_pid = info->si_pid;
	void *fault_addr  = info->si_addr;

	kmsgf("[AUTUMNOS]: Critical signal %d received from PID %d", sig, sender_pid);
    	kmsgf("[AUTUMNOS]: Memory fault detected at address %p", fault_addr);

    switch (sig) {
        case SIGSEGV:
            kmsg("[AUTUMNOS]: Segmentation fault. Unauthorized memory access.");
            break;
        case SIGFPE:
            kmsg("[AUTUMNOS]: Floating point exception. Invalid arithmetic operation.");
            break;
        case SIGILL:
            kmsg("[AUTUMNOS]: Illegal instruction. Architecture-incompatible binary detected.");
            break;
        case SIGBUS:
            kmsg("[AUTUMNOS]: Bus error. Memory alignment or physical access violation.");
            break;
        case SIGABRT:
            kmsg("[AUTUMNOS]: Process self-termination requested (Abort).");
            break;
        case SIGSYS:
            kmsg("[AUTUMNOS]: Invalid system call. Non-existent syscall requested.");
            break;
        case SIGTRAP:
            kmsg("[AUTUMNOS]: Trace/breakpoint trap triggered.");
            break;
        case SIGPIPE:
            kmsg("[AUTUMNOS]: Broken pipe. Write attempt on a disconnected pipe.");
            break;
        case SIGALRM:
            kmsg("[AUTUMNOS]: Timer expiration. Process execution timeout.");
            break;
        case SIGQUIT:
            kmsg("[AUTUMNOS]: Termination requested via terminal signal.");
            break;
        default:
            kmsg("[AUTUMNOS]: Unidentified system event recorded.");
            break;
    }
}
void sigact() {
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handle;
	sigemptyset(&sa.sa_mask);
	
	sigaction(SIGSEGV, &sa, NULL);
    	sigaction(SIGFPE,  &sa, NULL);
    	sigaction(SIGILL,  &sa, NULL);
    	sigaction(SIGBUS,  &sa, NULL);

    	sigaction(SIGABRT, &sa, NULL);
    	sigaction(SIGSYS,  &sa, NULL);
    	sigaction(SIGTRAP, &sa, NULL);
    
    	sigaction(SIGPIPE, &sa, NULL);
    	sigaction(SIGALRM, &sa, NULL);
    	sigaction(SIGQUIT, &sa, NULL);
}

int main() {
	sigact();
}
