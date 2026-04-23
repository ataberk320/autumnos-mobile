AUTUMNSDK_VERSION = main
AUTUMNSDK_SITE = https://github.com/ataberk320/AutumnOS-SDK-Tools
AUTUMNSDK_SITE_METHOD = git

ifeq ($(BR2_PACKAGE_AUTUMN_API_DEV),y)
define AUTUMNSDK_INSTALL_API
	$(INSTALL) -D -m 0644 $(@D)/AutumnAPI/AutumnAPI.h $(TARGET_DIR)/usr/include/api.h
	$(INSTALL) -D -m 0644 $(@D)/AutumnAPI/AutumnAPI.c $(TARGET_DIR)/usr/src/api.c
endef
endif

ifeq ($(BR2_PACKAGE_AUTUMN_PKG_CNV),y)
define AUTUMNSDK_INSTALL_CONVERTER
	$(INSTALL) -D -m 0755 $(@D)/autumn-cnv/autumn-cnv.py $(TARGET_DIR)/usr/bin/autumn-cnv
endef
endif

ifeq ($(BR2_PACKAGE_AUTUMN_PKG_UNPK),y)
define AUTUMNSDK_INSTALL_UNPACKAGER
	$(INSTALL) -D -m 0755 $(@D)/autumn-unpk/autumn-unpk.py $(TARGET_DIR)/usr/bin/autumn-unpk
endef
endif

define AUTUMNSDK_INSTALL_TARGET_CMDS
	$(AUTUMNSDK_INSTALL_API)
	$(AUTUMNSDK_INSTALL_CONVERTER)
	$(AUTUMNSDK_INSTALL_UNPACKAGER)
endef

$(eval $(generic-package))
