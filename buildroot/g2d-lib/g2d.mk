################################################################################
#
# sunxi-g2d
#
################################################################################

SUNXI_G2D_VERSION = main
SUNXI_G2D_SITE = https://github.com/cheofusi/sunxi-g2d.git
SUNXI_G2D_SITE_METHOD = git
SUNXI_G2D_INSTALL_STAGING = YES


SUNXI_G2D_SUBDIR = module

define SUNXI_G2D_BUILD_CMDS
	cd $(@D)/$(SUNXI_G2D_SUBDIR) && \
	$(TARGET_CC) $(TARGET_CFLAGS) -fPIC -shared \
		$$(ls *.c) -o ../libg2d.so -I.
endef

define SUNXI_G2D_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 0755 $(@D)/libg2d.so $(STAGING_DIR)/usr/lib/libg2d.so
	$(INSTALL) -D -m 0644 $(@D)/$(SUNXI_G2D_SUBDIR)/g2d_driver.h $(STAGING_DIR)/usr/include/g2d_driver.h
endef

define SUNXI_G2D_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/libg2d.so $(TARGET_DIR)/usr/lib/libg2d.so
endef

$(eval $(generic-package))
