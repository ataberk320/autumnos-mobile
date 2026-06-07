#include <sys/un.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
int AutumnAPI_Socket_CreateServer(const char *socket_path) {
        int server_fd;
        struct sockaddr_un addr;
        unlink(socket_path);
        server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_fd < 0) return -1;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
        if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
                close(server_fd);
                return -1;
        }

        if (listen(server_fd, 5) < 0) {
                close(server_fd);
                return -1;
        }

        return server_fd;
}

int AutumnAPI_Socket_SocRead(int server_fd, int *client_fd, char *buffer, int len) {
        
	if (*client_fd < 0) {
                *client_fd = accept(server_fd, NULL, NULL);
		if (*client_fd < 0) return -1;
        }
        
	int bytes = recv(*client_fd, buffer, len, 0);
	
	if (bytes == 0 || (bytes < 0 && errno != EAGAIN)) {
        	close(*client_fd);
        	*client_fd = -1;
        	return -1;
    	}

    return bytes;
}

int AutumnAPI_Socket_ConnSoc(const char *socket_path) {
        int sock = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un addr;
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) return -1;
        return sock;
}

void AutumnAPI_Socket_SendCmd(const char *path, const char *cmd) {
        int fd = AutumnAPI_Socket_ConnSoc(path);
        if (fd >= 0) {
                send(fd, cmd, strlen(cmd), 0);
                close(fd);
        }
}
