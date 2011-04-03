About this directory
====================
In this dir you can find various scripts, launchers and other helper files that are needed for packaging and distributing Stunt Rally.

Below is a short description of them:

* debian/*          - files needed for a debian package, see below "Debian packaging"
* makeinstaller.py  - script for createing Windows installer, see below "Windows installer"
* *.desktop         - menu launchers for Linux (freedesktop.org-compliant) systems
* stuntrally.png    - application icon
* CMakeLists.txt    - CMake install targets for files here
* Readme.txt        - this file


Debian/Ubuntu packaging
-----------------------

You need some Debian tools, at least packages dput and dpkg-dev.

Get the "original" tarball from Launchpad, and extract it. Don't remove the archive. Copy the dist/debian directory to the extracted sources and edit debian/changelog to contain info about the new version. Version number and Ubuntu/Debian version are important. After that, build and upload the source package (from the extracted sources):

  dpkg-buildpackage -sd -S
  cd ..
  dput ppa:stuntrally-team/stable stuntrally_VERSION_source.changes


Windows installer
-----------------

To create an installer, you need nsis (http://nsis.sourceforge.net/) and Python.

Just run the accompanying makeinstaller.py script, giving it the path to a clean StuntRally prefix as a command line argument. The installer will be created to the current working directory.

Designed for Linux, but should work on Windows too.

