## What's coming next 3.x - NEW

Stunt Rally 3 is continuing last SR 2.x, using latest Ogre-Next 3.0.  
More info in its [readme](https://github.com/stuntrally/stuntrally3/blob/main/Readme.md), [changelog](https://github.com/stuntrally/stuntrally3/blob/main/docs/Changelog.md) and [documentation](https://github.com/stuntrally/stuntrally3/blob/main/docs/_Menu.md).


------------------------------------------------------------------------

## What's coming next 2.x - OLD

This section lists stuff that is not yet released, but committed to old
[repository](https://github.com/stuntrally/stuntrally/commits/) (and
[tracks](https://github.com/stuntrally/tracks/commits/)).  
Stunt Rally 2.x, using Ogre 1.x and is now mainly meant for bugfixes.
Development continues in Stunt Rally 3.x.

|                 |               |
|-----------------|---------------|
| **Version 2.8** | ?\. ?. 2023 |

-   225 tracks  (**23 New**, 7 renewed, 2 removed)
-   3 New sceneries: Marble, Spring, Anomaly
-   New content for tracks: hexrocks, particles, fluids, dynamic objects etc
-   Editor
    -   Particles scale - new parameter for size e.g. for bigger clouds
        in distance (keys K,L)
-   A couple of fixes
-   Only tinyxml2 used now, no tinyxml

------------------------------------------------------------------------

# Change Log

Info about changes in **released versions**. How did they look like can
be seen in [gallery](https://stuntrally.tuxfamily.org/gallery).

|                 |               |
|-----------------|---------------|
| **Version 2.7** | 11\. 11. 2022 |

-   202 tracks (**34 New**)
    -   130 tracks renewed - added new Rocks, Particles, buildings,
        objects
    -   New Stunt Rally logo, track TestC12-SR
-   **7 New** vehicles
    [topic](https://forum.freegamedev.net/viewtopic.php?f=80&t=18526)
    -   HI Hyena - new default rally car
    -   SX Jaguar - new very fast car, for all tracks
    -   BE Gazelle - normal size bike, fast and agile
    -   U6 Beetle - (too) fast 6 wheeled terrain vehicle
    -   U8 Caterpillar - 8 wheeled terrain vehicle, like a bus
    -   MO Rhino - monster truck, has issues
    -   3B Scorpion - 3 balls futuristic racer, tricky, too wide, can't
        drive in pipes
    -   Vehicle nick names, from animals
        [topic](https://forum.freegamedev.net/viewtopic.php?f=80&t=18531)
-   Game
    -   **Racing line** - new driving aid, trail on road
        -   Changing colors for accelerating (green) and braking (red)
    -   **How to play** - new window with more hints and
        -   Driving school - 6 lessons, replays with subtitles
    -   New 2nd level **main menu** "New game", with game modes and:
        -   Game difficulty (new combo) - it changes:
            -   tracks filter and sorting
            -   also filters game modes lists (tutorials, championships,
                challenges)
            -   default track and vehicle
            -   HUD elements and driving aids
        -   Simulation mode (moved here) - changes:
            -   realism, driving difficulty and damage severity
    -   New rewind mode: go back time (in single race)
    -   HUD, taking damage blinks red, and goes red with more
    -   Smoother play (apart from vegetation lags etc), higher game
        update frequency
    -   Reworked Spaceships anti gravity, less ideal in normal
        simulation mode
-   Graphics
    -   New **rocks** (38 models), few new objects and 0AD buildings
        han\*, maur\*
    -   Particles on tracks: **clouds**, fires, smoke, radioactive etc
    -   Rivers, waterfalls WIP - done as Roads (toggle River alt-Enter
        and not looped N)
    -   Road max visible distance, better Fps on big tracks
    -   Better looking grass wind
-   Editor
    -   Sorted Pick lists, added separators and rating column (higher
        for best and popular)
        -   All Road,Pipe,Wall,Column materials in 1 common list
    -   Insert road point in middle of segment - Alt+Ins (**subdivide**)
    -   Many Roads possible, see TestC11-Forks (but only first road is
        for checkpoints etc)
    -   Not looped roads (N), in Start pos (Q) - Enter toggles between
        start and end place
    -   New mode for Particle emitters - key A
    -   Glass only pipes - Ctrl-9,0 - hides road wall for segment
    -   Mouse edit moves based on distance
-   GUI
    -   Editor: tool windows with added icons, wider, split Help pages,
        etc
    -   Vehicle tab
        -   more columns and stats: rating, difficulty, width
        -   **Drivability %** - current vehicle on track fitness
    -   Track tab
        -   separate driving Advice text (yellow), added more info
        -   description text (green) has general info, location etc
        -   preview and minimap images - click for fullscreen (or key
            Insert)
        -   new icon Sigma, has sum of all difficulties
    -   Tracks Filter window reworked, added icons
    -   3 new **tabs** in game Help
        -   links to websites,
            [Contributing](https://github.com/stuntrally/stuntrally/blob/master/Contributing.md),
            Readme now .md format
        -   Credits, gathering all data txt files with links, authors
            and CC licenses
    -   Moved
        -   Separated SplitScreen tab. No Game tab, moved Boost etc to
            Setup subtab
        -   Vehicle Colors now on 1st Setup subtab, added current color
            box
        -   Multiplayer, split Server (unavailable) to last tab
-   Other
    -   New params in .car and fixes for new vehicles, bigger vehicles
        have more grip
    -   [Conan](https://conan.io/downloads.html) build system added,
        should be easier to build on Windows [PR
        here](https://github.com/stuntrally/stuntrally/pull/39)
    -   sr-translator tool build in CMake, works also on Linux. Added
        support for country language variations
    -   Code cleanup, split big methods, files, use newer C++ auto, for,
        etc
-   Removed old
    -   Replays - Tools tab
    -   2 Super Racing cars, 5 VDrift cars and 4 tracks, VDrift sources
        reduced
-   Known bugs
    -   Screen resolution change now requires restart
    -   All effects are off, broken, tabs invisible
    -   Editor somewhat broken, none or bad vegetation placing.
        -   Due to bug in Ogre. It won't happen if using Ogre
            [13.5.2](https://github.com/OGRECave/ogre/releases/tag/v13.5.2)
            or higher.

  
|                   |               |
|-------------------|---------------|
| **Version 2.6.2** | 25\. 03. 2022 |

-   Just a tag before removing VDrift cars and tracks
-   Save direct connect address & port
-   Fix HUD issue on 4K
-   Code fixes for building etc.

|                   |               |
|-------------------|---------------|
| **Version 2.6.1** | 16\. 03. 2019 |

-   No usual binary releases made, git tag and new Flatpak build
    available for GNU/Linux
    [here](https://flathub.org/apps/details/org.tuxfamily.StuntRally)

<!-- -->

-   Fix editor crashing on any Update (F8)
-   Fixed silent sound for car type camera (e.g. in loops, car bonnet
    etc.)
-   Fixed Sphere O crashing simulation while being in water
-   Faster O steering with handbrake
-   Fixed crash on VDrift tracks
-   Translations update (languages: pl, cs)

|                 |               |
|-----------------|---------------|
| **Version 2.6** | 21\. 09. 2015 |

-   172 tracks (5 New)
-   Vehicles (2 New)
    * 1 new Car: Y7, very fast
    * Motorbikes implementation, 1 new: BV
* 83 Challenges, 65 Championships
* **Pacenotes** - 3D signs above road for:
    * Turns - 7 levels, from easy (green), medium (yellow), square (orange) to harpins and U-turns (red)
    * Long turns - same colors, less saturated
    * Jumps - with required speed text, changes color, best to jump when green or cyan
    * Loops - types: straight, side, barrel, 2 in 1, double, frenzy, ramp
    * On Pipe start and end, Terrain bumps and jumps
* Sound
    * New sound engine in game, using [[https://openal-soft.org/|OpenAL Soft]]
    * [[https://en.wikipedia.org/wiki/Reverberation|Reverberation]] sound effect, changed with scenery
    * Proper 3D distance pan and fade
* Graphics
    * Fog in fluids (underwater effect)
    * Alien buildings updated and new
* Game
    * Track ghosts for reversed direction (for all tracks)
    * New car colors set (now 114), loaded from config/colors.ini
    * Faster Game Reload (F5)
        * If track didn't change, reloads only cars. Shift-F5 forces full reload.
        * Reset (F4) also restores dynamic objects positions
    * Replays and ghosts
        * Reworked file format now 4x smaller size
        * Use button on Replays-Tools tab to Convert all old (takes some time)
        * Supports any number of cars and wheels
        * Vehicle hits dynamic objects in replay (far from ideal)
        * Fixed sphere replay. Fixed Hud progress and damage display
    * Checkpoint Beam working in splitscreen
    * Hidden arrow in replay or splitscreen (not working)
    * Fixed effects sliders update. Fixed motionblur on Windows
* Editor
    * Mark road segments as not real (shift-8), when only for decoration or point move
    * Cycle loop type (key 7, shift - back), for pacenotes (old ChkR mark for camera change)
* New dependencies, if [[compile|compiling]] from sources
    * For sound get [[https://pkgs.org/search/?q=libopenal-dev|libopenal-dev]]
    * Unbundled bullet, get [[https://pkgs.org/search/?q=libbullet-dev|libbullet-dev]], and if present [[https://pkgs.org/search/?q=libbullet-extras-dev|libbullet-extras-dev]]
* Sources
    * Removed log.txt, all now in ogre.log
    * Updated btOgre debug lines, params for quality in code
    * Fixed few memory leaks (on exit), removed not needed source files
* Tweak
    * New Graphs for sounds (no wave osc, not possible) and checks/pacenotes testing
    * Tires tab updated. Included many hard sim tires for tweaking

|     |                                              |
|-----|----------------------------------------------|
|     | [Newer versions continued here](/changelog). |

## Changelog (old)

|                 |               |
|-----------------|---------------|
| **Version 2.5** | 03\. 11. 2014 |

-   167 tracks (20 New)
-   5 New sceneries: Surreal, Stone, Space, Alien, BlackDesert
-   Common

<!-- -->

        * Renamed all tracks with 3 letter prefixes, showing short name in list \\ On Replays tab there is a button to "Rename All Old" replays,ghosts and records
    * Game
        * New challenges and championships
        * Solid fluids (e.g. ice, lava, added in editor fluids mode, but solid and flat)
        * Rewind cooldown, after rewind 1 sec delay until next use is possible
        * Fixed game setup update in multiplayer (before was updated only after track change)
    * GUI - Tracks list
        * Now with short name, difficulty and length ratings (in default short view)
        * Selectable columns, and filtering, button Y, Ctrl-F twice to toggle
    * Common
        * Fixed black terrain on ATI/AMD Radeon cards
        * Many smaller fixes
        * Changed translations, more info for strings, own program to generate .pot, [[https://forum.freegamedev.net/viewtopic.php?f=81&t=5729|topic]], [[Localization|Wiki]]
        * New user log files with errors and warnings only, ogre.err and ogre_ed.err
    * Editor
        * [[https://i.imgur.com/IGHsGQG.jpg|Easier on pipe]] creating, mark (key 8) now also moves pipe down (ctrl-8 the old way)
        * On pipe sections now marked on minimap as orange
        * Up/down keys working in pick window and objects lists, buildings groups list
        * Checkbox to ignore all "Wrong checkpoint" messages on track (for curly tracks where checks overlap road)
        * Better checkpoints in pipe, now in center and radius 1
    * Tweak
        * Reference graph for tires, loading tires
    * Sources
        * Reworked Road_Rebuild.cpp, cleaner, data stucts, split to Road_Prepass.cpp
        * Split common .cfg to settings_com.h and cpp
        * specMap_rgb option in .mat (for rgb only specular maps)
        * Check if tracks and cars exist before replay load
        * Fixed most CppCheck warnings

|                 |               |
|-----------------|---------------|
| **Version 2.4** | 09\. 08. 2014 |

-   147 tracks (6 new, 12 old deleted, some renewed)
-   2 New sceneries: Crystals, GreeceWhite
-   Cars

<!-- -->

        * 3 New: TU, SZ, FN (old FM)
        * 4 renewed: XZ, LK4, S1, UV
          * better materials, less submeshes, baked AO
        * 2 old deleted: XM, NS
    * Simulation
        * Spaceship hovercrafts, Not drivable in pipes
          * 3 New: V1, V2, V3 in cars list
        * Sphere, O in cars list, Too bouncy
    * Common
        * New sky textures, on half of tracks
        * New static objects on few tracks
        * Dropped SceneryID, each track has own cache
    * Gui
        * [[https://i.imgur.com/DkuMS6p.jpg|Car tab]] with bars for stats, speed graph, short list view
        * Split, to new Graphics and Tweak tabs
        * Fonts bigger and resized with resolution
    * Game
        * [[https://i.imgur.com/jaRlksI.jpg|Welcome screen]] with game Hints, shown on first run
        * Sounds for win, loose, lap, best time, wrong checkpoint
        * Fixed multiplayer ([[https://forum.freegamedev.net/viewtopic.php?f=78&t=5650#p57946|nick appeared twice]])
        * Damage from terrain, height fog, fluids on few tracks
        * Possible to have different road surfaces on 1 track
    * Editor
        * [[https://forum.freegamedev.net/viewtopic.php?f=79&t=5581#p57754|Color pick window]] with H,S,V sliders (for light,fog,trail colors)
        * Mode is now an image (not text Cam,Edit,Gui)
        * Pick window for skies, sky rotation
        * Height brush, Space - pick height from cursor
        * Fixed edit mouse move crash
    * Tweak
        * Linear speed sensitive steering, [[http://imgur.com/1r4AH0Y|graph on Gui]]
        * New graph (visualization) to help developing tire parameters
        * Edit surfaces in game's car tweak window

|                 |               |
|-----------------|---------------|
| **Version 2.3** | 11\. 05. 2014 |

-   153 tracks (7 new, 27 renamed)
-   Common

<!-- -->

        * All tracks renewed, new look
        * Terrain
          * New textures, bigger 1k, with CC 0 license, more info in [[https://github.com/stuntrally/stuntrally/blob/master/data/terrain/about.txt|data/terrain/about.txt]]
          * Blendmap noise with many parameters
          * Fixed triplanar uv swap, also now max 2 layers can be picked
          * Emissive light, on few tracks
        * Grass
          * Channels - different setups for grasses (e.g. white grass in higher mountains)
          * Grass density is a RenderToTexture
        * Tracks tab: list sorting fix, more scenery colors, icons hiding, start position
        * Frames per second limitting option
        * Disabled parallax, broken
        * Updated Compiling Wiki, made Windows pre-built dependencies archive
    * Game
        * Dynamic camera bouncing, camera view angle sliders
        * Boost fuel depending on track length, more boost options
    * Editor
        * Pick window for terrain textures, grasses and vegetation models
          * Tab key show/hide, bigger list, mouse wheel on button picks next/previous
          * Fill settings (marked with .) from presets.xml (optional)
        * Game tab with track settings: gravity, wind, deny reversed, etc.
        * Terrain
          * Test blendmap F9
          * Faster terrain editing, blendmap is now a RTT with shader
          * Swap layer buttons
          * Enter - lock brush position (use for big brushes to avoid blur)
        * Fixed selected objects rotation (many) and copy
        * Start position rotation also global and roll
        * Update button on Layers, Grasses, Vegetation for faster update (same for F8)
        * Vegetation model info: count and real sizes [m]
        * Surface tab for terrain and road surface params (split from Layers tab)
        * Test SceneryID button, shows % difference

|                   |               |
|-------------------|---------------|
| **Version 2.2.1** | 28\. 11. 2013 |

-   Fixed crash, happened in track loading, for tracks that have pipe
    transition segments.
-   Updated Italian translation

|                 |               |
|-----------------|---------------|
| **Version 2.2** | 25\. 11. 2013 |

-   146 tracks (19 new)
-   Game

<!-- -->

        * Challenges (new game mode)
          * Short series of tracks. Only the best drivers will succeed. \\ Drive with no mistakes to win bronze,silver or gold prize. \\ Normal simulation, no driving aids.
        * Multiplayer updated
          * Host can press new game and continue on other track (no need to quit and create)
          * Game info text, track info now on track tab
        * Hud
          * New layout (gauges on right, minimap on left)
          * Times bar shows current points and time difference each checkpoint
          * Lap result window (last, best times, final points) on right
        * Input checkbox to bind 1 axis to both throttle and brake
        * Rocks with better collision (triangle mesh)
        * Better water and mud particles
        * Damage repair each lap option
        * Auto camera change in loops (by marking checkpoints in editor, key 7)
    * Common
        * Gui reworked, more icons, also on Input tab
        * Graphics presets now on Screen tab (need to quit after change)
        * All sliders have default value, press RMB on slider to reset
    * Other
        * New objects for caves, pipe obstacles, column for rocks
        * New loading screens
        * Tire force circles visualization
        * Separated differential settings in .car, center of mass length offset, better TW handling
        * Road total on-pipe percentage (need to mark segments in editor, key 8)
        * Road stats banked angle average and max
    * Editor
        * Some edit fields now also with slider and sliders with edits
        * Checkpoint radius restrictions (1 to 2.5 on terrain, constant on bridges and pipes), force with ctrl
        * Objects
          * Selection mark (glow)
          * Better mouse picking
          * Selection scale, rotate (only simple works)
        * Gui revamp
          * Split tabs into Track window, Grasses tab, Settings on 3 tabs
        * Change in road point keys
          * 1,2 Roll, 3,4 Yaw, alt-1,2 angle type, 5,6 snap, alt-shift-1 zero angles
    * Fixes
        * Optimization, less batches for Hud, opponents list hidden by default
        * Fixed broken translations (wrong check, loading etc.)
        * Fixed input on gui edits (ctrl is stuck, press it to fix) in chat after network game or in find box
        * Linux release archive was outputting user data into wrong location (was in .local/data)
        * Fixed minimap road preview save in editor (shader errors)
        * No simulation before game start (fix multiplayer jumps, but shows car landing more)
    * Sources
        * Huge changes in headers and cpp, split to more (CGame,CHud,CGui,CApp), fixed includes
        * Gui Init code shorter, grouped controls in CGui.h, common Gui code in GuiCom class
        * Using own classes SliderValue and Check

|                 |               |
|-----------------|---------------|
| **Version 2.1** | 07\. 08. 2013 |

-   127 tracks (11 new)
-   5 new cars: UV, HR, OT, FR4, TW
-   Game

<!-- -->

        * Input reimplemented with SDL2 (shoud fix input issues), using OICS (not OIS and OISB)
        * Simulation fix (steering, too big car inertia, updated aerodynamics)
        * Next checkpoint beam
        * Other car ghost (when current unavailable)
        * Track's ghost (best drive for all tracks) (green ES car, from normal simulation mode)
        * Simple car damage
          * damage % indicator, no visual changes yet
          * decreased performance when over 50 %, car destroyed at 100 %
        * Reworked scoring, now points and race position
        * Brake lights flares
        * New championships and tutorials, higher pass score, reverse option
        * Steering range sliders on Gui tab setup
        * Adaptive fov effect when boosting
    * Gui
        * Gui rework, added icons to main tabs, all tab colors
        * Car paint reflectiveness, new slider
        * Car tab shows car speed and type
        * Track tab shows track time for current car, points
        * Championship groups, info text, times in list
        * Sections in .car editor
    * Editor
        * Terrain brushes
          * new presets (87 total)
          * new functions: Noise2 (inversed), N-gon (regular polygon)
        * Terrain generator
          * preview image (green up, blue down)
          * add/substract/multiply to current terrain
          * omit road possible
        * Context help for current mode Ctrl-F1
        * Top view toggle Alt-Z
        * Warnings tab showing track errors,warnings and hints after save
        * Road mirror selected segments Alt-Home
        * Rearranged keys (-= ;' ,. now also on 9,0 O,P K,L N,M)
        * Using key scan codes (regional keyboards shouldn't matter)
    * Fixes
        * Fix editor crash on align road U and simulate objects C
        * Fix auto gearbox change oscillations
        * Fix car particles emit on pause
        * cg is now not required to run
        * Small fixes in input bind Gui

|                 |               |
|-----------------|---------------|
| **Version 2.0** | 06\. 05. 2013 |

-   116 tracks (5 new, 18 renewed)
-   4 new cars: N1, S8, XZ, LK4, also renewed 3S and some wheels
-   Updated wiki pages with navigation and new car pages
-   Graphics

<!-- -->

        * New pines and rocks in winter
        * Height fog and 2 color fog on some tracks
        * Better materials, specular on glass pipes, objects
        * Blending between road materials
        * Terrain triplanar 1 layer option (more Fps than full)
        * 2 more graphics presets
    * Game
        * Car body glossiness, new slider
        * Grass deform under car
        * Minimap border option
        * Rewind ghost
        * Camera distance ray (shortens when near mountains)
    * Fixes
        * No memory leak (crashed after some New Games)
        * Optimisations for batch count, more Fps on big tracks
        * Proper start after loading (no camera jump, good reflections, preloaded impostors)
        * Fixed crashes, VDrift splitscreen, replay load, championship stage end
    * Editor
        * 3D preview for vegetation models and objects
        * Objects editing improvements
        * Fixed align terrain to road issue with objects and fluids sizing
        * Default terrain error for track on HMap tab (more for high terrains)
        * Triplanar 1 checkbox on Layer tab (to mark high mountains)
        * Road wall, column materials, scale multipliers

|                 |               |
|-----------------|---------------|
| **Version 1.9** | 15\. 01. 2013 |

-   111 tracks (7 new)
-   All tracks renewed
-   2 new sceneries: Autumn and Moss (mossy jungle)
-   Simulation

<!-- -->

        * Easy (beginner) and Normal (simulation) modes
        * Simulation fixes (Big changes):
          * New Tires - more grip driving and more control when sliding
          * Torque curve - fixed unrealistic behaviour at low rpm
          * No control in air after jump (only by flip)
          * Flip car speed and boost reduced
          * Suspension - soft for terrain, hard in jumps and pipes
          * Downforce - better handling at high speed
          * Proper center of mass, more mass
        * Car performance statistics on Gui (acceleration times, engine power etc.)
        * New graphs in game (for torque curves, power, clutch, diffs)
    * Graphics
        * New grasses on all tracks
        * New trees (generated with SnappyTree)
        * Decreased shadows options - better Fps
        * More trees 1,5x, grass range 2x
        * Option for Impostors Only (low quality trees but fast smooth fps)
        * Fixed black terrain issue on some cards
    * Editor
        * Grass layers
        * Surface combobox with presets (no changing all params)
        * Alt-1,2 prev/next layer tab
        * Fixed car start box, visible now

|                 |               |
|-----------------|---------------|
| **Version 1.8** | 27\. 10. 2012 |

-   104 tracks (5 new)
-   Graphics

<!-- -->

        * Using new materials generator: [[https://github.com/scrawl/shiny|shiny]]
        * New water and mud looks
        * Triplanar terrain option (high slopes with good texturing)
        * Fixed crash on amd and intel cards
    * Editor
        * Tweak page (in Options) for adjusting material parameters (for developers)
    * Game
        * New car S1
        * Fixed replay load crash

|                 |               |
|-----------------|---------------|
| **Version 1.7** | 02\. 09. 2012 |

-   99 tracks (12 new, 20 old renewed, with 8 VDrift tracks)
-   Gameplay

<!-- -->

        * Rewind - lets you take your car back in time after crash, spin out etc (default key: Backspace (hold)).
        * VDrift tracks (with all game features)
        * Static and Dynamic objects on tracks (barrels, boxes, buildings etc)
        * 3D sounds, new car hit sounds - scrap and screech
    * Simulation
        * Asphalt tires (on VDrift tracks)
        * Fixed all cars parameters
    * Editor
        * New video tutorial (10 chapters)
        * Align terrain to road tool (for selected segments, key U)
        * Terrain brush presets (quick change)
        * Objects inserting and simulating
        * Auto update blendmap
        * Color images (on Sun tab and road trail)
        * Weather particles
    * Tools in game
        * Graphs (for car values debug, eg. tires slip, suspension, sound)
        * Tires editing (Pacejka coefficients)
    * Graphics
        * Shadow fixes (now on road wall and columns, shadowed road bug in DirectX)
        * God Rays effect (sun glare)
        * HDR, HQ Bloom and Vignetting (beta, too saturated)
        * Motion blur broken (on GL)

|                 |               |
|-----------------|---------------|
| **Version 1.6** | 19\. 04. 2012 |

-   79 tracks (9 new)
-   Gameplay

<!-- -->

        * Camera auto tilt on slopes
        * 5 new gauge types (for rpm and velocity)
    * Editor
        * Filter brush (key F), filters terrain bumps
    * Bug fixes
        * split screen game crash from more to less players
        * car hood camera jumps
        * grass bad looking issue with effects
        * track percent for ghost and in replay

|                 |               |
|-----------------|---------------|
| **Version 1.5** | 08\. 04. 2012 |

-   70 tracks (+5 new, 5 renewed, -6 old deleted)
-   Gameplay

<!-- -->

        * Championships and tutorials (game mode to drive series of tracks for score)
        * Multiplayer (beta)
        * Keyboard sensitivity and return speed, also presets (slow,medium,fast)
        * Multi-threaded game (smoother play at lower fps)
    * Game main menu, new help screen
    * Graphics
        * Water reflection and refraction (screen effect), smooth transition to terrain
        * New screen effects: Soft particles, Depth of field
        * New HUD code (gauges, minimap work in split screen and with effects)
    * Replays
        * Fluid particles and sounds, hit sparks saved in replays
        * Viewing replays for more players with fewer viewports, changing player
    * Editor
        * Fixed surfaces saving/editing bug

|                 |               |
|-----------------|---------------|
| **Version 1.4** | 04\. 01. 2012 |

-   71 tracks
-   2 new cars TC6, NS
-   Graphics

<!-- -->

        * Proper shadows (depth/self shadows, terrain lightmap on car, road, trees...)
        * Improved shader effects (powered by a dynamic material & shader generator)
        * Car fresnel effect, reflectivity/specular maps
        * New effects "Screen Space Ambient Occlusion" and Antialiasing SSAA
    * Gameplay
        * Mud (on tracks J14-Muddy, D9-Mud)
        * F12: Go to last checkpoint, F4: Restart race (without loading)
        * New sounds (boost, water, mud) and particles
        * Track completion percentage indicator
        * Distance to other cars / ghost indicator
        * All cars / ghost on minimap (partial)
        * Improved in-car camera
        * Rear gear throttle-brake inverse option
    * Performance
        * Car paint color instant change
        * Option to turn off impostors (2D trees in the distance)
        * Customizable shader quality
        * Newer Paged Geometry version (faster vegetation)
    * Editor
        * Fluids editing (water, mud areas)
    * Other
        * Now using MyGUI3.2 (MyGUI 3.0 will NOT work anymore)

|                 |               |
|-----------------|---------------|
| **Version 1.3** | 19\. 10. 2011 |

-   Terrain blendmap from angles and height
-   Tracks info icons
-   Glass material fixed (now proper, not add)
-   Editor

<!-- -->

        * auto yaw angle (for road points)
        * new brush types: noise, set height
        * brush shapes (sinus, triangle, noise)
        * brush preview image
    * Minimap circle (zoomed and rotated minimap)
    * Water (1st version)
    * New crash sounds and sparks
    * Graphics presets (low,med,high..)
    * Game controllers support
    * Checkpoint arrow (shows direction)
    * Improved motion blur

|                 |               |
|-----------------|---------------|
| **Version 1.2** | 31\. 07. 2011 |

-   Replays
-   Ghost (best lap (or last) drive)
-   Split screen, for 2-4 players
-   Localization (translated to en,de,pl)
-   Keys rebinding, using oisb

|                   |               |
|-------------------|---------------|
| **Version 1.1.1** | 05\. 04. 2011 |

-   bugfixes
-   optimized tree collisions

|                 |               |
|-----------------|---------------|
| **Version 1.1** | 02\. 04. 2011 |

-   Linux (package for Ubuntu, sources compilable)
-   Using CMake (build system)
-   User config files (in user folder)
-   Tree collisions
-   Gui improvements
-   Effects (motion blur, bloom, hdr)
-   Screen tab (resolution)
-   New camera type (ext ang)

|                   |               |                                         |
|-------------------|---------------|-----------------------------------------|
| **Version 1.0.1** | 08\. 01. 2011 | update, fixes in editor, tutorial track |
| **Version 1.0**   | 02\. 01. 2011 | fully functional Track Editor           |
| **Version 0.9**   | 12\. 12. 2010 | track timing with records               |
| **Version 0.8**   | 28\. 11. 2010 | road with alpha, terrain physics        |
| **Version 0.7**   | 23\. 11. 2010 | car coloring, road pipes                |
| **Version 0.6**   | 15\. 11. 2010 | first Track Editor, many fixes          |
| **Version 0.5**   | 19\. 09. 2010 | materials, particles, desert yellow     |
| **Version 0.4**   | 10\. 08. 2010 | test Loops track                        |
| **Version 0.3**   | 31\. 07. 2010 | roads, 3 test tracks, minimap           |
| **Version 0.2**   | 06\. 06. 2010 | OGRE terrain, vegeatation, no roads     |
| **Version 0.1**   | 11\. 04. 2010 | initial, only VDrift track, no gui      |
