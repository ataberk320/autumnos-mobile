################################################################################
#
# sunxi-g2d
#
################################################################################

SUNXI_G2D_VERSION = 1.0
SUNXI_G2D_SITE = $(TOPDIR)/package/sunxi-g2d/src
SUNXI_G2D_SITE_METHOD = local

$(eval $(kernel-module))

define SUNXI_G2D_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 0644 $(@D)/g2d_driver.h $(STAGING_DIR)/usr/include/g2d_driver.h
endef

$(eval $(generic-package))
