#include "AutumnSyscall.h"

long AutumnSys_Syscall(long num, long a0, long a1, long a2, long a3, long a4, long a5) {
	long ret;
	register long r8 asm("x8") = num;
	register long r0 asm("x0") = a0;
	register long r1 asm("x1") = a1;
	register long r2 asm("x2") = a2;
	register long r3 asm("x3") = a3;
	register long r4 asm("x4") = a4;
	register long r5 asm("x5") = a5;

	asm volatile (
		"svc #0\n"
		: "+r"(r0)
		: "r"(r8), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5)
		: "memory"
	);
	return r0;
}
