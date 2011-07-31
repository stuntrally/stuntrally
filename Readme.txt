----------------------------------------------------------------------------------

    Stunt Rally - game based on VDrift and OGRE, with Track Editor
    Copyright (C) 2011  Crystal Hammer

    Project's Homepage:  (all links, Issue tracker, Wiki pages, old Windows releases)
            http://code.google.com/p/vdrift-ogre/
    Windows releases:
            https://sourceforge.net/projects/stuntrally/files/
    Screenshots gallery:  (from all versions)
            http://picasaweb.google.com/CryHam/
    Videos:  (gameplay and editor)
            http://www.youtube.com/user/TheCrystalHammer
    Git repositories:  (for latest sources, data, and tracks)
            https://github.com/stuntrally/

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
----------------------------------------------------------------------------------

Description

This game features 55 tracks in 9 sceneries and 8 drivable cars.
The Track Editor allows modifying and creating tracks.
Both game and editor run on Windows and Linux.

Hardware requirements

The recommended minimum hardware is:
a CPU with 2 cores, and GPU: GeForce 9600 GT or Radeon HD 3870,
with Shader Model 3.0 supported.

It is possible to run on older,
(e.g. on 1 core CPU and GeForce 7600 GT, with 30 fps),
but integrated GPUs (or laptops) can't handle the game.

The absolute minimum (ghastly) settings are
in config/game-min.cfg file, copy it to game.cfg to use them.

----------------------------------------------------------------------------------

About

The game aims at a rally style of driving (like in Richard Burns Rally),
with stunt elements, similar to the old game Stunts (from year 1990),
or such playable games like GeneRally or Revolt.
It also introduces road pipes, and provides a 3D spline generated road.

----------------------------------------------------------------------------------

Running

Run StuntRally, choose Rendering SubSystem (or [Video] tab), and its options.
You can change Track/Car with Up/Down keys and start with Enter.

Esc/Tab key shows/hides Options, keys used in game
can be seen in Options tab [Input].

Change cameras with C/X (with shift for main cameras only).
Cameras can be adjusted in game, by mouse - move mouse to see actions.

All settings and logs are saved to user folder (check Wiki page [Paths]).
If you have problems running, read Wiki page [Running].
If it doesn't help, search all [Issues] on project page (closed too).

Editor

In editor, F1 key shows/hides Options,
switch to tab [Input/Help] to read what can be edited and how.
Tab key switches between Camera/Edit mode.
There isn't (and won't be) an undo function -
hit F5 to reload last track state, and F4 to save it.
After saving, track can be played in game (exports all data).

Have fun !

----------------------------------------------------------------------------------

Compiling

For newest sources and how to compile them check
project's Wiki page [Compiling].

We are using CMake and C++.
Car simulation is done by VDrift, a great game by itself (also using Bullet).
Rendering is done by OGRE, trees/grass by PagedGeometry, Gui by MyGUI.

-- Currently these are used (and need to build before game):
OGRE 1.7 + Dependencies, with plugins:
    RenderSystem_GL, RenderSystem_Direct3D9
    OgreTerrain, OgrePaging
    Plugin_ParticleFX
    Plugin_CgProgramManager
    OIS
MyGUI 3.0
    MyGUIEngine, MyGUI.OgrePlatform
boost 1.43
    headers, thread, filesystem

-- And for VDrift:
SDL-1.2.14
libvorbis-1.2.3, libogg-1.1.4

-- These are included in Source (and compiled with project):
Bullet 2.76
BtOgre
OISB
PagedGeometry 1.1.0
TinyXML

----------------------------------------------------------------------------------

Help is welcomed

Currently we are 3 developers, we are looking for:

Track creators
    If you manage to create an interesting track,
    it could be released in future versions.

Artists
    If you can create tree/plant / car models,
    textures for them or for terrain.
	If you can create/edit sounds for game.

Programmers
    If you know C++, OGRE or Shaders, and
    could handle one (or some) of the project's [Issues], 
    or want to do something better and know how to.

Translators
    If you want to translate the game into a language
    that isn't yet available, check wiki page [Localization].

If so, let us know by posting on Issue tracker,
or tell us on IRC at #stuntrally on freenode,
or email me at cryham(at)gmail(dot)com.
