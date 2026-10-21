#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "../include/HAL/io.h"
#include "../include/SymbolTable/table.h"
#include "../include/HAL/autumn_ioctl.h"

#define TABLDEV "/dev/tabldev"

static SYM_TAB *sym = NULL;


extern void ldinit(void);

SYM_TAB* _AutumnSys_SaveTable(void) {
    if (sym) {
        return sym;
    }

    ldinit();

    int fd = _AutumnSys_ioOpen(TABLDEV, A_RDWR, 0);
    if (fd < 0) {
        perror("TabLoader: Error: /dev/tabldev open failed");
        return NULL;
    }

    size_t table_s = 0;
    if (ioctl(fd, IOCTBLGETSRV, &table_s) < 0) {
        perror("TabLoader: Error: IOCTBLGETSRV size sync failed");
        _AutumnSys_ioClose(fd);
        return NULL;
    }

    sym = mmap(NULL, table_s, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    if (sym == MAP_FAILED) {
        perror("TabLoader: Error: mmap MAP_SHARED failed");
        sym = NULL;
        _AutumnSys_ioClose(fd);
        return NULL;
    }

    if (ioctl(fd, IOCTBLREGSRV, sym) < 0) {
        perror("TabLoader: Error: IOCTBLREGSRV kernel registration failed");
    }
    else {
        printf("TabLoader: Success: System Symbol Table successfully SAVED and registered to kernel space!\n");
    }

    return sym;
}

SYM_TAB* _AutumnSys_GetTable(void) {
	if (sym) {
		return sym;
	}

	ldinit();

	int fd = _AutumnSys_ioOpen(TABLDEV, A_RDWR, 0);
	if (fd < 0) {
		perror("tabldev - ioOpen");
		return NULL;
	}

	
	size_t table_s = 0;
	if (ioctl(fd, IOCTBLGETSRV, &table_s) < 0) {
		perror("IOCTBLGETSRV");
		_AutumnSys_ioClose(fd);
		return NULL;
	}

	sym = mmap(NULL, table_s, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    	if (sym == MAP_FAILED) {
        	perror("mmap failed");
        	sym = NULL;
	        _AutumnSys_ioClose(fd);
                return NULL;
    	}

	return sym;
}
