#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>

int main() {
	int wd_fd = open("/dev/watchdog", O_WRONLY);
	if (wd_fd == -1) {
		std::cerr <<"[AUTUMNOS] Hardware watchdog is not compatible with your machine!" << std::endl; 
		return 1;
	}
	std::cout << "[AUTUMNOS] Hardware watchdog is active!" << std::endl;
	
	while (true) {
		ioctl(wd_fd, WDIOC_KEEPALIVE, 0);
		sleep(5);
	}

	close(wd_fd);
	return 0;
}
