//FIXME: adapted to sysfs and used strcasestr

#include <stdio.h>
#include <string.h>
#include <dirent.h>

int sys_scanv4l2dev_sysfs(const char *target_name, char *out_dev_path, size_t max_len) {
    DIR *dir = opendir("/sys/class/video4linux");
    if (!dir) return -1;

    struct dirent *entry;
    int found = -1;

    while ((entry = readdir(dir)) != NULL) {
        
        if (entry->d_type == DT_LNK || entry->d_type == DT_DIR) {
            if (strncmp(entry->d_name, "video", 5) == 0) {
                char name_path[256];
                snprintf(name_path, sizeof(name_path), "/sys/class/video4linux/%s/name", entry->d_name);

                FILE *f = fopen(name_path, "r");
                if (f) {
                    char dev_name[128];
                    if (fgets(dev_name, sizeof(dev_name), f)) {
                        
                        dev_name[strcspn(dev_name, "\r\n")] = 0;

                        
                        if (strcasestr(dev_name, target_name) != NULL) {
                            snprintf(out_dev_path, max_len, "/dev/%s", entry->d_name);
                            found = 0;
                            fclose(f);
                            break;
                        }
                    }
                    fclose(f);
                }
            }
        }
    }

    closedir(dir);
    return found;
}
