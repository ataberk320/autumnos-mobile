#define _DEFAULT_SOURCE
#include <stdio.h>
#include <dirent.h>

#define POWER_ROOT "/sys/class/power_supply"

int _AutumnSys_getbatcpc(void) {
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
