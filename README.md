# PiSwitch
GPIO interface to switch a Raspberry Pi off.

## Dependencies
sudo apt-get install build-essential git cmake

## Install bcm2835 library
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.44.tar.gz
tar -xvf bcm2835-1.44.tar.gz
cd bcm2835-1.44
./configure
make
sudo make check
sudo make install

## Install PiSwitch:
git clone https://github.com/axle-h/PiSwitch.git
cd PiSwitch
cmake .
make
sudo make install

