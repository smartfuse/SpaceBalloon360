#!/usr/bin/env bash

#sudo sed -i -e 's/ main$/ main contrib non-free/g' /etc/apt/sources.list

sudo apt update
# Debugging stuff
sudo apt install -y net-tools build-essential libpcap-dev wireless-tools
# Crucial stuff
# TODO: Consider --no-install-recommends
sudo apt install -y mpv firmware-ath9k-htc linux-modules-extra-$(uname -r)
# Something in here made it work I think?
#linux-modules-extra-4.18.0-12-generic
#sudo apt-get install linux-generic linux-image-generic linux-headers-generic build-essential dkms

# Do we need virtualbox-guest-x11?

# TODO: consider pre-compiling these binaries?
cd /wifibroadcast
make rx
make sharedmem_init_rx

# sudo DEBIAN_FRONTEND=noninteractive apt-get -y install xubuntu-core^
# sudo DEBIAN_FRONTEND=noninteractive apt-get -y install virtualbox-guest-dkms virtualbox-guest-utils virtualbox-guest-x11
# sudo VBoxClient --checkhostversion
# sudo VBoxClient --clipboard
# sudo VBoxClient --display
# sudo VBoxClient --draganddrop
# sudo VBoxClient --seamless
# echo "$(whoami):$(whoami)" | sudo chpasswd

sudo apt install -y lubuntu-core
