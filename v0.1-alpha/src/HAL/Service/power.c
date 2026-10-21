#define _DEFAULT_SOURCE

#include <sys/reboot.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../../../include/libatmchtn.h"

#define REQUEST_TUNNEL "action_req"
#define RESPONSE_TUNNEL "action_resp"

extern int _AutumnSys_getbatcpc(void);

int _AutumnSys_reboot(void)
{
    sync();
    return reboot(RB_AUTOBOOT) == 0 ? 0 : -errno;
}

int _AutumnSys_pwroff(void)
{
    sync();
    return reboot(RB_POWER_OFF) == 0 ? 0 : -errno;
}

void daemonize_actionmgrd(void) {
    pid_t pid = fork();
    
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    if (setsid() < 0) exit(EXIT_FAILURE);

    chdir("/");
    umask(0);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int main(void) {
	daemonize_actionmgrd();

	ChatTunnel *request = AutumnAPI_Tunnel_Connect(REQUEST_TUNNEL);
    	ChatTunnel *response = AutumnAPI_Tunnel_Connect(RESPONSE_TUNNEL);

	if (!request || !response) return 1;

	char command[256];
	char result[256];

	for (;;) {
		if (!AutumnAPI_Tunnel_ReceiveFromFriend(request, command)) {
            		usleep(10000);
            		continue;
        	}

		if (strcmp(command, "GETCPC") == 0) {
			int cap = _AutumnSys_getbatcpc();
			snprintf(result, sizeof(result), "%d", cap);
			AutumnAPI_Tunnel_Send_Msg(response, result);
		}
		else if (strcmp(command, "RBT") == 0) {
			snprintf(result, sizeof(result), "%d", _AutumnSys_reboot());
            		AutumnAPI_Tunnel_Send_Msg(response, result);
		}
		else if (strcmp(command, "POWEROFF") == 0) {
            		snprintf(result, sizeof(result), "%d", _AutumnSys_pwroff());
            		AutumnAPI_Tunnel_Send_Msg(response, result);
        	}
		else {
			AutumnAPI_Tunnel_Send_Msg(response, "UNKNOWN_COMMAND");
		}
	}

	return 0;
}
