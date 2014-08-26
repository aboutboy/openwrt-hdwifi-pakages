#!/bin/sh
while true
do
	[ ! -e /tmp/dog-startat ] && cat /proc/uptime |awk '{printf("%d\n",$1)}' >/tmp/dog-startat
	[ $(ps |grep wifidog |wc -l) -lt 2 ] &&
	{
		echo "No dog !start it"
		wifidog
		cat /proc/uptime |awk '{printf("%d\n",$1)}' >/tmp/dog-startat
	}
	dogstart=$(cat /tmp/dog-startat)
	dognow=$(cat /proc/uptime |awk '{printf("%d\n",$1)}')
	[ $(($dognow - $dogstart)) -ge 86400 ] && 
	{
		wdctl restart
		[ "$?" = "0" ] && cat /proc/uptime |awk '{printf("%d\n",$1)}' >/tmp/dog-startat
	}
	sleep 300
done
