# UDP Multi RX Station Receive Server

This directory has tools and information about running a server to receive all the UDP packets from all the groundstations. It has three responsibilities

* Receive raw monitor-mode packets from the ground stations over UDP
* Stitch the dual-fisheye video into equirectangular
* Make the video data available over RTMP (so that we can pull it from OBS)

## Setup

Run the docker for nginx-rtmp

    docker run -d -p 1935:1935 --name nginx-rtmp tiangolo/nginx-rtmp

Run the command to receive the UDP packets, stitch them, and send them to the RTMP server

	cd SpaceBalloon360/ez-wifibroadcast/wifibroadcast
	make rx sharedmem_init_rx
	./sharedmem_init_rx
	./rx -p 0 -d 100 -b 24 -r 12 -f 768 -s 0.0.0.0 -u 6000 | \
	ffmpeg -f h264 -r 10 -i - -vf dualfisheyetoer -vcodec h264 -b:v 3000k -strict experimental -f flv rtmp://localhost/live/payload360
