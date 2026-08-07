#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include "libatmchtn.h"

#define ARENA_SIZE (1024 * 1024)

typedef struct {
	uint8_t buffer[ARENA_SIZE] __attribute__((aligned(64)));
	size_t offset;
} __attribute__((aligned(64))) MemoryAr;

static MemoryAr global_a = { .offset = 0 };

void AutumnVM_Alloc(size_t size) {
	size = (size + 7) & ~7;
	if (global_a.offset + size > ARENA_SIZE) {
		global_a.offset = 0;
	}

	void *ptr = &global_a.buffer[global_a.offset];
	global_a.offset += size;
	return ptr;
}

typedef enum {
	TASK_Z_CP_PR = 0x01, // Zero copy parse,
	TASK_H_MATH = 0x02 // Heavy math process
} task_t;

typedef struct {
	uint8_t task_type;
	size_t data_len;
	uint8_t payload[MSG_LEN - 9];
} __attribute__((packed)) task_pack;

void* AutumnVM_Proc_Z_CpPayload(int memfd, size_t size) {
	void *mapped = mmap(NULL, size, PROT_READ, MAP_SHARED, memfd, 0);
    	if (mapped == MAP_FAILED) return;
	
	printf("%zu bytes data processed from RAM.\n", size);
	
	munmap(mapped, size);
}

void* avm(void *args) {
	ChatTunnel *tunnel = AutumnAPI_Tunnel_Connect("autumnvm");
	if (!tunnel) {
		tunnel = AutumnAPI_Tunnel_Create("autumnvm");
	}

	if (!tunnel) {
        	return NULL;
    	}

	char raw_msg[MSG_LEN];
	printf("Autumn Virtual Machine is running!\n");
	
	while (1) {
		if (AutumnAPI_Tunnel_ReceiveFromFriend(tunnel, raw_msg)) {
			task_pack *t = (task_pack*)raw_msg;

			void *a_ptr = AutumnVM_Alloc(t->data_len); // ...
			memcpy(a_ptr, t->payload, t->data_len);

			int memfd = memfd_create("avm", MFD_CLOEXEC | MFD_ALLOW_SEALING);
			if (memfd >= 0) {
                		ftruncate(memfd, t->data_len);
                		write(memfd, a_ptr, t->data_len); //write pointer to memfd
                		fcntl(memfd, F_ADD_SEALS, F_SEAL_WRITE | F_SEAL_SHRINK);

                		if (t->task_type == TASK_Z_CP_PR) {
                    			AutumnVM_Proc_Z_CpPayload(memfd, t->data_len);
                		}

                		close(memfd); 
            		}
		}
		else {
			usleep(2000); // Micro sleep ZZZ...
        	}
    	}
	return NULL;
}
