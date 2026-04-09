################################################################################
#
# sunxi-g2d
#
################################################################################

SUNXI_G2D_VERSION = master
SUNXI_G2D_SITE = https://github.com/cheofusi/sunxi-g2d.git
SUNXI_G2D_SITE_METHOD = git
SUNXI_G2D_INSTALL_STAGING = YES

define SUNXI_G2D_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)
endef

define SUNXI_G2D_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 0755 $(@D)/libg2d.so $(STAGING_DIR)/usr/lib/libg2d.so
	$(INSTALL) -D -m 0644 $(@D)/g2d_driver.h $(STAGING_DIR)/usr/include/g2d_driver.h
endef

define SUNXI_G2D_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/libg2d.so $(TARGET_DIR)/usr/lib/libg2d.so
endef

$(eval $(generic-package))
