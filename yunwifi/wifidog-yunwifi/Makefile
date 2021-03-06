#
# Copyright (C) 2006,2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wifidog-yunwifi
PKG_VERSION:=20130917-fixup-v6
PKG_RELEASE=1

#PKG_SOURCE_PROTO:=git
#PKG_SOURCE_URL:=git://github.com/wifidog/wifidog-gateway.git
#PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
#PKG_SOURCE_VERSION:=440445db60b0c3aff528ea703a828b0567293387
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/wifidog-yunwifi
  SUBMENU:=Captive Portals
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+iptables-mod-extra +iptables-mod-ipopt +iptables-mod-nat-extra +libpthread +yunwifi-bwcontrol +iptables-mod-hashlimit
  TITLE:=A wireless captive portal solution
  URL:=http://www.wifidog.org
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./source/* $(PKG_BUILD_DIR)/
endef

define Package/wifidog-yunwifi/description
	The Wifidog project is a complete and embeddable captive
	portal solution for wireless community groups or individuals
	who wish to open a free Hotspot while still preventing abuse
	of their Internet connection.
endef

define Package/wifidog-yunwifi/conffiles
/etc/wifidog.conf
endef

define Package/wifidog-yunwifi/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scripts/init.d/wifidog $(1)/usr/bin/wifidog-init
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifidog $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wdctl $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libhttpd.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/wifidog.conf $(1)/etc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/wifidog-msg.html $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/wifidog.init $(1)/etc/init.d/wifidog
endef

$(eval $(call BuildPackage,wifidog-yunwifi))
