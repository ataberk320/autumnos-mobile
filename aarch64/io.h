
#ifndef AARCH64_IO_H
#define AARCH64_IO_H

#include <stddef.h>
#include <sys/types.h>

#define SYS_LINUX_READ       63
#define SYS_LINUX_WRITE      64
#define SYS_LINUX_OPENAT     56
#define SYS_LINUX_CLOSE      57
#define SYS_LINUX_LSEEK      62
#define SYS_LINUX_RENAMEAT   38
#define SYS_LINUX_UNLINKAT   35

#define AT_FDCWD (-100)


static inline long _AutumnSys_Syscall1(
    long n,
    long a0
) {
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


static inline long _AutumnSys_Syscall2(
    long n,
    long a0,
    long a1
) {
    register long x0 __asm__("x0") = a0;
    register long x1 __asm__("x1") = a1;
    register long x8 __asm__("x8") = n;

    __asm__ volatile (
        "svc #0"
        : "+r"(x0)
        : "r"(x1),
          "r"(x8)
        : "memory"
    );

    return x0;
}


static inline long _AutumnSys_Syscall3(
    long n,
    long a0,
    long a1,
    long a2
) {
    register long x0 __asm__("x0") = a0;
    register long x1 __asm__("x1") = a1;
    register long x2 __asm__("x2") = a2;
    register long x8 __asm__("x8") = n;

    __asm__ volatile (
        "svc #0"
        : "+r"(x0)
        : "r"(x1),
          "r"(x2),
          "r"(x8)
        : "memory"
    );

    return x0;
}


static inline long _AutumnSys_Syscall4(
    long n,
    long a0,
    long a1,
    long a2,
    long a3
) {
    register long x0 __asm__("x0") = a0;
    register long x1 __asm__("x1") = a1;
    register long x2 __asm__("x2") = a2;
    register long x3 __asm__("x3") = a3;
    register long x8 __asm__("x8") = n;

    __asm__ volatile (
        "svc #0"
        : "+r"(x0)
        : "r"(x1),
          "r"(x2),
          "r"(x3),
          "r"(x8)
        : "memory"
    );

    return x0;
}


static inline int _AutumnSys_ioOpen(
    const char* path,
    int flags,
    int mode
) {
    return (int)_AutumnSys_Syscall4(
        SYS_LINUX_OPENAT,
        AT_FDCWD,
        (long)path,
        (long)flags,
        (long)mode
    );
}


static inline ssize_t _AutumnSys_ioRead(
    int fd,
    void* buf,
    size_t count
) {
    return (ssize_t)_AutumnSys_Syscall3(
        SYS_LINUX_READ,
        (long)fd,
        (long)buf,
        (long)count
    );
}


static inline ssize_t _AutumnSys_ioWrite(
    int fd,
    const void* buf,
    size_t count
) {
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


static inline int _AutumnSys_ioRename(
    const char* old_p,
    const char* new_p
) {
    return (int)_AutumnSys_Syscall4(
        SYS_LINUX_RENAMEAT,
        AT_FDCWD,
        (long)old_p,
        AT_FDCWD,
        (long)new_p
    );
}


static inline int _AutumnSys_ioUnlink(
    const char* path
) {
    return (int)_AutumnSys_Syscall3(
        SYS_LINUX_UNLINKAT,
        AT_FDCWD,
        (long)path,
        0
    );
}


static inline off_t _AutumnSys_ioLseek(
    int fd,
    off_t offset,
    int whence
) {
    return (off_t)_AutumnSys_Syscall3(
        SYS_LINUX_LSEEK,
        (long)fd,
        (long)offset,
        (long)whence
    );
}

#endif
```
