*How to compile the game from sources, on GNU/Linux.*

## Get sources

It is recommended that you get the latest version directly from the Git
code repository.  
Use software manager to install git or this command (on Debian based):

    sudo apt-get install git

Then, use these commands to get sources:

Newest SR 2.7 - use tracks repo as submodule:

    git clone --depth=1 https://github.com/stuntrally/stuntrally.git stuntrally
    cd stuntrally
    git submodule init
    git submodule update --depth=1

Older SR versions - needed to clone tracks repo:

    git clone --depth=1 https://github.com/stuntrally/stuntrally.git stuntrally
    cd stuntrally/data
    git clone --depth=1 https://github.com/stuntrally/tracks.git tracks

  
This will get sources to a directory called "stuntrally" and inside it
will download the tracks to a subdirectory "data/tracks".  
If you got sources some other way, be sure to have tracks/ inside data/.

> Note: The parameter --depth=1 tells git to not get history (to lower
> download time and space).  
> This way data is about: 300 MB and 500 MB for tracks.  
> With history it's quite big: 500 MB and 1,5 GB for tracks.

> You can use other depth value to get some recent commits.  
> Or download an archive from github:
> [here](https://github.com/stuntrally/stuntrally/tags) and tracks
> [here](https://github.com/stuntrally/tracks/tags).

  
If you want to use a (stable) release version, instead of the (latest)
development version,  
use the following command (replace 2.7 with the desired version number):

    git checkout 2.7

You can see the available versions with the following:

    git tag

(won't be any if --depth=1 was used)

  

## Dependencies

Here's a list of libraries you will need.  
*Newer versions should be used (except Ogre), listed here are only
minimal versions needed (for VM with Ubuntu 12.04 LTS).*

<table>
<thead>
<tr class="header">
<th>Library</th>
<th>Downloads page</th>
<th>Sources</th>
</tr>
</thead>
<tbody>
<tr class="odd">
<td>OGRE 1.9 (or 1.10) with plugins:<br />
OgrePaging, OgreTerrain, Plugin_ParticleFX</td>
<td><a href="https://www.ogre3d.org/download">https://www.ogre3d.org/download</a></td>
<td><a href="https://bitbucket.org/sinbad/ogre">https://bitbucket.org/sinbad/ogre</a></td>
</tr>
<tr class="even">
<td>MyGUI 3.2 with Ogre support</td>
<td><a href="https://mygui.info">https://mygui.info</a></td>
<td><a href="https://github.com/MyGUI/mygui">https://github.com/MyGUI/mygui</a></td>
</tr>
<tr class="odd">
<td>SDL2</td>
<td><a href="https://libsdl.org/download-2.0.php">https://libsdl.org/download-2.0.php</a></td>
<td><a href="https://libsdl.org/hg.php">https://libsdl.org/hg.php</a></td>
</tr>
<tr class="even">
<td>OGG, VorbisFile</td>
<td><a href="https://xiph.org/downloads">https://xiph.org/downloads</a></td>
<td></td>
</tr>
<tr class="odd">
<td>Boost (system, thread, filesystem, wave)</td>
<td><a href="https://www.boost.org/users/download">https://www.boost.org/users/download</a></td>
<td></td>
</tr>
<tr class="even">
<td>ENet</td>
<td><a href="https://enet.bespin.org/Downloads.html">https://enet.bespin.org/Downloads.html</a></td>
<td><a href="https://github.com/lsalzman/enet">https://github.com/lsalzman/enet</a></td>
</tr>
<tr class="odd">
<td>Bullet Physics 2.81</td>
<td><a href="https://www.bulletphysics.org/Bullet/phpBB3/viewforum.php?f=18">https://www.bulletphysics.org/Bullet/phpBB3/viewforum.php?f=18</a></td>
<td><a href="https://github.com/bulletphysics/bullet3">https://github.com/bulletphysics/bullet3</a></td>
</tr>
<tr class="even">
<td>OpenAL Soft 1.13</td>
<td><a href="https://github.com/kcat/openal-soft/tags">https://github.com/kcat/openal-soft/tags</a></td>
<td><a href="https://github.com/kcat/openal-soft">https://github.com/kcat/openal-soft</a></td>
</tr>
</tbody>
</table>

Either install them from your distribution's package manager (including
the -dev packages), or download and compile them yourself.

  
In Ubuntu (or Debian based), these commands should get you all
dependencies.

    sudo apt-get update
    sudo apt-get install --assume-yes git cmake g++

    # boost
    sudo apt-get install --assume-yes libboost-wave-dev libboost-system-dev libboost-filesystem-dev libboost-thread-dev

    # graphics
    sudo apt-get install --assume-yes libogre-1.9-dev libmygui-dev libsdl2-dev

    # sound
    sudo apt-get install --assume-yes libogg-dev libvorbis-dev libenet-dev libopenal-dev

    # bullet
    sudo apt-get install --assume-yes libbullet-dev libbullet-extras-dev

for latest SR ver 2.7 also

    # tinyxml
    sudo apt-get install --assume-yes libtinyxml-dev libtinyxml2-dev

> Note1: extras may be already inside libbullet, on older. Needed for
> BulletFileLoader and BulletWorldImporter.
>
> Note2: libbullet and libopenal are present since ver 2.6 or master
> since late Aug 2015.

  
These are included in sources and compiled with project (no need to do
anything):

-   [BtOgre](https://www.ogre3d.org/forums/viewtopic.php?f=5&t=46856)
    **#**
-   [OICS](https://sourceforge.net/projects/oics/) **#**
-   [PagedGeometry](https://www.ogre3d.org/addonforums/viewforum.php?f=14)
    1.1.1 **#**
-   [shiny](https://github.com/scrawl/shiny/)
-   [TinyXML](https://www.grinninglizard.com/tinyxml/index.html),
    [TinyXML2](https://github.com/leethomason/tinyxml2) (before SR ver
    2.7)

Marked modified sources with **#**.

  

## Compiling

Project is written in C++. For compilation, we use CMake (and since SR
2.7 also conan, optionally).  
Use following commands *(for Unix-like shell environments, such as Bash
or MSYS)*:

    cd stuntrally   # Change directory to the game sources
    mkdir build     # Create a build directory
    cd build        # Change to the build directory

    cmake ..        # Create build files
    make -j4        # Compile using 4 threads (change to your number of CPUs)

If you have [conan](https://conan.io/downloads.html) installed
(`pip install conan`) build will use it.  
If not, it will build without conan.

  

## Troubleshooting

If you get errors on the "cmake"-step, you are most likely missing some
library or CMake couldn't find it. Install the development files for the
library in question and if automatic finding fails, tell the paths to
CMake manually. On Windows you should be able to open up a graphical
interface by double-clicking the CMakeCache.txt file in the build
directory. Similar interface is also available on other platforms, but
the command-line alternative "ccmake" is also quite popular on
GNU/Linux.

CMake has [various
generators](https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/Generator-Specific-Information)
besides Makefiles for you to choose from, including Visual Studio,
Eclipse CDT and Code::Blocks project files.

#### Useful CMake variables

Here's some variables that come in handy frequently. You can set them on
command line by: `cmake -DVARIABLE_NAME=VALUE` or use one of the GUIs.

-   CMAKE_BUILD_TYPE - this determines release/debug builds, possible
    values are Release, Debug, MinSizeRel, RelWithDebInfo, defaulting to
    the last one.
-   CMAKE_INSTALL_PREFIX - Specify the path where files are copied if
    `make install` is invoked (on GNU/Linux, usually `/usr` or
    `/usr/local`).
-   BUILD_EDITOR - Disable editor compilation by setting this to OFF.

For more information about CMake, see the
[documentation](https://cmake.org/documentation/) and
[wiki](https://gitlab.kitware.com/cmake/community/-/wikis/home).

  

## Running

Once everything is compiled, you can run the game from the build
directory with `./stuntrally`.  
If you didn't use CMake, you'll probably have the binary in the source
root, from where you can also run the game.  
Same also applies to the editor, which can be started with
`./sr-editor`.

More information available in the [Running](Running) wiki page.

    # Optionally install everything (you can run without installing)
    make install    # might need administrator privileges
