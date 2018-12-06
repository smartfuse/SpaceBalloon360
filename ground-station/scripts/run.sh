#!/usr/bin/env bash
#
# Start the groundstation. Must be run as root/sudo
#

# Multi-rx-server info
SERVER_HOST=space.johnboiles.com
SERVER_IP=$(dig +short $SERVER_HOST)
SERVER_PORT=6000
echo "Connecting to $SERVER_IP on port $SERVER_PORT"

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
$WIFIBROADCAST_PATH/rx -p 0 -d 1 -b 24 -r 12 -f 768 -s $SERVER_IP -n $SERVER_PORT $WLAN | mpv --fs -fps 200 --input-ipc-server=/tmp/mpvsocket - &
sleep 10
$WIFIBROADCAST_PATH/onscreen_display_server | socat - /tmp/mpvsocket
