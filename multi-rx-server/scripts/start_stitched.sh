#!/usr/bin/env bash
rx -p 0 -d 100 -b 24 -r 12 -f 768 -s 0.0.0.0 -u 6000 | ffmpeg -f h264 -i - -i /home/xmap_insta360.pgm -i /home/ymap_insta360.pgm -filter_complex remap -vcodec h264 -b:v 3000k -strict experimental -f flv rtmp://space.prscp.net/live/payload360
