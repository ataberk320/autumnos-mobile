#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

#define SYS_MMAP 222
#define SYS_MUNMAP 215
#define SYS_BRK 214
#define SYS_CLONE 220
#define SYS_EXECVE 221
#define SYS_WAIT4 260
#define SYS_GETPID 172
#define SYS_NANOSLEEP 101
#define SYS_RT_SIGPROCMASK 135
#define SYS_REBOOT 142
#define SYS_SETUID 146

long AutumnSys_Syscall(long num, long a0, long a1, long a2, long a3, long a4, long a5);

#endif
