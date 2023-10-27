*List and explanation of track editor warning messages.*

### Introduction

Editor does simple checks when saving a track.  
It shows warnings or errors in the Warnings tab (use alt-J to show).  
It also provides some info with values that were checked.  
Usually some short info is shown on what to change to fix them.

Errors are in red, warnings in orange, info in yellow, note in green,
and just text in blue.  
Keep in mind that those are quite simple checks, must be fast to not
delay saving.  
And not always a warning means that there is something wrong with/on
track.  
Also most great looking tracks will have a warning about too many
grasses or vegetation etc.

This list here is a reference with all warnings and has some more info
on how to fix them.  
It also has developer remarks when needing improvement (TODO).

*For reference code see in
[source/editor/Warnings.cpp](https://github.com/stuntrally/stuntrally/blob/master/source/editor/Warnings.cpp)*

### List

#### Start position

<table>
<tbody>
<tr class="odd">
<td>Error</td>
<td>Car start outside track area</td>
</tr>
<tr class="even">
<td>Error</td>
<td>Car start below terrain</td>
</tr>
<tr class="odd">
<td></td>
<td></td>
</tr>
<tr class="even">
<td>Info</td>
<td>Car start far above terrain\n (skip this if on bridge or in pipe), distance</td>
</tr>
<tr class="odd">
<td></td>
<td>other start places inside terrain (split screen)</td>
</tr>
<tr class="even">
<td></td>
<td></td>
</tr>
<tr class="odd">
<td>Warning</td>
<td>Road dir check wrong, road dir is likely opposite</td>
</tr>
<tr class="even">
<td></td>
<td>TODO: throw out dir, use this check for value</td>
</tr>
<tr class="odd">
<td>Warning</td>
<td>Car start isn't facing first checkpoint<br />
(wrong direction or first checkpoint), distance</td>
</tr>
</tbody>
</table>

#### Checkpoints

|         |                                                  |              |
|---------|--------------------------------------------------|--------------|
| Error   | First checkpoint not set (use ctrl-0)            |              |
| Warning | Too small checkpoint at road point               |              |
|         |                                                  |              |
| Error   | No checkpoints set (use K,L on some road points) |              |
| Info    | Too few checkpoints (add more), count            | numChks \< 3 |

#### Road

<table>
<tbody>
<tr class="odd">
<td>Info</td>
<td>HQ Road</td>
<td>mtr &gt;= 3</td>
</tr>
<tr class="even">
<td>Info</td>
<td>Too few road materials used</td>
<td>&lt;= 1</td>
</tr>
<tr class="odd">
<td></td>
<td></td>
<td></td>
</tr>
<tr class="even">
<td>Warning</td>
<td>Car start width small</td>
<td>width &lt; 8 or width &lt; rdW * 1.4</td>
</tr>
<tr class="odd">
<td>Warning</td>
<td>Car start height small</td>
<td>height &lt; 4.5</td>
</tr>
<tr class="even">
<td></td>
<td></td>
<td></td>
</tr>
<tr class="odd">
<td>Warning</td>
<td>Extremely low checkpoints ratio, add more</td>
<td></td>
</tr>
<tr class="even">
<td>Warning</td>
<td>Very few checkpoints ratio, add more</td>
<td></td>
</tr>
<tr class="odd">
<td></td>
<td></td>
<td></td>
</tr>
<tr class="even">
<td>Warning</td>
<td>Road points (on average) are very far<br />
(corners could behave sharp and straights become not straight)</td>
<td>len &gt; 85</td>
</tr>
<tr class="odd">
<td>Info</td>
<td>Road points are far</td>
<td>&gt; 60</td>
</tr>
</tbody>
</table>

|                                                                    |
|--------------------------------------------------------------------|
| Road has over 200 points, use recommended merge length 600 or more |
| Road has over 120 points, use recommended merge length 300 or more |
| Road has over 50 points, use recommended merge length 80 or more   |
| TODO: make this auto, and only some bias 0..2 param                |

#### Terrain

|       |                                                 |            |
|-------|-------------------------------------------------|------------|
| Error | Using too big heightmap 2048, file size is 16MB |            |
| Info  | Using big heightmap 1024, file size is 4MB      |            |
| Info  | Using too small heightmap                       | 128        |
|       |                                                 |            |
| Info  | Terrain triangle size is small                  | \< 0.9     |
| Info  | Terrain triangle size is big                    | \> 1.9     |
|       |                                                 |            |
| Info  | HQ Terrain                                      | layers = 4 |
| Error | Too many terrain layers used, max 4 possible    | \> 4       |
| Info  | Too few terrain layers used                     | \<= 2      |

#### Vegetation

|         |                                                          |              |
|---------|----------------------------------------------------------|--------------|
| Info    | HQ Vegetation                                            | models \>= 5 |
| Warning | Too many models used, not recommended                    | veg \>= 7    |
| Info    | Too few models used                                      | veg \<= 2    |
|         |                                                          |              |
| Error   | Vegetation use is huge, trees density is                 | \> 3.1       |
| Warning | Using a lot of vegetation, trees density is              | \> 2         |
|         |                                                          |              |
| Warning | Smooth grass density is high saving will take long time" | \> 10        |

#### Grass

|         |                                        |              |
|---------|----------------------------------------|--------------|
| Info    | HQ Grass                               | layers \>= 4 |
| Warning | Too many grasses used, not recommended | \>= 5        |
| Info    | Too few grasses used                   | \<= 2        |

#### Total Quality

|      |                                                                                             |         |
|------|---------------------------------------------------------------------------------------------|---------|
|      | hq = hqTerrain + hqGrass + hqVeget + hqRoad                                                 |         |
| Info | Quality too high (possibly low Fps), try to reduce densities or layers/models/grasses count | hq \> 3 |
| Info | Great quality, but don't forget about some optimisations                                    | hq \> 2 |
| Info | Low quality (ignore for deserts), try to add some layers/models/grasses                     | hq = 0  |

### Other

Things that are not checked but should be, TODO:

-   Gui button for a complex (long time) check
-   terrain error vs(and) complexity
-   highest slopes (triplanar) used more than twice, used on low angle
    layers (not needed)
-   blendmap layers coverage (used area %), % of blending between
    (separation), default layer % (no range), noise ..
-   grass dens map % mul by trk grass dens

<!-- -->

-   ! use of vegetation models, real counts. 1 model used few times
    isn't that bad as many models used constantly..
-   static objects count, different (and mtr? pers use 1), (needs
    instancing)
-   paged-geom:? page size small, distance big, old..
