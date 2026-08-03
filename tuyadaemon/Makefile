include $(TOPDIR)/rules.mk
 
PKG_NAME:=tuyadaemon
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
 
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk
 
define Package/tuyadaemon
	DEPENDS:=+libubus +libubox +libblobmsg-json +tuya-iot-core-sdk +libuci
	CATEGORY:=custom
	TITLE:=tuyadaemon
endef

define Package/tuyadaemon/description
	Daemon for sending system information to the Tuya IoT Cloud
endef

define Build/Install
# Skips Build/Install, because
# Build/Install/Default tries to "make install" the program, which is undefined
endef

define Package/tuyadaemon/install	
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tuyadaemon $(1)/usr/bin/
	$(INSTALL_BIN) ./files/tuyadaemon.init $(1)/etc/init.d/tuyadaemon
endef
 
$(eval $(call BuildPackage,tuyadaemon))