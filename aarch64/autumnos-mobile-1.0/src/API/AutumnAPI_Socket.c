#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdatomic.h>

#define MSG_SIZE 512

typedef struct {
	char to_server[MSG_SIZE];
	char to_client[MSG_SIZE];
	atomic_int status;
	atomic_flag lock;
} ChatSock;

ChatSock* AutumnAPI_Sock_Create(const char* name, int is_server) {
	char path[128];
	sprintf(path, "/%s.acsock", name);
	int fd = shm_open(path, O_CREAT | O_RDWR, 0666);
	if (is_server) ftruncate(fd, sizeof(ChatSock));
	ChatSock *sock = (ChatSock*)mmap(NULL, sizeof(ChatSock), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	if (is_server) {
        	atomic_flag_clear(&sock->lock);
        	atomic_init(&sock->status, 0);
    	}
    
    	return sock;
}

void AutumnAPI_LockSock(ChatSock *sock) {
    while (atomic_flag_test_and_set(&sock->lock));
}

void AutumnAPI_UnlockSock(ChatSock *sock) {
    atomic_flag_clear(&sock->lock);
}

int AutumnAPI_Listen_Friend(ChatSock *sock) {
	return (sock->status == 1);
}

void AutumnAPI_Receive_From_Friend(ChatSock *sock, char *buffer, int is_server) {
	if (is_server) strcpy(buffer, sock->to_server);
	else strcpy(buffer, sock->to_client);
}

void AutumnAPI_Send_Msg(ChatSock *sock, const char *data, int is_server) {
    AutumnAPI_LockSock(sock);
	if (is_server) {
        strncpy(sock->to_client, data, MSG_SIZE - 1);
        atomic_store(&sock->status, 2); 
    } else {
        strncpy(sock->to_server, data, MSG_SIZE - 1);
        atomic_store(&sock->status, 1); 
    }
	AutumnAPI_UnlockSock(sock);
}
