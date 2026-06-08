#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define MSG_SIZE 512

typedef struct {
	char to_server[MSG_SIZE];
	char to_client[MSG_SIZE];
	int status;
} ChatSock;

ChatSock* AutumnAPI_Sock_Create(const char* name, int is_server) {
	char path[128];
	sprintf(path, "/%s.acsock", name);
	int fd = shm_open(path, O_CREAT | O_RDWR, 0666);
	if (is_server) ftruncate(fd, sizeof(ChatSock));
	return mmap(NULL, sizeof(ChatSock), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
}

int AutumnAPI_Listen_Friend(ChatSock *sock) {
	return (sock->status == 1);
}

void AutumnAPI_Receive_From_Friend(ChatSock *sock, char *buffer, int is_server) {
	if (is_server) strcpy(buffer, sock->to_server);
	else strcpy(buffer, sock->to_client);
}

void AutumnAPI_Send_Msg(ChatSock *sock, const char *data, int is_server) {
    if (is_server) {
        strcpy(sock->to_client, data);
        sock->status = 2; 
    } else {
        strcpy(sock->to_server, data);
        sock->status = 1; 
    }
}
