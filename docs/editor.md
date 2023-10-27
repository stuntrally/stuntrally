*Help on using Track Editor.*

## Introduction

Stunt Rally includes a Track Editor that can be used for creating your
own tracks or modifying the existing ones.

# Video Tutorial

There is a 10 chapters video tutorial (version 1.7) on YouTube:

|                                                                         |
|-------------------------------------------------------------------------|
| [Chapter 1: Intro](http://www.youtube.com/watch?v=nJrpQbwzlXE)          |
| [Chapter 2: Terrain](http://www.youtube.com/watch?v=FDJFRgW52ms)        |
| [Chapter 3: Road, basic](http://www.youtube.com/watch?v=ylUySzCSjd8)    |
| [Chapter 4: Road, advanced](http://www.youtube.com/watch?v=Gycjc7noflY) |
| [Chapter 5: Fluids](http://www.youtube.com/watch?v=CiKkueI2tqU)         |
| [Chapter 6: Vegetation](http://www.youtube.com/watch?v=L6eZKLG79TQ)     |
| [Chapter 7: Checkpoints](http://www.youtube.com/watch?v=LlU1Sp_6bcU)    |
| [Chapter 8: Drive](http://www.youtube.com/watch?v=qdLlY4EYnyM)          |
| [Chapter 9: Objects](http://www.youtube.com/watch?v=9DzgmYmxu3w)        |
| [Chapter 10: Tools](http://www.youtube.com/watch?v=6GQx0pW6HEk)         |

and 4 addidtional chapters (from version 2.1)

|                                                                             |
|-----------------------------------------------------------------------------|
| [Chapter 11: Fog](http://www.youtube.com/watch?v=yNPIR29iv0U)               |
| [Chapter 12: Grasses](http://www.youtube.com/watch?v=xeazOxtcLCY)           |
| [Chapter 13: Warnings](http://www.youtube.com/watch?v=UWvm-okLuy8)          |
| [Chapter 14: Terrain, advanced](http://www.youtube.com/watch?v=v8Evcw7nLok) |

These videos (64 minutes total) cover creating a new track in detail.
Even if being quite old now.  
There is a **more recent** page describing editor features
[here](https://cryham.tuxfamily.org/portfolio/2015-sr-track-editor).

*Note:* Creating a very simple track (or just editing an existing one)
doesn't require watching all parts (at least 1,2,3,7).  
Creating a good, long or difficult track reaches 2 or more hours of
editing. But a simple track can be done in 5 minutes if you know editor.

  

------------------------------------------------------------------------

### Chapter 1: Intro

0:00 gui navigation, screen resolution, graphics presets  
0:51 help window, shortcuts, tooltips  
2:09 basics  
2:30 camera  
3:20 edit modes (terrain)  
5:19 edit modes (road, water)  
5:48 approaches to making tracks,  
7:45 quickest track - terrain align to road result (TestC8-Align)

### Chapter 2: Terrain

0:00 duplicate track, set name, delete rest  
0:36 terrain and heightmap sizes  
1:06 brush presets (used: deform, noise, filter)  
2:47 new terrain layer sand

### Chapter 3: Road, basic

0:00 help pages  
0:30 insert basic road shape  
1:33 selection, rebuild, move point, height, roll angle  
2:57 align terrain to road done  
3:14 bridge  
3:43 bridge entry fix  
4:15 smooth (flatten) brush  
4:33 place car start, rotate

### Chapter 4: Road, advanced

0:00 copy loop from other track, trk find use, rotate sel  
1:17 make pipe, adjust  
2:34 copy, insert point, adjust  
3:16 banked terrain corner  
4:00 copy jumps from other track

### Chapter 5: Fluids

0:00 add water are, sizing, move, height (depth), deform terrain  
2:20 add mud area  
2:57 road material changing to sand (part)

### Chapter 6: Vegetation

0:00 adjust density of trees and grass  
0:20 grass min. height  
1:21 parameters description  
1:54 vegetation models, and their parameters

### Chapter 7: Checkpoints

0:00 road direction set +1/-1  
0:26 mark 1st checkpoint (1)  
0:35 adding checkpoints  
1:40 track description text  
2:07 track preview screenshot

### Chapter 8: Drive

driving tutorial track with comments on minor track errors left

### Chapter 9: Objects

0:00 objects list, inserting  
0:45 height adjust, stacking  
0:56 simulation toggle  
1:50 select, move, delete  
3:13 static building, scaling  
3:47 multiselect, move group  
4:15 driving on track, hitting objects

### Chapter 10: Tools

0:17 pick source track, copy terrain layers, vegetation, road
parameters  
1:04 sun tab, fog, rain, sky  
2:08 sun light direction angles  
2:43 copy, turning scenery to desert  
3:16 terrain generator  
4:09 scale track, terrain height  
4:45 terrain layers particles, trail color  
5:18 surfaces, for tire parameters  
5:51 sunset, ambient color, sky

### Chapter 11: Fog

  

### Chapter 12: Grasses

  

### Chapter 13: Warnings

  

### Chapter 14: Terrain, advanced

0:00 Terrain generator  
2:58 New Brushes  
(3:07 Top view)  
4:53 Triplanar1 option and Terrain Error

------------------------------------------------------------------------

  

# Shortcuts

All key and mouse actions can be seen in Options tab \*Input/Help\*, use
Ctrl-F1 to show tab for current mode.

The text included on those 8 pages is a complete reference to all editor
actions (and gets translated to your language if supported).

This text here is only a quick reference to most common actions.

  

**F1 or \` - GUI Options toggle**

**Tab - Camera / Edit mode toggle** (current mode is shown in left
bottom corner: Cam, Edit or Gui)

  
### Camera

| Keys:                             | W,S, A,D, Q,E - move |                  | Arrows - rotate |
|-----------------------------------|----------------------|------------------|-----------------|
| Mouse:                            | LMB - rotate         | RMB - move (pan) | MMB - zoom      |
| All moves and value changes with: |                      |                  |                 |
|                                   | shift - slower       | ctrl - faster    |                 |

### Edit Modes

-   Terrain
    -   D - Deform
    -   E - Set Height
    -   S - Smooth (flatten)
    -   F - Filter (remove bumps,noise)

<!-- -->

-   Road
    -   R - Edit Road
    -   Q - car Start Position and area

<!-- -->

-   Extras
    -   W - fluid areas (water or mud)
    -   X - objects (dynamic or static)

Ctrl-F1 - Press to show help page for current mode.

### Main

F4 - Save track (after this track is ready to be played in game)  
F5 - Reload track (one time undo)

### Toggle

V - toggle Vegetation (hide it, for more fps when editing road)  
G - toggle Fog (when editing from distance)  
I - toggle Weather (rain, snow)  
Alt-Z - toggle top view camera

### Update

B - Rebuild road (after terrain under it changed, etc.)  
F8 - Update (after changes made on Terrain/Vegetation tab)

### Tools

U - align terrain to road tool (in road edit mode, for selected
segments)  
C - toggle on/off dynamic objects simulation (in objects mode)  
F7 - enter/leave Preview camera

  

### Shortcuts

| Jump directly to Gui tab: |                         |
|---------------------------|-------------------------|
| Alt-I - Input/help        | Alt-D - Terrain Brushes |
| Alt-Q - Track             | Alt-T - Terrain Layers  |
| Alt-E - Settings          | Alt-R - Road            |
|                           | Alt-V - Vegetation      |
|                           | Alt-X - Objects         |

  

------------------------------------------------------------------------

## Approaches

Approaches to making a track:

1.  Make great terrain and place road in it (difficult)
2.  Make cool road and decorate terrain after (easy)
3.  = 1+2 Mixed mode. Edit terrain and road freely.

# Workflow

A list of short steps that describe creating a new track on a high
level:

Save frequently (F4), after each step or change. When made a mistake hit
F5 to reload last track state.

-   1
    -   load an existing track - that has the most of desired features
        already set up (eg. has good terrain layers, vegetation, terrain
        heightmap etc.).
    -   enter new track name, press button: new (duplicate)
-   2
    -   on terrain tab, choose area, make new flat terrain (pick
        heightmap and triangle size) - *when making new terrain shape*
    -   edit terrain (deform up/down or smooth)
    -   it's good to place car start now and test terrain scale in game
        by driving
    -   *optionally* edit terrain layers
-   3
    -   draw initial road shape - inserting road points from top view
    -   edit road points (make bridges,etc), adjust roll angles,
        *optionally* use align terrain to road tool
-   4
    -   setup car start position and its area (in Edit mode press Q and
        Ctrl-LMB to place it at cursor) - after this track can be driven
        in game
    -   choose road direction (+1 or -1), add road checkpoints (spheres,
        chkR), mark 1st checkpoint - after this track timing works
-   5
    -   adjust vegetation amounts (grass, trees)
    -   *optionally* edit vegetation layers and other parameters on
        vegetation tab
    -   enter preview camera and make a track screenshot
    -   drive the track, go back to editor and make changes to road /
        terrain to get the desired diving experience (this is quite
        important and best to repeat it often, starting early and after
        each major change)
