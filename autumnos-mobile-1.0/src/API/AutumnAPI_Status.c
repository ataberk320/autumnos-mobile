#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <stdbool.h>
#include <dlfcn.h>
long AutumnAPI_Status_GetUptime(void) {
        struct  sysinfo s_info;
        int error = sysinfo(&s_info);
        if (error !=0) return -1;
        return s_info.uptime;
}

long AutumnAPI_Status_Get_Free_RAM(void) {
        struct sysinfo s_info;
        sysinfo(&s_info);
        return (s_info.freeram * s_info.mem_unit) / (1024 * 1024);
}

long AutumnAPI_Status_Get_Used_RAM(void) {
        struct sysinfo s_info;
        if (sysinfo(&s_info) != 0) {
                return -1;
        }
        unsigned long total_mb = (s_info.totalram * s_info.mem_unit) / (1024 * 1024);
        unsigned long free_mb = (s_info.freeram * s_info.mem_unit) / (1024 * 1024);
        unsigned long used_mb = total_mb - free_mb;
        return (long)used_mb;
}

void AutumnAPI_Status_WiFi(bool enable) {
	void *handle = dlopen("/usr/lib/atmhal.so", RTLD_LAZY);
	if (handle) {
		typedef void (*wifi_func)(bool);
		wifi_func func = (wifi_func)dlsym(handle, "atmsys_wifi");
		if (func) func(enable);
		dlclose(handle);
	}
}

void AutumnAPI_Status_Bluetooth(bool enable) {
	void *handle = dlopen("/usr/lib/atmhal.so", RTLD_LAZY);
        if (handle) {
                typedef void (*wifi_func)(bool);
                wifi_func func = (wifi_func)dlsym(handle, "atmsys_bt");
                if (func) func(enable);
                dlclose(handle);
        }
}

void AutumnAPI_Status_Flight(bool enable) {
	void *handle = dlopen("/usr/lib/atmhal.so", RTLD_LAZY);
        if (handle) {
                typedef void (*wifi_func)(bool);
                wifi_func func = (wifi_func)dlsym(handle, "atmsys_flight");
                if (func) func(enable);
                dlclose(handle);
        }
}
