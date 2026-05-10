#include "cheaders/AutumnSyscall.h"

long AutumnSys_Syscall(long num, long a0, long a1, long a2, long a3, long a4, long a5) {
	long ret;
	register long r7 asm("a7") = num;
	register long r0 asm("a0") = a0;
	register long r1 asm("a1") = a1;
	register long r2 asm("a2") = a2;
	register long r3 asm("a3") = a3;
	register long r4 asm("a4") = a4;
	register long r5 asm("a5") = a5;

	asm volatile (
		"ecall\n"
		: "+r"(r0)
		: "r"(r7), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5)
		: "memory"
	);
	ret = r0;
	return ret;
}
