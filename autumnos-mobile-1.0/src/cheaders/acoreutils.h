#ifndef ACORE_H
#define ACORE_H
#include <stdint.h>

//kfunction

void kmsg_open();

void kmsg(const char *msg);

void kpanic(const char *err);

void kbridge(uintptr_t phys_addr);

void kmsgf(const char *fmt, ...);

void kmap(uintptr_t addr);

//File functions

void coreu_catf(const char *path);

void coreu_crdir(const char *path, int mode);

void coreu_lsdir(const char *path);

void coreu_copyf(const char *src, const char *dest);

void coreu_remf(const char *path);

//Root permission functions

void atmsu_root(const char *command);

void coreu_tellwhoami();
#endif
