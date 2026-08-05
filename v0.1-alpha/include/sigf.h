#ifndef SIG_H
#define SIG_H
#include <signal.h>
#include <setjmp.h>

extern sigjmp_buf jump_buffer;
int set_sig();
void atmsig_handle(int sig, siginfo_t *si, void *unused);
void atm_rsod(int sig, siginfo_t *si, void *unused);
const char* get_sig_name(int sig, char* buffer);

#endif
