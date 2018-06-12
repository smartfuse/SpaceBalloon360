#!bin/bash

echo "Fetching changes from repo"

make clean
git pull
cd wifibroadcast
make
cd ..
rm -rf ~/wifibroadcast
ln -s `pwd`/wifiboadcast ~/wifibroadcast
rm -rf ~/.dotprofile
cp -R dotprofile ~/.profile

if [ $1 == 'transmit' ] || [ $1 == 'receive' ]; then
	echo $1
	rm -rf /boot/wifibroadcast-1.txt
	cp /boot/wifibroadcast-1.txt
fi

if [ $1 == "receive_udp" ]; then
	echo $1
	rm -rf /boot/wifibroadcast-udp.txt
	cp /boot/wifibroadcast-1.txt
fi

echo "Done"
