#!/bin/sh
. /etc/openwrt_release
. /usr/share/libubox/jshn.sh
. /lib/hdwifi.sh
local board=$(hdwifi_get_board)
local yunwifi_str
local domain=$(uci get yunwifi.config.hostname)

set_yunwifi_str() {
	[ -e /tmp/lock/yunwifi_str.lck ] && {
		echo "has run!"
		return 1
	}
	touch /tmp/lock/yunwifi_str.lck
	local url="http://${domain}/yunwifi/wifi/getyunwifistr.action"
	wget -qO /tmp/yunwifi_str.txt $url
	while [ "$?" != "0" ]
	do
		sleep 5
		wget -qO /tmp/yunwifi_str.txt $url
	done
	local str=$(cat /tmp/yunwifi_str.txt)
	yunwifi_str=$(cat /tmp/yunwifi_str.txt)
	local mac=$(hdwifi_get_mac)
	url="http://${domain}/yunwifi/wifi/confirmyunwifistr.action?gw_id=${str}&mac=${mac}&aptype="
	url=${url}$(hdwifi_get_board)
	hdwifi_set_str $yunwifi_str
	[ "$?" = "0" ] && {
		wget -qO /dev/null $url
		while [ "$?" != "0" ]
		do
			sleep 5
			wget -qO /dev/null $url
		done
	}
	rm /tmp/lock/yunwifi_str.lck
}
get_yunwifi_str() {

	yunwifi_str=$(hdwifi_get_str)
	[ "$?" != "0" ] && {
		echo "not set yunwifi str get from server"
		set_yunwifi_str
	}
}

get_conf(){
        [ "$1" = "" ] && {
                echo "not give host!"
                exit 1
        }
		get_yunwifi_str
        uri="/yunwifi/wifi/getconf.action"
        host="http://${1}"
        url="${host}${uri}?aptype=${board}&gw_id=${yunwifi_str}&currentversion=${DISTRIB_VERSION}"
        wget -qO /tmp/wifidog.conf $url
        while [ "$?" != "0" ]
		do
            sleep 5
			wget -qO /tmp/wifidog.conf $url
        done
        sed -i 's/\r//' /tmp/wifidog.conf
        newdog_md5=$(md5sum /tmp/wifidog.conf |awk '{print $1}')
        olddog_md5=$(md5sum /etc/wifidog.conf |awk '{print $1}')
		is_wifidog_conf=$(cat /tmp/wifidog.conf |grep GatewayID)
        [ "$is_wifidog_conf" != "" -a "$newdog_md5" != "" -a "$olddog_md5" != "" -a "$newdog_md5" = "$olddog_md5" ] && {
                logger "YUNWIFI:remote wifidog conf same ! do nothing!!"
				[ -z "$(pidof wifidog)" ] &&
				{
					logger "No dog !start it"
					wifidog
				}
                exit 0
        }
        cp /tmp/wifidog.conf /etc/wifidog.conf
        wdctl restart
        [ "$?" != "0" ] && wifidog

        exit 0
}
if [ "$1" != "" ]
then
	get_conf $1
else
	local host=$(uci get yunwifi.config.hostname)
	get_conf $host
fi
