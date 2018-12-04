#!/usr/bin/env bash

WLAN=$(iw dev | awk '$1=="Interface"{print $2}')
echo "Using interface $WLAN"

# Put the adapter into monitor mode
ifconfig $WLAN down
iw dev $WLAN set monitor otherbss fcsfail 
ifconfig $WLAN up 
iwconfig $WLAN channel 13 

# Fire up wifibroadcast!
cd /wifibroadcast
/wifibroadcast/sharedmem_init_rx 
export DISPLAY=:0
/wifibroadcast/rx -d 100 -p 0 -d 1 -b 24 -r 12 -f 768 $WLAN | mpv --fs -fps 200 --input-ipc-server=/tmp/mpvsocket -
