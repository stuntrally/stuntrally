The process of releasing and things that need to be checked before.

### 1. Code

-   Fix crashes and critical bugs
-   Be sure the code works and there is nothing that would make it
    broken
-   Make sure all tools in code use `#if 0`, search for `_Tool_`
-   Try a clean rebuild and check `make install`

### 2. Update

-   Content
    -   For new tracks: make track's ghosts, add to ini, champs and
        challs xmls (all in config/)
    -   Run editor tools to check tracks (errors, presets, etc):
        ToolSceneXml, ToolTracksWarnings meh
    -   For new cars: check in-car cameras, perf test, easy sim, etc
-   `Readme.txt` (links, track count, etc)
-   Version numbers
    -   in Gui, file `*_en.tag.xml`, tag `name="GameVersion"`
    -   in `installer.nsi` script, at top
-   Localization
    -   run `locale/tx_1upd-all.bat` or `1upd-all.sh`, so there are no
        `#{MyGUI Tags}` in GUI
    -   run it again after translations have been made

### 3. Test

-   Check all the game modes (Championship, Challenge, Split screen,
    Multiplayer, Replays)
-   Check at least new features and important older ones (see
    [Features](features.md))
-   Delete/rename your user dir to check default config and cache
    generation
-   Try a few different cars and tracks
-   Test graphics presets from combo and effects
-   Try editor (duplicate a track, save, and at least basic editing)

### 4. Packages

-   Create packages for supported systems and test them
    -   Linux binaries - need VM (was 12.04), info how to here
        [dist/linux-archive/Readme.md](https://github.com/stuntrally/stuntrally/tree/master/dist/linux-archive)
    -   Windows installer - use dist/installer.nsi with
        [NSIS](http://nsis.sourceforge.net/Main_Page)
-   Make sure all packages use the same version of the sources
-   Once the packages work, tag the version number to all repositories
    (stuntrally and tracks)
-   Upload packages, download back and test (or check checksums) to make
    sure there wasn't corruption

### 5. Websites

-   Update websites
    -   [Homepage](http://stuntrally.tuxfamily.org/), tracks count,
        [Downloads](http://stuntrally.tuxfamily.org/downloads) release
        date, links
    -   [Changelog](changelog.md), [Statistics](statistics.md) (in SR3),
        [Features](features.md)
-   Update Tracks Browser
    -   rename tracks.ini to .txt, use dist/make_roadstats_xml.py, copy
        into cms
    -   also cars.xml for Car Browser, remove unnecessary comments
-   For significant look changes update home screen, and pics on other
    websites
-   Notify known third-party packagers about new release
-   Announce release - on forums: [SR General](http://forum.freegamedev.net/viewforum.php?f=81),  
    [Ogre showcase](http://www.ogre3d.org/forums/viewtopic.php?f=11&t=58244),
    [VDrift dev](http://vdrift.net/Forum/showthread.php?tid=1629), email
    [freegamer](http://freegamer.blogspot.com/), etc.
