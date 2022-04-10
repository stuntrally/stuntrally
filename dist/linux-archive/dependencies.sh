#!/bin/bash -e

# Manual step: First enable Universe and Multiverse repos from Software Sources GUI app

JOBS=`nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1`
echo "Using $JOBS jobs for compilation"

sudo apt-get update
sudo apt-get install --assume-yes git cmake g++ libogre-1.8-dev libboost-wave-dev libboost-system-dev libboost-filesystem-dev libboost-thread-dev libogg-dev libvorbis-dev libenet-dev libxcursor-dev
sudo updatedb # For `locate` command required by make-archive.sh

# SDL
sudo apt-get install --assume-yes libasound2-dev libpulse-dev libaudio-dev libesd0-dev
cd /tmp
wget https://www.libsdl.org/release/SDL2-2.0.0.tar.gz
tar xvf SDL2-2.0.0.tar.gz
cd SDL2-2.0.0
./configure
make -j $JOBS
sudo make install
cd ..
rm -rf SDL2-2.0.0

# MyGUI
cd /tmp
wget https://downloads.sourceforge.net/project/my-gui/MyGUI/MyGUI_3.2.0/MyGUI_3.2.0.zip
unzip MyGUI_3.2.0.zip
cd MyGUI_3.2.0
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DMYGUI_BUILD_DEMOS=OFF -DMYGUI_BUILD_TOOLS=OFF
make -j $JOBS
sudo make install
cd ../..
rm -rf MyGUI_3.2.0

# SR sources
sudo ldconfig
cd ~
git clone --depth=0 git://github.com/stuntrally/stuntrally
# Tracks are disabled due to installing them runs out of disk space
#cd stuntrally/data
#git clone --depth=0 git://github.com/stuntrally/tracks

