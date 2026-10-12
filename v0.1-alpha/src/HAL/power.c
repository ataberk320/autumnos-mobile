#include <sys/reboot.h>
#include <unistd.h>
#include <errno.h>

//converted to system service for prevent spaghetti code.

void hal_reboot(void) {
        sync();
        return reboot(RB_AUTOBOOT) == 0 ? 0 : errno;
}

void hal_pwroff(void) {
        sync();
        return reboot(RB_POWER_OFF) == 0 ? 0 : errno;
}

int hal_getcpc(void)
{
    DIR *dir = opendir(POWER_ROOT);
    if (!dir)
        return -1;

    struct dirent *entry;
    char path[256];
    int capacity = -1;

    while ((entry = readdir(dir))) {
        if (entry->d_name[0] == '.')
            continue;

        snprintf(path, sizeof(path), "%s/%s/capacity",
                 POWER_ROOT, entry->d_name);

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
