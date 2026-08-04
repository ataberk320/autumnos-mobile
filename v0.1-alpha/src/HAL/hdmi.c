#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

void hal_hdmiact(void) {
	int fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC); //drm device
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
    //FIXME: repaired memory freeing.
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
		drmModeFreeResources(resources); //allocation
		close(fd);
		return;
	}

	for (int i = 0; i < connector->count_props; i++) { //device properties
        	drmModePropertyPtr prop = drmModeGetProperty(fd, connector->props[i]);
        	if (prop) {
            		if (strcmp(prop->name, "DPMS") == 0) { //hdmi status controller to write.
                		drmModeConnectorSetProperty(fd, connector_id, prop->prop_id, 0);
            		}
            		drmModeFreeProperty(prop); //free properties for memory allocation
        	}
    	}

    	drmModeFreeConnector(connector);
    	drmModeFreeResources(resources);
    	close(fd); // closing the descriptor
}

void hal_hdmicls(int drm_fd) {
    if (drm_fd >= 0) {
        close(drm_fd);
    }
}
