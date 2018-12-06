# Groundstation Receiver VM

This directory has everything needed to use Vagrant to create a VM that's ready to decode packets from a USB-connected wifi adapter that supports monitor mode. The groundstation has 3 resposibilities.

* Handles the Linux driver magic for monitor mode wifi
* Decodes and shows a local copy of the video for local viewing
* Displays helpful metrics about the signal strenth to assist in aiming the antenna
* Sends any received packets to the server specified in scripts/run.sh

## First time setup

```bash
# Install VirtualBox
brew cask install virtualbox virtualbox-extension-pack
# Install Vagrant
brew cask install vagrant
# Install the Vagrant VirtualBox guest-additions updater plugin
vagrant plugin install vagrant-vbguest
# Build the image
vagrant up
# Once it builts you usually need to reboot
vagrant halt && vagrant up
```

You can now log into the desktop environment using username:vagrant password:vagrant

## Developing

* `vagrant ssh` to open an ssh session into the VM.
* `vagrant halt` to shut down the instance.
* `vagrant destroy` to delete the instance and start over 
