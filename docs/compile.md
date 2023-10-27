*How to compile (build) Stunt Rally (old 2.x) from sources - general
info.*

The **latest** Stunt Rally 3.0 (WIP, beta) has its own instruction
[page](https://github.com/stuntrally/stuntrally3/blob/main/Building.md).

  

## Latest Ogre

Latest version from [git repo](https://github.com/stuntrally/stuntrally)
(master, SR 2.7) works with Ogre 1.13.  
Earlier versions weren't tested.

Ogre 1.13 also needs specific flags set in CMake:  
`OGRE_RESOURCEMANAGER_STRICT = 0`  
`OGRE_NODELESS_POSITIONING = ON`  
Important when building Ogre from sources (and then MyGui too).  
But if Ogre was built so by packagers, then no need to build Ogre from
sources.

I'd also recommend latest Ogre [from
master](https://github.com/OGRECave/ogre/commits/master), since it has a
fix for SR editor.  
Here is a guide for [building
Ogre](https://ogrecave.github.io/ogre/api/1.12/building-ogre.html) from
sources.

### Older Ogre 1.9

Previous SR version 2.6.2 and older use Ogre 1.9 and will work only with
that (or 1.10 possibly).

Last SR git hash that builds with Ogre 1.9 is either  
\* completely before conan: `baf482661ce8263bbfaee60f9568584e8eedb9e7`  
\* or somewhat after e.g.: `9c43322f6b788871025bd0d10f7c768819dd2c17` -
before `disable pkg-config`.  
\* or possibly *last* (newest) would be:
`9e5fdf2f59df74c34fa7d9ee43027f0ac2a892dc`  
but *last* needs manual fix in `cmake/DependenciesConfig.cmake`  
`# uncomment below for Ogre 1.9`  
`PKG_CONFIG "OGRE, OGRE-Terrain, OGRE-Paging, OGRE-Overlay"`  
removed # in above line.

This should not matter for tracks repo. You can use latest tracks.  
*You can also get latest `tracks.ini` and replace (should work without
too).*

  

## Conan

SR can be built using Conan on GNU/Linux or Windows.  
Latest SR in git repo is using it.  
Logs from both builds can be seen in [Actions
tab](https://github.com/stuntrally/stuntrally/actions), for chosen
commit.  
To use Conan you'll need [Python](https://www.python.org/downloads/),
then run this command:  
`pip install conan`  
More info about [Conan here](https://conan.io/downloads.html).

If you don't have Conan installed then CMake should build the old way,
without it.  
There is a cmake variable USE_PACKAGE_MANAGER for using conan build. Can
be disabled to not use Conan.

  
===== GNU/Linux =====

Compiling Stunt Rally on Ubuntu (also [Ubuntu
based](https://en.wikipedia.org/wiki/List_of_Linux_distributions#Ubuntu-based)
distros and [Debian based](https://www.debian.org/derivatives/),
[wiki](https://en.wikipedia.org/wiki/List_of_Linux_distributions#Debian-based),
[other
list](https://distrowatch.com/search.php?basedon=Debian&status=All#distrosearch))
was quite easy,  
if Ogre 1.9 is available and all dependencies can be just installed
using apt-get.  
SR 2.7 switched to 1.13, making it more difficult on some distros, since
it would need building Ogre 1.13 and then MyGui from sources.  
For more info about Ogre packages in
[distros](https://en.wikipedia.org/wiki/Linux_distribution), check
[here](https://pkgs.org/search/?q=ogre) or in package manager.  
E.g. Debian 10 has Ogre 1.9 (also Fedora 35, 36), and Debian 11 has both
Ogre 1.9 and 1.12.

On other GNU/Linux distributions building probably isn't much trouble,  
if you know how to build 1 or 2 dependencies from sources (if they
aren't available).

*Since it is (or was) way easier it's the recommended way, even if you
don't know any [Ubuntu
OS](http://en.wikipedia.org/wiki/Ubuntu_%28operating_system%29),
[Debian](https://www.debian.org/) or GNU/Linux at all.*

How to compile on GNU/Linux continued on this [Wiki page](compilelinux).

Optionally use SR_FORCE_SYSTEM_DEPENDENCIES CMake variable to force
using system deps (Ogre, MyGui etc) instead of detecting them by Conan.

  

## Windows

Recent SR 2.7 version on repo master, has Conan support merged and
working. Link to [conan PR with info
here](https://github.com/stuntrally/stuntrally/pull/39).  
Conan is used to automate and simplify getting dependencies like Ogre,
MyGui for SR.

First, install [Git](https://git-scm.com/downloads),
[CMake](https://cmake.org/download/),
[Python](https://www.python.org/downloads/) and
[Conan](https://conan.io/downloads.html), if not already present.

Getting SR sources, run:  
`git clone https://github.com/stuntrally/stuntrally.git`  
This will create stuntrally folder.

Start CMake Gui, use this stuntrally folder for sources and e.g. `build`
subdir inside or other folder for binaries.  
CMake builds should now use Conan (variable USE_PACKAGE_MANAGER).  
Press Configure (it will ask for build system, Visual Studio version)
and get deps (Ogre etc. may take a while).  
Then press Generate in CMake Gui.  
If both succeeded there should be StuntRally.sln generated in chosen
build folder.  
Open it in Visual Studio and build.

  
If building with Conan fails, there are about 2 other alternatives,
which are way harder to complete and I don't recommend any.  
- Using vcpkg which was started and has its own issues and probably
fails.  
- Building all deps from sources which is a painfully long, tedious
process, and needs to be done cautious too, since all deps need to use
same options.  

### Building all from sources

On Windows, compiling SR with dependencies (all from sources) is
tedious, and can be a nightmare.  
I only recommend this for those who did such operations before and have
experience with it or a lot of patience.

How to compile on Windows continued on this [Wiki page](compilevs). It
is very old and outdated, but the process is the same.

#### vcpkg

Looks that Ogre 1.12 is there and almost all dependencies. But to build,
many changes were needed in SR, and are on a branch in PR, older.  
Link to [vcpkg PR with info
here](https://github.com/stuntrally/stuntrally/pull/34).
