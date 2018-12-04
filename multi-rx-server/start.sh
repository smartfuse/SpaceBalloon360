#!/usr/bin/env bash
rx -d 100 -p 0 -d 100 -b 24 -r 12 -f 768 -s 0.0.0.0 -u 6000 | \
ffmpeg -f h264 -r 10 -i - -vcodec copy -strict experimental -f flv rtmp://space.prscp.net/live/payload360
