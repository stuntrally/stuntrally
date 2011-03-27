----------------------------------------------------------------------------------

    Stunt Rally - game based on VDrift and OGRE, with Track Editor
    Copyright (C) 2010  Crystal Hammer

	project:		http://code.google.com/p/vdrift-ogre/
	screenshots:	http://picasaweb.google.com/CryHam/
	videos:			http://www.youtube.com/user/TheCrystalHammer

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

This game features 49 tracks in 6 sceneries and 7 drivable cars.
The Track Editor allows modifying and creating tracks.
Both game and editor run on Windows and Linux.

The recommended minimum hardware is:
a CPU with 2 cores, and GPU: GeForce 9600 GT or Radeon HD 3870.
But it's possible to run on older, the absolute minimum (ghastly) settings
are in game-min.cfg file, copy it to game.cfg to use them.

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

In the game archive, there are only 5 tracks included. So if you want
all tracks, be sure to also download the "Tracks Pack" archives,
and extract it in the same directory where you extracted game archive
(always replace older files).

Esc/Tab key shows/hides Options, keys used in game (not configurable now)
can be seen in Options tab [Input].

Change cameras with C/X or PgDown/PgUp (with shift for main cameras only).
Cameras can be adjusted in game, by mouse - move mouse to see actions.

All settings and logs are saved user folder (check Wiki page [Paths]).
If you have problems running, read Wiki page [Running].
If it doesn't help, check [Issues] on project page (closed too).

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
project's Wiki page [Compiling]. We are using CMake and C++.

Car simulation is done by VDrift, a great game by itself.
Rendering is done by OGRE, trees/grass by PagedGeometry, Gui by MyGUI.

-- Currently these are used (and need to build before game):

OGRE + Dependencies, with plugins:
	RenderSystem_GL, RenderSystem_Direct3D9
	OgreTerrain, OgrePaging
	Plugin_ParticleFX
	Plugin_CgProgramManager
	OIS

MyGUI3.0
	MyGUIEngine, MyGUI.OgrePlatform

-- and for VDrift:
SDL-1.2.14
libvorbis-1.2.3, libogg-1.1.4

-- These are included in Source (and compile with project):
PagedGeometry 1.1.0
BtOgre
Bullet 2.76
TinyXML

OGRE can be installed from SDK package (used 1.7.1).
Maybe some can be too, but it's best to compile all.

----------------------------------------------------------------------------------

Help is welcomed

Currently we are 3 developers, we are looking for:

Track creators
    If you manage to create an interesting Track (or car settings)
    it could be released in future versions.

Graphic Artists
    If you can create tree/plant models,
    textures for them or for terrain, or you
    know such free models/textures (there are some out there).

Programmers
    If you know C++, OGRE or shaders, and
    could handle one (or some) of the project's [Issues], 
    or want to do something better and know how to.

If so, let us know by replying on Issue tracker,
or email me at cryham(at)gmail(dot)com.
