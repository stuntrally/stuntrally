*Where is your user data located (also saved logs and configs).*

  

## Introduction

This page describes where the game searches for its data and
configuration files.

The information is useful if you are e.g.:

-   submitting a bug/crash (you have to include your configs and logs)
-   need to reset or backup settigs
-   doing manual configuration (without the GUI)
-   or packaging Stunt Rally

## Location of User's dir

***Since version 1.9 you can see the path used, directly in game, in
Gui, on Help page, under User Path. You can mark it and copy.***

On Windows Vista and 7, the user's directory is located in

    C:\Users\USERNAME\AppData\Roaming\stuntrally

Earlier versions of Windows probably use

    C:\Documents and Settings\USERNAME\Application Data\stuntrally

Note that the actual directory names might be affected by localization
of your Windows.

On GNU/Linux the user's dir is most likely

    ~/.config/stuntrally

(where tilde means the user's home directory).

However, `$XDG_CONFIG_HOME/stuntrally` takes precedence if the
environment variable is set.

  

## User Configs and Logs

The **default** configuration files are located in the data path, under
`config` subdirectory. The game creates dedicated configuration files
for each users and they take precedence over the defaults. If you want
to reset your configuration to defaults, just delete your user's dir
game.cfg, input\*.xml, editor.cfg.

Files in user's dir are:

-   **Configs**
    -   editor.cfg - settings for editor
    -   game.cfg - settings for game
    -   input.xml - game input bindings from General page only
    -   input_p0.xml .. input_p3.xml - input bindings for players 1..4
        (you can copy them for backup, and replace files to have
        different setups)
    -   progress.xml - championships progress
    -   progress_rev.xml - same as above but for reverse direction
    -   progressL.xml, progressL_rev.xml - challenges progress
-   **Logs**
    -   ogre.log - rendering and game log (most important)
    -   ogre_ed.log - editor log
    -   ogre.err, ogre_ed.err - same as .log but only with errors and
        warnings, also from std::cerr
    -   log.txt - VDrift simulation log *(since 2.6 not present,
        included in ogre.log)*
    -   MyGUI.log - not important, stuff from GUI

## User data

As the game's installation directory might very well be write protected
from non-administrators, your user generated data is saved in user's
dir. This data might be critical to the game or editor, but they can
start without it with default settings.

Subdirs are: *(mode is easy or normal, both simulation modes)*

-   cache - see more below
-   data/carsim/mode/cars - if you edit car (Alt-Z) and save it, then it
    will be here
-   data/carsim/mode/tires - if you edit tires (one of graphs mode F9),
    and then save them (Alt-Z, Tires tab), they will appear here
-   ghosts/mode - best laps for each car on each track are saved here
-   records/mode - .txt files for best times on tracks
-   replays - if you save replays they land here
-   screenshots - filled when making screens in game or editor (by
    default F12)
-   tracks - user made tracks are located here. To share a track just
    pack the track's dir. Then unpacking here makes the track available.

If you want to reset record time for a track just go inside records/ and
delete the .txt file with same track name (or edit it and remove section
for a car name).

To completely remove the game you need to delete your user's dir. Ghosts
and replays may become big (e.g. 1GB).

  

## Game Data

As a rule of thumb, Stunt Rally doesn't care what its working directory
is (i.e. from where you run the game). It will detect the actual
executable location and search the data in relation to that.

There are few different possibilities here, making it possible to run
the game from the Git source tree as well as from `make install`ed
prefix. On Linux, common system data installation paths as well as
`XDG_DATA_DIRS` are checked too.

If you have your data in a really obscure path, you can always define
`STUNTRALLY_DATA_ROOT` environment variable.

## Cache

Cache is used for automatically generated files. Deleting cache files is
safe - the application will re-create them when they are next needed,
although this might result in a temporary slow-down.

On Windows, this is currently the same as user's config directory.

On Linux (and other similar systems), the cache dir is most likely
`~/.cache/stuntrally` (where tilde means the user's home directory).
However, `$XDG_CACHE_HOME/stuntrally` takes precedence if the
environment variable is set.

  

## OGRE plugins

On Windows, Ogre plugins should be in the same directory with the exe.
On Linux, the following paths are searched (in that order):

1.  `/usr/local/lib64/OGRE` (only on 64 bit systems)
2.  `/usr/local/lib64/ogre` (only on 64 bit systems)
3.  `/usr/lib64/OGRE` (only on 64 bit systems)
4.  `/usr/lib64/ogre` (only on 64 bit systems)
5.  `/usr/local/lib32/OGRE` (only on 32 bit systems)
6.  `/usr/local/lib32/ogre` (only on 32 bit systems)
7.  `/usr/lib32/OGRE` (only on 32 bit systems)
8.  `/usr/lib32/ogre` (only on 32 bit systems)
9.  `/usr/local/lib/OGRE`
10. `/usr/local/lib/ogre`
11. `/usr/lib/OGRE`
12. `/usr/lib/ogre`

On all platforms, you can specify a custom location for the plugins by
setting the environment variable `OGRE_PLUGIN_DIR`.

  

## Config XMLs

Here is a list of config files used in game and editor. Changing them
isn't needed.

This can be useful for those who want to know, tinker or extend the
content (no need to build).

\* Game

-   config/colors.ini - *since 2.6*, has all colors presets (Vehicle
    tab) and Gui settings for row count and image size
-   config/championships.xml - contains all championships with tracks
-   config/challenges.xml - has all challeges setup, see top of file for
    complete reference  
    if you want to edit or make your own challenges
-   config/cars.xml - has all what is shown in cars list and some timing
    factors
-   data/cars/cameras.xml - all cameras with their parameters, own
    camera setups can be made there

\* Common

-   config/tracks.ini - all track stats shown in tracks list, and track
    time, more info in file at top
-   data/materials2/fluids.xml - has all params for water/mud buoyancy
-   data/trees/collisions.xml - setup for all vegetation models
    collision (single capsule for trees, trimesh for rocks etc)  
    can be edited and tested directly in game
-   data/materials/scene/\*.mat - has most materials, useful eg. when
    addig new skies, grasses, trees, road and object materials and/or
    tweaking their look

\* Editor

-   config/presets.xml - *since ver 2.3*, contains parameter values for
    terrain layers, road,  
    grasses and vegetation (shown in lists in pick window)  
    adding here is needed if you make any new material (so it is shown
    in editor and can be picked)  
    *if not then only way is to put in scene.xml in track*
