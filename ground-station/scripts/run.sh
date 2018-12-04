#!/usr/bin/env bash

WLAN=wlx10feed2698ce
echo "Using interface $WLAN"
ifconfig $WLAN down
iw dev $WLAN set monitor otherbss fcsfail 
ifconfig $WLAN up 
iwconfig $WLAN channel 13 

cd /wifibroadcast
# TODO save these binaries
/wifibroadcast/sharedmem_init_rx 
export DISPLAY=:0
/wifibroadcast/rx -d 100 -p 0 -d 1 -b 24 -r 12 -f 768 $WLAN | mpv --fs -fps 200 -
