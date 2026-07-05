#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdatomic.h>
#include "libatmchtn.h"
#include <stdio.h>    // sprintf için
#include <string.h>
ChatTunnel* AutumnAPI_Tunnel_Create(const char* name) {
    char path[128];
    sprintf(path, "/%s.tun", name);
    int fd = shm_open(path, O_CREAT | O_RDWR, 0666);
    if (fd == -1) return NULL;

    ftruncate(fd, sizeof(ChatTunnel));
    
    ChatTunnel* tunnel = (ChatTunnel*)mmap(NULL, sizeof(ChatTunnel), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    if (tunnel != MAP_FAILED) {
        atomic_init(&tunnel->head, 0);
        atomic_init(&tunnel->tail, 0);
    }
    return tunnel;
}

ChatTunnel* AutumnAPI_Tunnel_Connect(const char* name) {
    char path[128];
    sprintf(path, "/%s.tun", name);
    int fd = shm_open(path, O_RDWR, 0666);
    if (fd == -1) return NULL;

    ChatTunnel* tunnel = (ChatTunnel*)mmap(NULL, sizeof(ChatTunnel), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    return (tunnel == MAP_FAILED) ? NULL : tunnel;
}

bool AutumnAPI_Tunnel_ReceiveFromFriend(ChatTunnel *tunnel, char *buffer) {
    int head = atomic_load_explicit(&tunnel->head, memory_order_acquire);
    int tail = atomic_load_explicit(&tunnel->tail, memory_order_acquire);

    if (head == tail) return false;

    memcpy(buffer, tunnel->buffer[head], MSG_LEN);
    
    atomic_store_explicit(&tunnel->head, (head + 1) % TUNNEL_SIZE, memory_order_release);
    return true;
}

bool AutumnAPI_Tunnel_Send_Msg(ChatTunnel *tunnel, const char *data) {
    int tail = atomic_load_explicit(&tunnel->tail, memory_order_acquire);
    int head = atomic_load_explicit(&tunnel->head, memory_order_acquire);

    int next_tail = (tail + 1) % TUNNEL_SIZE;
    if (next_tail == head) return false;

    memcpy(tunnel->buffer[tail], data, MSG_LEN - 1);
    
    atomic_store_explicit(&tunnel->tail, next_tail, memory_order_release);
    return true;
}
