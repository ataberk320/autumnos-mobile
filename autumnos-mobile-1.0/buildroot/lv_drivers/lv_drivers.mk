LV_DRIVERS_VERSION = v8.3.0
LV_DRIVERS_SITE = $(call github,lvgl,lv_drivers,$(LV_DRIVERS_VERSION))
LV_DRIVERS_INSTALL_STAGING = YES

$(eval $(generic-package))
