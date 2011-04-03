About this directory
====================
In this directory you can find various scripts, launchers and other helper files that are needed for packaging and distributing Stunt Rally.

Below is a short description of them:

* debian/*          - files needed for a debian package, see below "Debian/Ubuntu packaging"
* ppastats.py       - prints statistics about the Launchpad PPA
* makeinstaller.py  - script for creating Windows installer, see below "Windows installer"
* *.desktop         - menu launchers for Linux (freedesktop.org-compliant) systems
* stuntrally.png    - application icon
* CMakeLists.txt    - CMake install targets for files here
* Readme.txt        - this file


Debian/Ubuntu packaging
-----------------------

This is mainly designed for uploading source packages to Launchpad PPA for automatic building.

You need some Debian tools, at least packages dput and dpkg-dev. Also, you need a GPG key for signing the package (Launchpad requires signing).

Get the "original" tarball (stuntrally_XX.orig.tar.gz) from Launchpad, and extract it. Don't remove the archive. Copy the dist/debian directory to the extracted sources and edit debian/changelog to contain info about the new version. Version number and distribution version are important. Suffix the version with the distribution (e.g. ~natty). After that, build and upload the source package (from the extracted sources):

  dpkg-buildpackage -sd -S
  cd ..
  dput ppa:stuntrally-team/stable stuntrally_VERSION_source.changes


Windows installer
-----------------

To create an installer, you need nsis (http://nsis.sourceforge.net/) and Python. You need the "makensis" program (from nsis) in your PATH or alternatively you can set MAKENSIS environment variable to point to that program.

Run the accompanying makeinstaller.py script, giving it the path to a clean StuntRally prefix (i.e. the directory containing all the files that you want to put to the installer - ideally the files from CMake install target, "make install") as a command line argument. The installer will be created to the current working directory.

This works on both Windows and Linux.

