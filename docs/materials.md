*Info on how to add new textures and materials for terrain, road, grass
etc.*

## Introduction

This page is for ver 2.3 and above it explains what to do when adding
new content.  
I.e. where to put your new data and which files need to be edited so it
can appear and look good.

Since ver 2.3 editor uses config/presets.xml. Nothing new will appear in
editor unless you add a line for it in that file.  
It is quite simple, just copy a similar line and add it (keep it
alphabetically sorted). Change the 3rd param to your resource name.

See comments inside presets.xml for (short) info on each param.  
Later you can setup values for parameters, e.g. layer scaling or model
size etc. that are set (by default) when picking (less manual setup
needed).  
Note, for release make sure that data dirs ending with \_s have the new
textures resized for small size.

## Skies

Sky textures are in data/skies.  
Those are 360x90 degree spherical textures. One texture for whole
skydome, size 4096x1024.  
Materials are in data/materials/scene/sky.mat. Have to add new there,
just copy last and replace texture for yours.

## Terrain

Textures for terrain layers are in data/terrain. See about.txt for
sources.  
They are all .jpg saved at about 97%. Size is 1024x1024.

Name endings mean:  
\_d - diffuse texture (main)  
\_s - specular amount (not color)  
if not present, will be black (no specular)  
\_n - normal map (needed, if not provided can be made with some tool)  
You can use flat_n.png before real normalmap for quicker test.  
\_h - height (optional, if none will be white)  
this is only for parallax, and not used now, since it's broken

Unlike other things, terrain has its own material so only adding to
presets.xml is needed.

## Road

Road textures are in data/road.  
Materials in data/materials/scene/road.mat.

When adding a new road material you need to add two materials e.g.
roadJungle_ter and roadJungle. The one with \_ter is for road on
terrain, it has more bumps, and alpha border/texture. The other is for
bridged roads and is more flat, can also use other textures.

Also for editor adding a line in presets.xml is needed.  
Other road materials are in data/materials/scene/pipe.mat: road wall,
pipe, pipe glass, pipe wall, column.

## Grass

Grass textures are in data/grass. They are transparent .png and mostly
512x512.  
Materials in data/materials/scene/grass.mat. Just copy a line and change
texture name (and color if needed).  
Lastly adding a line in presets.xml is needed.

## Vegetation

Models (meshes) are in data/trees.  
Materials in data/materials/scene/trees.mat.

## Objects

Objects have their own wiki page, it also has more info on material
editing (.mat files).
