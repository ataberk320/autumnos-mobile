#include <stdint.h>

uint32_t AutumnAPI_UnicodeTF8(const char** s) {
	const unsigned char* p = (const unsigned char*)*s;
	if (!*p) return 0;
	
	if (p[0] < 0x80) {
		*s += 1;
		return p[0];
	}
	else if ((p[0] & 0xE0) == 0xC0) {
		if (p[1] == 0) return 0xFFFD;
		*s += 2;
		return ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
	}
	else if ((p[0] & 0xF0) == 0xE0) {
		if (p[1] == 0 || p[2] == 0) return 0xFFFD;
        	*s += 3;
		return ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
    	}
	else if ((p[0] & 0xF8) == 0xF0) {
        	if (p[1] == 0 || p[2] == 0 || p[3] == 0) return 0xFFFD;
        	*s += 4;
        	return ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
    	}
	*s += 1;
	return 0xFFFD;
}

