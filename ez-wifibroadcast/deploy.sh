#!/bin/bash

echo "Fetching changes from repo"

git pull
cd wifibroadcast
make clean
make
cd ..
rm -rf ~/wifibroadcast
ln -s `pwd`/wifibroadcast ~/wifibroadcast
chmod -R 777 ~/wifibroadcast
rm -rf ~/.dotprofile
cp -R dotprofile ~/.profile

if [ "$1" == "transmit" ] || [ "$1" == "receive" ]; then
	echo $1
	rm -rf /boot/wifibroadcast-1.txt
	cp boot/wifibroadcast-1.txt /boot/wifibroadcast-1.txt
fi

if [ "$1" == "receive_udp" ]; then
	echo $1
	rm -rf /boot/wifibroadcast-1.txt
	cp boot/wifibroadcast-udp.txt /boot/wifibroadcast-1.txt
fi

echo "Done"
