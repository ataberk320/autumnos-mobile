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
void coreu_exec(const char *cmd);

void coreu_catf(const char *path);

void coreu_crdir(const char *path, int mode);

void coreu_lsdir(const char *path);

void coreu_copyf(const char *src, const char *dest);

void coreu_remf(const char *path);

void coreu_lsdir(const char *path);

void coreu_crfil(const char *filename);

void coreu_mnt(const char *source, const char *target, const char *fs_type);

void coreu_rdsoc(int fd);

void coreu_taskkill(int pid);

void coreu_umnt(const char *target);

int coreu_goto(const char *path);

void coreu_dd(const char *src, const char *dest);

void coreu_pullandgrep(const char *path, const char *str);

void coreu_find(const char *path, const char *filename);

//Root permission functions

void atmsu_root(const char *command);

void coreu_tellwhoami();
#endif
