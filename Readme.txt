----------------------------------------------------------------------------------

    Stunt Rally - game based on VDrift and OGRE, with Track Editor
    Copyright (C) 2012  Crystal Hammer

    Project's Homepage:  (all links, Issue tracker, Wiki pages, old Windows releases)
            http://code.google.com/p/vdrift-ogre/

    Releases (Windows, Linux, source packs):
            https://sourceforge.net/projects/stuntrally/files/
    Git repositories:  (for latest sources, data, and tracks)
            https://github.com/stuntrally/

    Screenshots gallery:  (from all versions)
            http://picasaweb.google.com/CryHam/
    Videos:  (gameplay and editor)
            http://www.youtube.com/user/TheCrystalHammer
    Editor tutorial:
            http://code.google.com/p/vdrift-ogre/wiki/TrackEditor

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

This game features 79 tracks in 11 sceneries and 10 drivable cars.
The Track Editor allows modifying and creating tracks.
Both run on Windows and Linux.

Hardware requirements

The recommended minimum hardware is:
a CPU with 2 cores, and GPU: GeForce 9600 GT or Radeon HD 3870,
with Shader Model 3.0 supported.

It is possible to run on older,
(e.g. on 1 core CPU and GeForce 7600 GT, with 30 fps),
but integrated GPUs (or laptops) can't handle the game.

----------------------------------------------------------------------------------

About

The game aims at a rally style of driving (like in Richard Burns Rally),
with stunt elements, similar to the old game Stunts (from year 1990),
or such playable games like GeneRally or Revolt.
It also introduces road pipes, and provides a 3D spline generated road.

----------------------------------------------------------------------------------

Running

Run StuntRally. Esc/Tab key shows/hides Options.

At first (in Options window) go to tab [Screen] and adjust resolution.
Pick quick settings preset (from the combobox) according to your GPU.

Keys used in game can be seen in Options tab [Input].
If you want to reassign keys, or have a game controller
go to tab [Input] to configure it and test range.

You can change Track/Car (on their tabs) with Up/Down keys and start with Enter.
Change cameras with C/X (with shift for main cameras only).
Cameras can be adjusted in game, by mouse - move mouse to see actions.

All settings and logs are saved to user folder (check Wiki page [Paths]).
If you have problems running, read Wiki page [Running].
If it doesn't help, look for [Issues] on project's page, if a similar issue
doesn't exist report a new issue. Be sure to attach your log files.

Editor

In editor, F1 key shows/hides Options,
switch to tab [Input/Help] to read what can be edited and how.
Tab key switches between Camera/Edit mode.
There isn't (and won't be) an undo function -
hit F5 to reload last track state, and F4 to save it.
After saving, track can be played in game (exports all data).
There is a Editor tutorial Wiki page with videos (link at top).

Have fun !

----------------------------------------------------------------------------------

Compiling

For newest sources and how to compile them check
project's Wiki page [Compiling].

We are using CMake and C++.
Car simulation is done by VDrift, a great game by itself (also using Bullet).
Rendering is done by OGRE, trees/grass by PagedGeometry, Gui by MyGUI.

-- Currently these are used (and need to build before game):
newer versions can be used
OGRE 1.8 with plugins: (1.7 works but has fewer effects)
    RenderSystem_GL, RenderSystem_Direct3D9
    OgreTerrain, OgrePaging
    Plugin_ParticleFX
    Plugin_CgProgramManager
    OIS
MyGUI 3.2
    MyGUIEngine, MyGUI.OgrePlatform
Boost 1.43
    headers, thread, filesystem
Bullet 2.76
	BulletCollision, BulletDynamics, LinearMath
	
-- And for VDrift:
SDL-1.2.14
libvorbis-1.2.3, libogg-1.1.4

-- These are included in Source (and compiled with project):
BtOgre *
OISB *
PagedGeometry 1.1.0 *
TinyXML
* are modified sources

----------------------------------------------------------------------------------

Help is welcomed

Currently we are 4 developers, we are looking for:

Track creators
    If you manage to create an interesting track,
    it could be released in future versions.

Artists
    If you can create tree/plant / car models,
    textures for them or for terrain.
    If you can create/edit sounds for the game.

Programmers
    If you know C++, OGRE or shaders, and
    could handle one (or some) of the project's [Issues], 
    or want to do something better and know how to.

Translators
    If you want to translate the game into a language
    that isn't yet available, check wiki page [Localization].

If so, let us know by posting on Issue tracker,
or tell us on IRC at #stuntrally on freenode,
or email me at cryham (at) g m a i l (dot) com.
