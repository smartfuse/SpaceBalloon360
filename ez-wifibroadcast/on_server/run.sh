#!/usr/bin/env bash
export DISPLAY=:1
~/SpaceBalloon360/ez-wifibroadcast/wifibroadcast/sharedmem_init_rx
~/SpaceBalloon360/ez-wifibroadcast/wifibroadcast/rx -p 0 -d 10 -b 24 -r 12 -f 768 -s 0.0.0.0 -u 6000 | ~/FFmpeg/ffmpeg -f h264 -r 10 -i - -vf dualfisheyetoer -vcodec h264 -b:v 3000k -strict experimental -f flv rtmp://localhost/live/payload360
