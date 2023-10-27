## Introduction

This page is about how to install Stunt Rally and where to get it
from.  
The dev team itself maintains **Windows installer** and before
maintained **GNU/Linux** binaries.  
Latest are located on the official **[Releases
page](https://github.com/stuntrally/stuntrally/releases)** (on
github).  
Older are in Archive [downloads
page](https://sourceforge.net/projects/stuntrally/files/) (on
SourceForge).

See details below for info how to use them.  

## Windows

Official installer is located in our [Releases
page](https://github.com/stuntrally/stuntrally/releases).

It is an executable (.exe) that will install game (and editor) in chosen
destination.  
After installation shortcuts will appear in start menu folder.  
Tested on Windows 10, 64-bit.  
Version 2.2.1 was the last one to run on Windows XP.

## GNU/Linux

## Flatpak

Flatpak build is most recent for GNU/Linux and supported on many
distros.  
How to set up flatpak for your distro
[here](https://flatpak.org/setup/).  
And how to install StuntRally with it
[here](https://flathub.org/apps/details/org.tuxfamily.StuntRally).  

## Old archive

Official Archive with old precompiled binaries (32 and 64 bit) is
located in our
[downloads](https://sourceforge.net/projects/stuntrally/files/).

It is an .xz archive that needs to be unpacked (anywhere).  
Then start the game or editor running the included links (shell
scripts).  

It likely doesn't work now, but it could be fixed by removing
libstdc++.so.6 (in lib/64 or lib/32). Info [on
forum](https://forum.freegamedev.net/viewtopic.php?f=78&t=7980).  
If this doesn't work, start in console to see any errors. Some libraries
may be missing, etc.

## Packages

There might also be individual packages for your distribution (check the
sections below).  
But most of them are maintained by third-party individuals and could be
not up-to-date or not available for your distro.  
Problems with the actual packages should be directed to their respective
packagers.

List with some [here](https://pkgs.org/search/?q=stuntrally).

### Ubuntu

Package is available for recent versions on [launchpad
here](https://launchpad.net/~xtradeb/+archive/ubuntu/play).  
Forum topic
[here](https://forum.freegamedev.net/viewtopic.php?f=81&t=17068).

### Arch Linux

Stable releases:
[link](http://aur.archlinux.org/packages.php?K=stuntrally) (broken).

### OpenSUSE

Third-party packages available
[here](http://software.opensuse.org/search?q=stuntrally&baseproject=ALL&lang=en&include_home=true&exclude_debug=true).

### Gentoo

Can be installed from the 'gamerlay' overlay.

#### Others

If you have packaged Stunt Rally for a new distribution, we can include
links here.
