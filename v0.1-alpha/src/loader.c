#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/ioctl.h>
#include "../include/SymbolTable/table.h"
#include "../include/HAL/io.h"
#include "../include/HAL/HAL_Symb.h"
#include "../include/HAL/autumn_ioctl.h"

#define MAX_HANDLES 128

static void* loaded_handles[MAX_HANDLES];
static int handle_count = 0;

typedef int (*hal_init_func_t)(void);
typedef void (*api_init_func_t)(SYM_TAB*);

SYM_TAB *symb = NULL;

void load_hal_directory(int tab_fd, const char *base_path) {
    DIR *dir = opendir(base_path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, entry->d_name);

        struct stat st;
        int is_dir = 0, is_regl = 0;

        if (entry->d_type == DT_DIR) is_dir = 1;
        else if (entry->d_type == DT_REG) is_regl = 1;
        else {
            if (stat(full_path, &st) == 0) {
                if (S_ISDIR(st.st_mode)) is_dir = 1;
                else if (S_ISREG(st.st_mode)) is_regl = 1;
            }
        }
        
        if (is_dir) {
            load_hal_directory(tab_fd, full_path);
        }
        else if (is_regl) {
            size_t len = strlen(entry->d_name);
            if (len > 3 && strcmp(entry->d_name + len - 3, ".so") == 0) {
                void *handle = dlopen(full_path, RTLD_GLOBAL | RTLD_NOW);
                if (!handle) {
                    fprintf(stderr, "loader: HAL Error: Could not load [%s]: %s\n", full_path, dlerror());
                } else {
                    printf("loader: HAL Loaded: %s\n", full_path);
                    if (handle_count < MAX_HANDLES) {
                        loaded_handles[handle_count++] = handle;
                    }

                    dlerror();
                    hal_init_func_t hal_init = (hal_init_func_t)dlsym(handle, "hal_init");
                    if (hal_init) {
                        int ret = hal_init();
                        printf("loader: HAL Init Success (code: %d) -> [%s]\n", ret, entry->d_name);
                    } else {
                        printf("loader: HAL Notice: [%s] has no hal_init function.\n", entry->d_name);
                    }
                }
            }
        }
    }
    closedir(dir);
}

void load_api_directory(const char *base_path, SYM_TAB *target_symb) {
    DIR *dir = opendir(base_path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, entry->d_name);

        struct stat st;
        int is_dir = 0, is_regl = 0;

        if (entry->d_type == DT_DIR) is_dir = 1;
        else if (entry->d_type == DT_REG) is_regl = 1;
        else {
            if (stat(full_path, &st) == 0) {
                if (S_ISDIR(st.st_mode)) is_dir = 1;
                else if (S_ISREG(st.st_mode)) is_regl = 1;
            }
        }
        
        if (is_dir) {
            load_api_directory(full_path, target_symb);
        }
        else if (is_regl) {
            size_t len = strlen(entry->d_name);
            if (len > 3 && strcmp(entry->d_name + len - 3, ".so") == 0) {
                void *handle = dlopen(full_path, RTLD_GLOBAL | RTLD_NOW);
                if (!handle) {
                    fprintf(stderr, "loader: API Error: Could not load [%s]: %s\n", full_path, dlerror());
                } else {
                    printf("loader: API Loaded: %s\n", full_path);
                    if (handle_count < MAX_HANDLES) {
                        loaded_handles[handle_count++] = handle;
                    }

                    dlerror();
                    api_init_func_t api_init = (api_init_func_t)dlsym(handle, "api_init");
                    if (api_init) {
                        api_init(target_symb);
                        printf("loader: API Symbols Mapped -> [%s]\n", entry->d_name);
                    } else {
                        printf("loader: API Notice: [%s] has no api_init function (Pure wrapper).\n", entry->d_name);
                    }
                }
            }
        }
    }
    closedir(dir);
}

void cleanup() {
    for (int i = handle_count - 1; i >= 0; i--) {
        if (loaded_handles[i]) {
            dlclose(loaded_handles[i]);
            loaded_handles[i] = NULL;
        }
    }
    handle_count = 0;
}
        
void ldinit() {
    memset(loaded_handles, 0, sizeof(loaded_handles));
    handle_count = 0;

    int tab_fd = _AutumnSys_ioOpen("/dev/tabldev", A_RDWR, 0);
    if (tab_fd < 0) {
        perror("loader -> tabldev: Error");
        exit(1);
    }

    extern SYM_TAB *sym;
    symb = sym;
    if (!symb) {
        printf("loader: Error: NULL Global Symbol Table. Allocating now.\n");
        symb = (SYM_TAB*)calloc(1, sizeof(SYM_TAB));
    }

    printf("loader: Notify: Scanning and loading hardware layers from /usr/lib/hal...\n");
    load_hal_directory(tab_fd, "/usr/lib/hal");

    printf("loader: Notify: Scanning and loading system APIs from /usr/lib/api...\n");
    load_api_directory("/usr/lib/api", symb);

    unsigned long glob_tab_ptr = (unsigned long)symb;
    if (ioctl(tab_fd, IOCTBLREGSRV, glob_tab_ptr) == 0) {
        printf("SYM_TAB Address mapped to Tabloader device: %p\n", (void*)glob_tab_ptr);
    }
    else {
        perror("loader: Error: ");
    }
}
