
#ifndef IO_H
#define IO_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* =========================================
 * AutumnOS File Flags
 * Linux-compatible values
 * ========================================= */

#define A_RDONLY    O_RDONLY
#define A_WRONLY    O_WRONLY
#define A_RDWR      O_RDWR
#define A_CRT       O_CREAT
#define A_EXCL      O_EXCL
#define A_NOCTTY    O_NOCTTY
#define A_TRUNC     O_TRUNC
#define A_APPEND    O_APPEND
#define A_NONBLOCK  O_NONBLOCK
#define A_CLOEXEC   O_CLOEXEC

#ifndef AT_FDCWD
#define AT_FDCWD (-100)
#endif

/* File permissions */

#define A_IRUSR S_IRUSR
#define A_IWUSR S_IWUSR
#define A_IXUSR S_IXUSR

#define A_IRGRP S_IRGRP
#define A_IWGRP S_IWGRP
#define A_IXGRP S_IXGRP

#define A_IROTH S_IROTH
#define A_IWOTH S_IWOTH
#define A_IXOTH S_IXOTH

#define A_IRWXU S_IRWXU
#define A_IRWXG S_IRWXG
#define A_IRWXO S_IRWXO

/* =========================================
 * Architecture: x86_64
 * ========================================= */

#if defined(__x86_64__)

#define SYS_LINUX_READ       0
#define SYS_LINUX_WRITE      1
#define SYS_LINUX_CLOSE      3
#define SYS_LINUX_LSEEK      8
#define SYS_LINUX_OPENAT     257

static inline long _AutumnSys_Syscall1(long n, long a0)
{
    register long rax __asm__("rax") = n;
    register long rdi __asm__("rdi") = a0;

    __asm__ volatile (
        "syscall"
        : "+a"(rax)
        : "D"(rdi)
        : "rcx", "r11", "memory"
    );

    return rax;
}

static inline long _AutumnSys_Syscall3(
    long n, long a0, long a1, long a2)
{
    register long rax __asm__("rax") = n;
    register long rdi __asm__("rdi") = a0;
    register long rsi __asm__("rsi") = a1;
    register long rdx __asm__("rdx") = a2;

    __asm__ volatile (
        "syscall"
        : "+a"(rax)
        : "D"(rdi), "S"(rsi), "d"(rdx)
        : "rcx", "r11", "memory"
    );

    return rax;
}

static inline long _AutumnSys_Syscall4(
    long n, long a0, long a1, long a2, long a3)
{
    register long rax __asm__("rax") = n;
    register long rdi __asm__("rdi") = a0;
    register long rsi __asm__("rsi") = a1;
    register long rdx __asm__("rdx") = a2;
    register long r10 __asm__("r10") = a3;

    __asm__ volatile (
        "syscall"
        : "+a"(rax)
        : "D"(rdi), "S"(rsi), "d"(rdx), "r"(r10)
        : "rcx", "r11", "memory"
    );

    return rax;
}

/* =========================================
 * Architecture: RISC-V 64
 * ========================================= */

#elif defined(__riscv) && __riscv_xlen == 64

#define SYS_LINUX_READ       63
#define SYS_LINUX_WRITE      64
#define SYS_LINUX_CLOSE      57
#define SYS_LINUX_LSEEK      62
#define SYS_LINUX_OPENAT     56

static inline long _AutumnSys_Syscall1(long n, long a0)
{
    register long a0_reg __asm__("a0") = a0;
    register long a7_reg __asm__("a7") = n;

    __asm__ volatile (
        "ecall"
        : "+r"(a0_reg)
        : "r"(a7_reg)
        : "memory"
    );

    return a0_reg;
}

static inline long _AutumnSys_Syscall3(
    long n, long a0, long a1, long a2)
{
    register long a0_reg __asm__("a0") = a0;
    register long a1_reg __asm__("a1") = a1;
    register long a2_reg __asm__("a2") = a2;
    register long a7_reg __asm__("a7") = n;

    __asm__ volatile (
        "ecall"
        : "+r"(a0_reg)
        : "r"(a1_reg), "r"(a2_reg), "r"(a7_reg)
        : "memory"
    );

    return a0_reg;
}

