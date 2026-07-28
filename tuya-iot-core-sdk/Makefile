include $(TOPDIR)/rules.mk

PKG_NAME:=tuya-iot-core-sdk
PKG_RELEASE:=1
PKG_VERSION:=1.0.0
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/tuya/tuya-iot-core-sdk
PKG_SOURCE_VERSION:=0208badefb2003d30e70b5b875395e3187c850d9

PKG_BUILD_DIR:=$(BUILD_DIR)/tuya-iot-core-sdk-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_BINARY_DIR:=$(PKG_BUILD_DIR)/build

define Package/tuya-iot-core-sdk
	CATEGORY:=Libraries
	TITLE:=tuya-iot-core-sdk
endef

# Build/Configure calls cmake

# Build/Compile calls make in the build directory
CMAKE_BINARY_SUBDIR:=build

define Build/Install
# Skips Build/Install, because
# Build/Install/Default tries to "make install", which is an undefined target
endef

# Copy library files to /usr/lib and header files to /usr/include
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/lib/libutils_modules.so  $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/lib/libplatform_port.so  $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/lib/libmiddleware_implementation.so  $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/lib/liblink_core.so  $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/*.h $(1)/usr/include/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/interface/*.h $(1)/usr/include/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/middleware/*.h $(1)/usr/include/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/utils/*.h $(1)/usr/include/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/examples/custom_protocol_basic_demo/*.h $(1)/usr/include/
endef

define Package/tuya-iot-core-sdk/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/lib/libutils_modules.so  $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/lib/libplatform_port.so  $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/lib/libmiddleware_implementation.so  $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/lib/liblink_core.so  $(1)/usr/lib/
endef

$(eval $(call BuildPackage,tuya-iot-core-sdk))         
