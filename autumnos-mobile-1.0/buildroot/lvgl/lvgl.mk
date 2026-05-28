LVGL_VERSION = 8.4.0
LVGL_SOURCE = lvgl-$(LVGL_VERSION).tar.gz
LVGL_SITE = https://github.com/lvgl/lvgl/archive/refs/tags/v$(LVGL_VERSION)
LVGL_INSTALL_STAGING = YES

$(eval $(generic-package))LVGL_VERSION = 8.4.0

