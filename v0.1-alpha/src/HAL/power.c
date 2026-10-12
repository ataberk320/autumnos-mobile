
#define _DEFAULT_SOURCE

#include <sys/reboot.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include "../../include/libatmchtn.h"

#define POWER_ROOT "/sys/class/power_supply"
#define REQUEST_TUNNEL "power_req"
#define RESPONSE_TUNNEL "power_resp"

int hal_reboot(void)
{
    sync();
    return reboot(RB_AUTOBOOT) == 0 ? 0 : -errno;
}

int hal_pwroff(void)
{
    sync();
    return reboot(RB_POWER_OFF) == 0 ? 0 : -errno;
}

int hal_getcpc(void)
{
    DIR *dir = opendir(POWER_ROOT);
    if (!dir)
        return -1;

    struct dirent *entry;
    char path[256];
    int capacity = -1;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;

        int n = snprintf(path, sizeof(path),
                         "%s/%s/capacity",
                         POWER_ROOT, entry->d_name);

        if (n < 0 || (size_t)n >= sizeof(path))
            continue;

        FILE *fp = fopen(path, "r");
        if (!fp)
            continue;

        int value;
        int ok = fscanf(fp, "%d", &value) == 1;
        fclose(fp);

        if (ok && value >= 0 && value <= 100) {
            capacity = value;
            break;
        }
    }

    closedir(dir);
    return capacity;
}

int main(void)
{
    ChatTunnel *request =
        AutumnAPI_Tunnel_Connect(REQUEST_TUNNEL);
    ChatTunnel *response =
        AutumnAPI_Tunnel_Connect(RESPONSE_TUNNEL);

    if (!request || !response)
        return 1;

    char command[MSG_LEN];
    char result[MSG_LEN];

    for (;;) {
        if (!AutumnAPI_Tunnel_ReceiveFromFriend(request, command)) {
            usleep(1000);
            continue;
        }

        if (strcmp(command, "GET_CAPACITY") == 0) {
            snprintf(result, sizeof(result), "%d", hal_getcpc());
            AutumnAPI_Tunnel_Send_Msg(response, result);
        } else if (strcmp(command, "REBOOT") == 0) {
            snprintf(result, sizeof(result), "%d", hal_reboot());
            AutumnAPI_Tunnel_Send_Msg(response, result);
        } else if (strcmp(command, "POWEROFF") == 0) {
            snprintf(result, sizeof(result), "%d", hal_pwroff());
            AutumnAPI_Tunnel_Send_Msg(response, result);
        } else {
            AutumnAPI_Tunnel_Send_Msg(response, "UNKNOWN_COMMAND");
        }
    }

    return 0;
}
