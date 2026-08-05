#include <png.h>
#include <stdint.h>
#include <sys/types.h>
#include "io.h"

void _png_RdCallback(png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead) {
	int fd = (int)(intptr_t)png_get_io_ptr(png_ptr);
	ssize_t bytesRead = _AutumnSys_ioRead(fd, outBytes, byteCountToRead
	if (bytesRead < 0) {
		png_error(png_ptr, "AutumnSyscall: Reading error");
	}
}
