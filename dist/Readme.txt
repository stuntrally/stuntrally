About this directory
====================
In this directory you can find various scripts, launchers and other helper files that are needed for packaging and distributing Stunt Rally.

Below is a short description of them:

* create-clean-sources.sh   - for aiding in creation of source packages from git, see below "Clean sources"
* debian/*                  - files needed for a debian package, see below "Debian/Ubuntu packaging"
* ppastats.py               - prints statistics about the Launchpad PPA
* makeinstaller.py          - script for creating Windows installer, see below "Windows installer"
* *.desktop                 - menu launchers for Linux (freedesktop.org-compliant) systems
* stuntrally.png            - application icon
* CMakeLists.txt            - CMake install targets for the files here
* make_roadstats_xml.py     - utility script that generates roadstats.xml used in online track browser


Clean sources
-------------

Since the development environment usually contains extra files like build directory, logs and work-in-progress material, it is not exactly a clean environment for creating distributable source packages. Add the presence of version control stuff and possibly fast evolving repo history, it is a tedious task to create source tarballs.

The script create-clean-sources.sh comes to rescue: it clones the repo to a temporary location, optionally checks out a specific tag (release), removes all clutter from version control system and finally (optionally) puts it all into a compressed archive. This is also useful and used for the Ubuntu Launchpad uploading.

Note that --tag option won't work with version 1.1.1 and below due to repository rearrangements.


Debian/Ubuntu packaging
-----------------------

This is mainly designed for uploading source packages to Launchpad PPA for automatic building.

You need some Debian tools, at least packages "dput" and "dpkg-dev". Also, you need a GPG key for signing the package (Launchpad requires signing).

1. Either get the "original" tarball (stuntrally_XX-0.orig.tar.bz2) from Launchpad or create a new one with the create-clean-sources.sh. Note that the name is important.
2. Extract the archive, but don't delete it.
3. If using Launchpad's orig tarball, copy all changes (from clean sources) on top of the extracted sources.
4. Copy the dist/debian directory to the extracted sources.
5. Edit debian/changelog to contain info about the new version. Version number and distribution version are important. Suffix the version with the distribution (e.g. ~natty).
6. Type in commands:

  dpkg-buildpackage -sa -S    # Only if you created the orig archive yourself
  dpkg-buildpackage -sd -S    # Only if you used Launchpad's orig archive
  cd ..
  dput ppa:stuntrally-team/stable stuntrally_VERSION_source.changes     # For releases
  dput ppa:stuntrally-team/testing stuntrally_VERSION_source.changes    # For test versions


Windows installer
-----------------

To create an installer, you need nsis (http://nsis.sourceforge.net/) and Python. You need the "makensis" program (from nsis) in your PATH or alternatively you can set MAKENSIS environment variable to point to that program.

Run the accompanying makeinstaller.py script, giving it the path to a clean StuntRally prefix (i.e. the directory containing all the files that you want to put to the installer - ideally the files from CMake install target, "make install") as a command line argument. The installer will be created to the current working directory.

This works on both Windows and Linux (however, the installer will always target Windows).

