#include <stdio.h>
#include <dlfcn.h>
#include "libfbdev.h"

int main() {
	void* fb_handle = dlopen("/usr/lib/libfbdev.so", RTLD_LAZY);
	if (!fb_handle) {
		printf("Could not load library: %s\n", dlerror());
		return -1;
	}

	void (*AutumnUI_DrawBackground)(FbDev*, const char*, int, int);
	AutumnUI_DrawBackground = dlsym(fb_handle, "AutumnAPI_DrawImg");

	int (*AutumnUI_InitFb)(FbDev*);
    	AutumnUI_InitFb = dlsym(fb_handle, "AutumnAPI_Init");

	if (!AutumnUI_DrawBackground || !AutumnUI_InitFb) {
		printf("Could not find symbol: %s\n", dlerror());
		dlclose(fb_handle);
		return 1;
	}

	FbDev screen;
	if (AutumnUI_InitFb(&screen) != 0) {
        	printf("Framebuffer error!\n");
        	return -1;
    	}
	AutumnUI_DrawBackground(&screen, "/usr/share/test.png", 0, 0);

    	dlclose(fb_handle);
    	return 0;
}
