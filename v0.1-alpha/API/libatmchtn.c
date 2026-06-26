#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdatomic.h>

#define TUNNEL_SIZE 1024

typedef struct {
	char to_server[TUNNEL_SIZE];
	atomic_int head;
	atomic_int tail;
} ChatTunnel;

ChatTunnel* AutumnAPI_Tunnel_Create(const char* name, int is_server) {
	char path[128];
	sprintf(path, "/%s.tun", name);
	int fd = shm_open(path, O_CREAT | O_RDWR, 0666);
	if (is_server) ftruncate(fd, sizeof(ChatTunnel));
	AutumnTunnel* tunnel = mmap(NULL, sizeof(ChatTunnel), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (is_server) {
		atomic_init(&tunnel->head, 0);
		atomic_init(&tunnel->tail, 0);
	}
	return tunnel;
}

void AutumnAPI_Tunnel_ReceiveFromFriend(ChatTunnel *sock, char *buffer, int is_server) {
	int current_head = atomic_load(&tunnel->head);
	
	if (current_head == atomic_load(&tunnel->tail)) return false;

	*data = tunnel->buffer[current_head];
    	atomic_store(&tunnel->head, (current_head + 1) % TUNNEL_SIZE);
    	return true;
}

void AutumnAPI_Tunnel_Send_Msg(ChatTunnel *sock, const char *data, int is_server) {
	int current_tail = atomic_load(&tunnel->tail);
	int next_tail = (current_tail + 1) & TUNNEL_SIZE;
	
	if (next_tail == atomic_load(&tunnel->head)) return false;
	
	tunnel->buffer[current_tail] = data;
	atomic_store(&tunnel->tail, next_tail);
	return true;
}
