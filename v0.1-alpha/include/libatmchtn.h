#ifndef LIBATMCHTN_H
#define LIBATMCHTN_H

#include <stdatomic.h>
#include <stdbool.h>

#define TUNNEL_SIZE 256
#define MSG_LEN 64

typedef struct {
    atomic_int head;
    atomic_int tail;
    char buffer[TUNNEL_SIZE][MSG_LEN];
} __attribute__((aligned(8))) ChatTunnel;

ChatTunnel* AutumnAPI_Tunnel_Create(const char* name);
ChatTunnel* AutumnAPI_Tunnel_Connect(const char* name);
bool AutumnAPI_Tunnel_ReceiveFromFriend(ChatTunnel *tunnel, char *buffer);
bool AutumnAPI_Tunnel_Send_Msg(ChatTunnel *tunnel, const char *data);

#endif
