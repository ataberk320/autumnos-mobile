#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "../../include/HAL/HAL_Symb.h"

void hal_hdmiact(void) {
	int fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		return;
	}

	drmModeRes *resources = drmModeGetResources(fd);
	if (!resources) {
		close(fd);
		return;
	}

	uint32_t connector_id = 0;
	drmModeConnector *connector = NULL;

	for (int i = 0; i < resources->count_connectors; i++) {
            drmModeConnector *temp_conn = drmModeGetConnector(fd, resources->connectors[i]);
            if (temp_conn) {
                if (temp_conn->connection == DRM_MODE_CONNECTED) {
                    connector_id = temp_conn->connector_id;
                    connector = temp_conn;
                    break;
                }
                drmModeFreeConnector(temp_conn);
            }
        }

	if (!connector_id || !connector) {
		drmModeFreeResources(resources);
		close(fd);
		return;
	}

	for (int i = 0; i < connector->count_props; i++) {
        	drmModePropertyPtr prop = drmModeGetProperty(fd, connector->props[i]);
        	if (prop) {
            		if (strcmp(prop->name, "DPMS") == 0) {
                		drmModeConnectorSetProperty(fd, connector_id, prop->prop_id, 0);
            		}
            		drmModeFreeProperty(prop);
        	}
    	}

    	drmModeFreeConnector(connector);
    	drmModeFreeResources(resources);
    	close(fd);
}

void hal_hdmicls(int drm_fd) {
    if (drm_fd >= 0) {
        close(drm_fd);
    }
}

typedef struct {
    hal_dev_t base;
    void (*Activate)(void);
    void (*Close)(int drm_fd);
} hdmi_dev_t;

static int hdmi_bind_dev(const struct hal_mod_t* module, const char* name, struct hal_dev_t** device) {
    hdmi_dev_t* dev = malloc(sizeof(hdmi_dev_t));
    if (!dev) return -1;
    
    dev->base.common = (hal_mod_t*)module;
    dev->Activate = hal_hdmiact;
    dev->Close = hal_hdmicls;
    
    *device = (hal_dev_t*)dev;
    return 0;
}

hal_mod_t HAL_MODULE_INFO_SYM = {
    .id = "hdmi",
    .version = 1,
    .open = hdmi_bind_dev
};
