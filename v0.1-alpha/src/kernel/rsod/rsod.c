void _start(unsigned long fb_base) {
	volatile unsigned int *fb = (unsigned int *)fb_base;

    	for (int i = 0; i < (240 * 320); i++) {
        	fb[i] = 0x00FF0000;
    	}

    	while(1) {
        	asm volatile("wfi");
    	}
}
