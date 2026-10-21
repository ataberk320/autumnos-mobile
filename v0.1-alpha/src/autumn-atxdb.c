#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <pty.h>
#include <dirent.h>
#include <sys/mount.h>

#define GADGET_DIR "/sys/kernel/config/usb_gadget/atxdb"
#define FFS_DIR "/dev/usb-ffs/atxdb"
#define MAX_EVENTS 10

int ufwrite(const char *path, const char *str) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    ssize_t ret = write(fd, str, strlen(str));
    close(fd);
    return (ret < 0) ? -1 : 0;
}

void u_gad_set(void) {
    struct stat st;
    if (stat(GADGET_DIR, &st) == 0) return;

    mkdir(GADGET_DIR, 0755);
    char path_buf[512];

    // VID: 0x4154 (AT), PID: 0x1EAF
    snprintf(path_buf, sizeof(path_buf), "%s/idVendor", GADGET_DIR);
    ufwrite(path_buf, "0x4154");

    snprintf(path_buf, sizeof(path_buf), "%s/idProduct", GADGET_DIR);
    ufwrite(path_buf, "0x1EAF");

    snprintf(path_buf, sizeof(path_buf), "%s/bcdDevice", GADGET_DIR);
    ufwrite(path_buf, "0x0100");

    snprintf(path_buf, sizeof(path_buf), "%s/bcdUSB", GADGET_DIR);
    ufwrite(path_buf, "0x0200");

    snprintf(path_buf, sizeof(path_buf), "%s/strings/0x409", GADGET_DIR);
    mkdir(path_buf, 0755);

    snprintf(path_buf, sizeof(path_buf), "%s/strings/0x409/serialnumber", GADGET_DIR);
    ufwrite(path_buf, "AUTUMN-D1-001");

    snprintf(path_buf, sizeof(path_buf), "%s/strings/0x409/manufacturer", GADGET_DIR);
    ufwrite(path_buf, "Autix Ecosystem");

    snprintf(path_buf, sizeof(path_buf), "%s/strings/0x409/product", GADGET_DIR);
    ufwrite(path_buf, "AutumnOS Debug Terminal");

    snprintf(path_buf, sizeof(path_buf), "%s/configs/c.1/strings/0x409", GADGET_DIR);
    mkdir(path_buf, 0755);
    snprintf(path_buf, sizeof(path_buf), "%s/configs/c.1/strings/0x409/configuration", GADGET_DIR);
    ufwrite(path_buf, "Config 1");

    snprintf(path_buf, sizeof(path_buf), "%s/configs/c.1/MaxPower", GADGET_DIR);
    ufwrite(path_buf, "120");

    snprintf(path_buf, sizeof(path_buf), "%s/functions/ffs.atxdb", GADGET_DIR);
    mkdir(path_buf, 0755);

    mkdir("/dev/usb-ffs", 0755);
    mkdir(FFS_DIR, 0755);
    mount("atxdb", FFS_DIR, "functionfs", 0, NULL);

    char sym_src[512], sym_dst[512];
    snprintf(sym_src, sizeof(sym_src), "%s/functions/ffs.atxdb", GADGET_DIR);
    snprintf(sym_dst, sizeof(sym_dst), "%s/configs/c.1/ffs.atxdb", GADGET_DIR);
    symlink(sym_src, sym_dst);

    DIR *d = opendir("/sys/class/udc");
    if (d) {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_name[0] != '.') {
                snprintf(path_buf, sizeof(path_buf), "%s/UDC", GADGET_DIR);
                ufwrite(path_buf, dir->d_name);
                break;
            }
        }
        closedir(d);
    }
}

void daemonize(void) {
	pid_t pid = fork();
	if (pid < 0) exit(EXIT_FAILURE);
	if (pid > 0) exit(EXIT_SUCCESS);
	
	if (setsid() < 0) exit(EXIT_FAILURE);

	signal(SIGHUP, SIG_IGN);
    	signal(SIGCHLD, SIG_IGN);

    	pid = fork();
    	if (pid < 0) exit(EXIT_FAILURE);
    	if (pid > 0) exit(EXIT_SUCCESS);

	umask(0);
	chdir("/");
	
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	open("/dev/null", O_RDONLY);
    	open("/dev/null", O_WRONLY);
    	open("/dev/null", O_WRONLY);
}

int main(void) {
	u_gad_set();
    daemonize();

    char ep0_path[256];
    char ep_out_path[256];
    char ep_in_path[256];
    
    snprintf(ep0_path, sizeof(ep0_path), "%s/ep0", FFS_DIR);
    snprintf(ep_out_path, sizeof(ep_out_path), "%s/ep1", FFS_DIR);
    snprintf(ep_in_path, sizeof(ep_in_path), "%s/ep2", FFS_DIR);

    int ep0_fd = open(ep0_path, O_RDWR);
    if (ep0_fd < 0) return 1;

    int ep_out_fd = open(ep_out_path, O_RDWR);
    int ep_in_fd = open(ep_in_path, O_RDWR);

    if (ep_out_fd < 0 || ep_in_fd < 0) {
        close(ep0_fd);
        return 1;
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        close(ep0_fd);
        close(ep_out_fd);
        close(ep_in_fd);
        return 1;
    }

    // create pty(pseudo term)
    int master_fd;
    pid_t shell_pid = forkpty(&master_fd, NULL, NULL, NULL);
    if (shell_pid < 0) {
        close(epoll_fd);
        close(ep0_fd);
        close(ep_out_fd);
        close(ep_in_fd);
        return 1;
    } else if (shell_pid == 0) {
        execl("/bin/mksh", "mksh", NULL);
        execl("/bin/sh", "sh", NULL); //fallback
        _exit(127);
    }

	//master pty and epoll
    struct epoll_event ev1, ev2;
    ev1.events = EPOLLIN | EPOLLRDHUP;
    ev1.data.fd = ep_out_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ep_out_fd, &ev1);

    ev2.events = EPOLLIN | EPOLLRDHUP;
    ev2.data.fd = master_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, master_fd, &ev2);

    char buf[4096];
    struct epoll_event events[MAX_EVENTS];

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            if (errno == EINTR) continue;
            break;
        }

        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd;

            if (fd == ep_out_fd) {
                ssize_t n = read(ep_out_fd, buf, sizeof(buf)); //read command
                if (n > 0) {
                    write(master_fd, buf, n); //redirect output(command) to mksh
                } else if (n <= 0) {
                    break;
                }
            } else if (fd == master_fd) {
                ssize_t n = read(master_fd, buf, sizeof(buf)); //send tty output to usb bulk
                if (n > 0) {
                    write(ep_in_fd, buf, n);
                } else if (n <= 0) {
                    break;
                }
            }
        }
    }

    close(master_fd);
    close(epoll_fd);
    close(ep0_fd);
    close(ep_out_fd);
    close(ep_in_fd);
    return 0;
}
