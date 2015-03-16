1. Build Ogre 2.0

mkdir build
cd build
cmake ..
make

2. Build MyGUI

To find Ogre without installing it:
 
cmake .. -DOGRE_HOME=/home/scrawl/Dev/ogre  -DOGRE_BUILD=/home/scrawl/Dev/ogre/build -DOGRE_SOURCE_DIR=/home/scrawl/Dev/ogre/ -DOGRE_SOURCE=/home/scrawl/Dev/ogre 
```

3. Install MyGUI to a local prefix

cmake .. -DCMAKE_INSTALL_PREFIX=/opt/mygui-ogre2

3. Build SR

To find OGRE and MyGUI:

export PKG_CONFIG_PATH=/opt/mygui2/lib/pkgconfig
cmake .. -DOGRE_HOME=/home/scrawl/Dev/ogre  -DOGRE_BUILD=/home/scrawl/Dev/ogre/build -DOGRE_SOURCE_DIR=/home/scrawl/Dev/ogre/ -DOGRE_SOURCE=/home/scrawl/Dev/ogre 

In CMake, the INCLUDE_DIR for Ogre components will not be set correctly and must be adjusted manually, e.g. to
/home/scrawl/Dev/ogre/Components/Overlay/include
