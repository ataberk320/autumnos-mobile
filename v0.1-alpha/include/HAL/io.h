#ifndef IO_H
#define IO_H

#define _GNU_SOURCE
#include <stddef.h>
#include <sys/types.h>

#define SYS_LINUX_READ   63
#define SYS_LINUX_WRITE  64
#define SYS_LINUX_OPEN   56
#define SYS_LINUX_CLOSE  57
#define SYS_LINUX_RENAME 38
#define SYS_LINUX_UNLINK 102
#define SYS_LINUX_LSEEK  62

static inline long _AutumnSys_Syscall3(long n, long a0, long a1, long a2) {
        register long ret __asm__("a0") = a0;
        register long _a1 __asm__("a1") = a1;
        register long _a2 __asm__("a2") = a2;
        register long _a7 __asm__("a7") = n;

        __asm__ volatile (
                "ecall"
                : "+r" (ret)
                : "r" (_a1), "r" (_a2), "r" (_a7)
                : "memory"
        );

        return ret;
}

static inline long _AutumnSys_Syscall2(long n, long a0, long a1) {
    register long ret __asm__("a0") = a0;
    register long _a1 __asm__("a1") = a1;
    register long _a7 __asm__("a7") = n;

    __asm__ volatile (
        "ecall"
        : "+r" (ret)
        : "r" (_a1), "r" (_a7)
        : "memory"
    );
    return ret;
}

static inline long _AutumnSys_Syscall1(long n, long a0) {
    register long ret __asm__("a0") = a0;
    register long _a7 __asm__("a7") = n;

    __asm__ volatile (
        "ecall"
        : "+r" (ret)
        : "r" (_a7)
        : "memory"
    );
    return ret;
}

static inline int _AutumnSys_ioOpen(const char* path, int flags, int mode) {
        return (int)_AutumnSys_Syscall3(SYS_LINUX_OPEN, (long)path, (long)flags, (long)mode);
}

static inline ssize_t _AutumnSys_ioRead(int fd, void* buf, size_t count) {
        return _AutumnSys_Syscall3(SYS_LINUX_READ, (long)fd, (long)buf, (long)count);
}

static inline ssize_t _AutumnSys_ioWrite(int fd, const void* buf, size_t count) {
        return _AutumnSys_Syscall3(SYS_LINUX_WRITE, (long)fd, (long)buf, (long)count);
}

static inline int _AutumnSys_ioClose(int fd) {
        return (int)_AutumnSys_Syscall2(SYS_LINUX_CLOSE, (long)fd, 0);
}

static inline int _AutumnSys_ioRename(const char* old_p, const char* new_p) {
    return (int)_AutumnSys_Syscall2(SYS_LINUX_RENAME, (long)old_p, (long)new_p);
}

static inline int _AutumnSys_ioUnlink(const char* path) {
    return (int)_AutumnSys_Syscall1(SYS_LINUX_UNLINK, (long)path);
}

static inline off_t _AutumnSys_ioLseek(int fd, off_t offset, int whence) {
	return (off_t)_AutumnSys_Syscall3(SYS_LINUX_LSEEK, (long)fd, (long)offset, (long)whence);
}

#endif
