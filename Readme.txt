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

This game is in it's medium stage, it features
35 tracks in 6 sceneries and 7 drivable cars.
The Track Editor is fully functional now.

It should run on any Windows system (checked on XP 32bit and 7 64bit).

If you have problems running, check [issues] (closed too)
on project page, there may be a solution already.

The (recommended) minimum hardware is (at least):
a CPU with 2 cores, and GPU: GeForce 9600 GT or Radeon HD 3870.
But it's possible to run on older GPUs, the absolute minimum (ghastly) settings
are in _game-min.cfg file, rename it to _game.cfg to use them.

Car simulation is done by VDrift, a great game by itself.
Rendering is done by OGRE, trees/grass by PagedGeometry, Gui by MyGUI.

----------------------------------------------------------------------------------

Running

Run StuntRally.exe, choose Rendering SubSystem, and its options.
Press [New Game] button to start driving (or change Track/Car first).

In the game archive, there are only 5 tracks included. So if you want
all tracks, be sure to also download the "tracks pack" archive,
and extract it in the same directory where you extracted game archive
(always replace older files).

Esc/Tab key shows/hides Options, keys used in game (not configurable now)
can be seen in Options tab [Input].

Change cameras with C/X or PgDown/PgUp (with shift for main cameras only).
Cameras can be adjusted in game, by mouse - move mouse to see actions.

VDrift cars and tracks (version from 2009) can be used by copying them
into data\cars or data\tracks folder.

Editor

In editor (SR_Editor.exe), F1 key shows/hides Options,
switch to tab [Input/Help] to read what can be edited and how.
Tab key switches between Camera/Edit mode.
There isn't (and won't be) an undo function -
hit F5 to reload last track state, and F4 to save it.
After saving, track can be played in game (exports all data).

Most graphics settings work immediately, those which don't are marked by *.
VDrift tracks need Shadows: Old.
All settings are saved in _game.cfg / _editor.cfg files in exe folder.
To reset OGRE settings delete _ogreset.cfg file (_ogreset_ed.cfg for editor).

If problems occur check _ogre.log and _log.log for errors. Compare them
with those included in archive. (_ed in filename stands for editor).

Have fun !

----------------------------------------------------------------------------------

Compiling

To compile under Visual Studio 2008 use the vcproj/sln files from Source.7z archive.
Check AdditionalIncludeDirectories from compiler and AdditionalDependencies
in linker options for used libraries, there are my global paths (e:\) so you
have to adjust them for your own ones.

-- Currently these are used (and need to build before game):

OGRE + Dependencies, with plugins:
	RenderSystem_Direct3D9, RenderSystem_GL
	OgreTerrain.lib OgrePaging.lib
	Plugin_ParticleFX
	Plugin_CgProgramManager
	OIS

MyGUI3.0
	MyGUIEngine.lib, MyGUI.OgrePlatform.lib

-- and for VDrift:
SDL-1.2.14
libvorbis-1.2.3, libogg-1.1.4

-- These are included in Source (and compile with project):
PagedGeometry 1.1.0
BtOgre
Bullet 2.76
TinyXML

OGRE can be installed from SDK (used 1.7.1).
Maybe some can be too, but it's best to compile all.


----------------------------------------------------------------------------------

About

I'm aiming at a rally game with Richard Burns Rally style of driving
(probably already done, not much changes here).
With stunt elements, similar to the old game Stunts (from year 1990),
or such playable game like GeneRally.

Help is welcomed

This game and editor are still a one man project, so:
If you are a graphic artist, can create trees/plants,
and textures for them (or for terrain) for free,
or you know such free models/textures,
let me know and email me at: cryham(at)gmail(dot)com,

If you're a programmer, know OGRE, shaders,
or just know you'd done something better,
know how to do it and have time for it, let me know.

If you manage to create an interesting track (or car settings),
let me know, so it could be released in future versions.