static inline long _AutumnSys_Syscall4(
    long n, long a0, long a1, long a2, long a3)
{
    register long a0_reg __asm__("a0") = a0;
    register long a1_reg __asm__("a1") = a1;
    register long a2_reg __asm__("a2") = a2;
    register long a3_reg __asm__("a3") = a3;
    register long a7_reg __asm__("a7") = n;

    __asm__ volatile (
        "ecall"
        : "+r"(a0_reg)
        : "r"(a1_reg), "r"(a2_reg),
          "r"(a3_reg), "r"(a7_reg)
        : "memory"
    );

    return a0_reg;
}

/* =========================================
 * Architecture: AArch64
 * ========================================= */

#elif defined(__aarch64__)

#define SYS_LINUX_READ       63
#define SYS_LINUX_WRITE      64
#define SYS_LINUX_CLOSE      57
#define SYS_LINUX_LSEEK      62
#define SYS_LINUX_OPENAT     56

static inline long _AutumnSys_Syscall1(long n, long a0)
{
    register long x0 __asm__("x0") = a0;
    register long x8 __asm__("x8") = n;

    __asm__ volatile (
        "svc #0"
        : "+r"(x0)
        : "r"(x8)
        : "memory"
    );

    return x0;
}

static inline long _AutumnSys_Syscall3(
    long n, long a0, long a1, long a2)
{
    register long x0 __asm__("x0") = a0;
    register long x1 __asm__("x1") = a1;
    register long x2 __asm__("x2") = a2;
    register long x8 __asm__("x8") = n;

    __asm__ volatile (
        "svc #0"
        : "+r"(x0)
        : "r"(x1), "r"(x2), "r"(x8)
        : "memory"
    );

    return x0;
}

static inline long _AutumnSys_Syscall4(
    long n, long a0, long a1, long a2, long a3)
{
    register long x0 __asm__("x0") = a0;
    register long x1 __asm__("x1") = a1;
    register long x2 __asm__("x2") = a2;
    register long x3 __asm__("x3") = a3;
    register long x8 __asm__("x8") = n;

    __asm__ volatile (
        "svc #0"
        : "+r"(x0)
        : "r"(x1), "r"(x2),
          "r"(x3), "r"(x8)
        : "memory"
    );

    return x0;
}

#else

#error "AutumnOS: Unsupported architecture. Supported: x86_64, riscv64, aarch64."

#endif

/* =========================================
 * Common I/O API
 * ========================================= */

/* openat(dirfd, path, flags, mode) */

static inline int _AutumnSys_ioOpen(
    const char *path,
    int flags,
    int mode)
{
    long ret = _AutumnSys_Syscall4(
        SYS_LINUX_OPENAT,
        AT_FDCWD,
        (long)path,
        (long)flags,
        (long)mode
    );

    return (int)ret;
}

static inline ssize_t _AutumnSys_ioRead(
    int fd,
    void *buf,
    size_t count)
{
    return (ssize_t)_AutumnSys_Syscall3(
        SYS_LINUX_READ,
        (long)fd,
        (long)buf,
        (long)count
    );
}

static inline ssize_t _AutumnSys_ioWrite(
    int fd,
    const void *buf,
    size_t count)
{
    return (ssize_t)_AutumnSys_Syscall3(
        SYS_LINUX_WRITE,
        (long)fd,
        (long)buf,
        (long)count
    );
}

static inline int _AutumnSys_ioClose(int fd)
{
    return (int)_AutumnSys_Syscall1(
        SYS_LINUX_CLOSE,
        (long)fd
    );
}

static inline off_t _AutumnSys_ioLseek(
    int fd,
    off_t offset,
    int whence)
{
    return (off_t)_AutumnSys_Syscall3(
        SYS_LINUX_LSEEK,
        (long)fd,
        (long)offset,
        (long)whence
    );
}

#endif
