#!/usr/bin/env bash
#
# Start the groundstation. Must be run as root/sudo
#

# Multi-rx-server info
SERVER_HOST=35.161.202.130
SERVER_PORT=6000

WLAN=$(iw dev | awk '$1=="Interface"{print $2}')
echo "Using interface $WLAN"

# Put the adapter into monitor mode
ifconfig $WLAN down
iw dev $WLAN set monitor otherbss fcsfail 
ifconfig $WLAN up 
iwconfig $WLAN channel 13 

# Fire up wifibroadcast!
WIFIBROADCAST_PATH=/SpaceBalloon360/ez-wifibroadcast/wifibroadcast
$WIFIBROADCAST_PATH/sharedmem_init_rx
export DISPLAY=:0
$WIFIBROADCAST_PATH/SpaceBalloon360/ez-wifibroadcast/wifibroadcast/rx -p 0 -d 1 -b 24 -r 12 -f 768 -s $SERVER_HOST -n $SERVER_HOST $WLAN | mpv --fs -fps 200 --input-ipc-server=/tmp/mpvsocket -
$WIFIBROADCAST_PATH/SpaceBalloon360/ez-wifibroadcast/wifibroadcast/onscreen_display_server | socat - /tmp/mpvsocket
