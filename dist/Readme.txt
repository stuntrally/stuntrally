About this directory
====================
In this directory you can find various scripts, launchers and other helper files that are needed for packaging and distributing Stunt Rally.

Below is a short description of them:

* linux-archive/*           - Scripts for creating a Linux binary release
* installer.nsi             - NSIS Windows installer script
* *.desktop                 - menu launchers for Linux (freedesktop.org-compliant) systems
* CMakeLists.txt            - CMake install targets for the files here
* make_roadstats_xml.py     - utility script that generates roadstats.xml used in online track browser

To update roadstats: run the python script and copy generated roadstats.xml to website's tracks/ dir. Also copy tracks.xml form config/ into there.


If someone ever decides to start maintaining Launchpad PPA or Debian package, look into the history of this dist/ directory.

