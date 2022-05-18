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
// 2022 modified by CryHam

#pragma once
#include "BuoyMath.h"
#include <vector>


struct BFace
{
	int i1, i2, i3;

	BFace() : i1(0),i2(0),i3(0)
	{	}
	BFace(int i1, int i2, int i3) : i1(i1), i2(i2), i3(i3)
	{	}
};

struct BPlane
{
	Vec3 normal;
	float offset;
};

struct Polyhedron
{
	std::vector<Vec3> verts;
	std::vector<BFace> faces;
	
	float length = 0.f;
	float volume = 1.f;

	Polyhedron()
	{	}
};

struct WaterVolume
{
	BPlane plane;
	Vec3 velocity;
	float density;
	float linearDrag,linearDrag2;
	float angularDrag;
};

// Compute the volume of the given polyhedron.
float ComputeVolume(Polyhedron& poly);

// Compute the buoyancy and drag forces.
bool ComputeBuoyancy(RigidBody& body, Polyhedron& poly,
					 WaterVolume& water, float gravity);
