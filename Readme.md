![](/data/hud/stuntrally-logo.jpg)

[![Build game](https://github.com/stuntrally/stuntrally/workflows/Build%20game/badge.svg)](https://github.com/stuntrally/stuntrally/actions?query=workflow%3A%22Build+game%22)
![last commit](https://flat.badgen.net/github/last-commit/stuntrally/stuntrally)
![last tag](https://flat.badgen.net/github/tag/stuntrally/stuntrally)
![license](https://flat.badgen.net/github/license/stuntrally/stuntrally)

## Links

[Stunt Rally Homepage](https://stuntrally.tuxfamily.org/) - Download links, track & vehicle browsers etc.  
Sources: [New 3.x](https://github.com/stuntrally/stuntrally3), [Old 2.x](https://github.com/stuntrally/stuntrally/) - with Documentation in docs/, also for bugs & Issues, pull requests, etc  
[Forums](https://forum.freegamedev.net/viewforum.php?f=77) - Discussions, tracks, Issues, etc.  
[Screenshots](https://stuntrally.tuxfamily.org/gallery) - Galleries from all versions and development  
[Videos](https://www.youtube.com/user/TheCrystalHammer) - from game and editor  
[Donations](https://cryham.tuxfamily.org/donate/) - financial support

------------------------------------------------------------------------------

## Note

This is an older outdated version of Stunt Rally 2.x.  
New repo with Stunt Rally 3.0 and up is [here](https://github.com/stuntrally/stuntrally3).

## Description

3D racing game, based on VDrift and OGRE, with own Track Editor.

Stunt Rally features 202 tracks in 37 sceneries and 25 vehicles.  
Game modes include:
* Single Race (with your Ghost drive, track car guide), Replays,
* Challenges, Championships, Tutorials,
* Multiplayer (info [here](docs/multiplayer.md), no official server) and Split Screen.  

The Track Editor allows creating and modifying tracks.  
Both run on GNU/Linux and Windows.  

The game has a **rally** style of driving, mostly on gravel (like Richard Burns Rally),  
with possible **stunt** elements (jumps, loops, pipes) and generated roads from 3D spline.  
It also features few Sci-Fi vehicles and planets.

Full features list on [our Wiki](docs/features.md).

------------------------------------------------------------------------------

### Hardware requirements

The recommended minimum hardware is:  
* CPU: with 2 cores, above 2.4GHz,  
* GPU: dedicated, low budget (e.g. GeForce GTX 560 Ti, Radeon RX 570),  
with Shader Model 3.0 supported and 1GB GPU RAM.  

Integrated GPUs and old laptops don't handle the game well or at all.  

------------------------------------------------------------------------------

## Game

In game - Esc or Tab key shows/hides GUI.

Quick setup help is on the Welcome screen, shown at game start, or by Ctrl-F1.  
At first, open Options to adjust Screen resolution, pick graphics preset according to your GPU and *do* restart.  
Open Options tab Input, to see or reassign keys, or configure a game controller, info [here](docs/running.md#input).  
Game related Hints are available in menu: How to drive, with few driving lessons.  

Have fun 😀

### Troubleshooting

If you have problems running, check page [Running](docs/running.md).  
All settings and logs are saved to user folder, see [Paths](docs/paths.md). It is also shown on 1st Help page.

------------------------------------------------------------------------------

## Editor

In editor - F1 (or tilde) key shows/hides GUI,  
Tab key - switches between Camera and Edit modes.  
Press Ctrl-F1 to read what can be edited and how.  

There is no undo, so use F5 to reload last track state, and F4 to save it (often).  
After each save, track can be tested in game.

### Tutorial

Editor Tutorial page with more info and videos [here](docs/editor.md).

------------------------------------------------------------------------------

## Feedback

Before reporting bugs or issues, be sure to [Read before posting](https://forum.freegamedev.net/viewtopic.php?f=78&t=3814) topic first ;)

Then create a topic on Forum [SR Bugs & Help](https://forum.freegamedev.net/viewforum.php?f=78)  
or a [new issue on github](https://github.com/stuntrally/stuntrally/issues/new) (if not present). PRs are welcome too.  

------------------------------------------------------------------------------

## Building from sources

For newest sources and how to compile them see page [Compiling](docs/compile.md).

------------------------------------------------------------------------------

## Contributing

If you'd like to contribute, please check [Contributing](Contributing.md)

------------------------------------------------------------------------------

## License

    Stunt Rally - 3D racing game, based on VDrift and OGRE, with Track Editor
    Copyright (C) 2010-2023  Crystal Hammer and contributors


    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see https://www.gnu.org/licenses/


    The license GNU GPL 3.0 applies only to code written by us.
    Which is in source dir, subdirs: editor, network, ogre and road.
    Libraries used have their own licenses.
    For data licenses look in various _*.txt files in data subdirs.

------------------------------------------------------------------------------
