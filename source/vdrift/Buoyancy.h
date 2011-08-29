/*
Copyright (c) 2005-2007 Erin Catto http://www.gphysics.com

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BUOYANCY_H
#define BUOYANCY_H

#include "BuoyMath.h"

struct Face
{
	Face() {}
	Face(int i1, int i2, int i3) : i1(i1), i2(i2), i3(i3) {}
	int i1, i2, i3;
};

struct Polyhedron
{
	Vec3* verts;
	Face* faces;
	int numVerts;
	int numFaces;
	float length;
	float volume;
};

struct Plane
{
	Vec3 normal;
	float offset;
};

struct WaterVolume
{
	Plane plane;
	Vec3 velocity;
	float density;
	float linearDrag;
	float angularDrag;
};

// Compute the volume of the given polyhedron.
float ComputeVolume(Polyhedron& poly);

// Compute the buoyancy and drag forces.
void ComputeBuoyancy(RigidBody& body, Polyhedron& poly,
					 WaterVolume& water, float gravity);

#endif