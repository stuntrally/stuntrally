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

#include "pch.h"
#include "Buoyancy.h"
#include <assert.h>


// Returns the volume of a tetrahedron and updates the centroid accumulator.
static float TetrahedronVolume(Vec3& c, Vec3 p, Vec3 v1, Vec3 v2, Vec3 v3)
{
	Vec3 a = v2 - v1;
	Vec3 b = v3 - v1;
	Vec3 r = p - v1;

	float volume = (1.0f / 6.0f) * (b % a) * r;
	c += 0.25f * volume * (v1 + v2 + v3 + p);
	return volume;
}


// Clips a partially submerged triangle and returns the volume of the
// resulting tetrahedrons and updates the centroid accumulator.
static float ClipTriangle(	Vec3& c, Vec3 p,
							Vec3 v1, Vec3 v2, Vec3 v3,
							float d1, float d2, float d3)
{
	assert(d1*d2 < 0);
	Vec3 vc1 = v1 + (d1 / (d1 - d2)) * (v2 - v1);
	float volume = 0;

	if (d1 < 0)
	{	if (d3 < 0)
		{	// Case B - a quadrilateral or two triangles.
			Vec3 vc2 = v2 + (d2 / (d2 - d3)) * (v3 - v2);
			volume += TetrahedronVolume(c, p, vc1, vc2, v1);
			volume += TetrahedronVolume(c, p, vc2, v3, v1);
		}else{
			// Case A - a single triangle.
			Vec3 vc2 = v1 + (d1 / (d1 - d3)) * (v3 - v1);
			volume += TetrahedronVolume(c, p, vc1, vc2, v1);
		}
	}else{
		if (d3 < 0)
		{	// Case B
			Vec3 vc2 = v1 + (d1 / (d1 - d3)) * (v3 - v1);
			volume += TetrahedronVolume(c, p, vc1, v2, v3);
			volume += TetrahedronVolume(c, p, vc1, v3, vc2);
		}else{
			// Case A
			Vec3 vc2 = v2 + (d2 / (d2 - d3)) * (v3 - v2);
			volume += TetrahedronVolume(c, p, vc1, v2, vc2);
		}
	}
	return volume;
}


// Computes the submerged volume and center of buoyancy of a polyhedron with
// the water surface defined as a plane.
static float SubmergedVolume(Vec3& c, Vec3 x, Quat q,
							Polyhedron& poly, BPlane& plane)
{
	// Transform the plane into the polyhedron frame.
	Quat qt = q.Conjugate();
	Vec3 normal = qt.Rotate(plane.normal);
	float offset = plane.offset - plane.normal*x;

	// Compute the vertex heights relative to the surface.
	float TINY_DEPTH = -1e-6f;
	std::vector<float> ds;  ds.reserve(poly.verts.size());

	// Compute the depth of each vertex.
	int numSubmerged = 0;
	int sampleVert = 0;
	for (int i = 0; i < poly.verts.size(); ++i)
	{
		ds.push_back(normal * poly.verts[i] - offset);
		if (ds[i] < TINY_DEPTH)
		{
			++numSubmerged;
			sampleVert = i;
		}
	}

	// Return early if no vertices are submerged
	if (numSubmerged == 0)
	{
		c.SetZero();
		return 0;
	}

	// Find a point on the water surface. Project a submerged point to
	// get improved accuracy. This point serves as the point of origin for
	// computing all the tetrahedron volumes. Since this point is on the
	// surface, all of the surface faces get zero volume tetrahedrons. This
	// way the surface polygon does not need to be considered.
	Vec3 p = poly.verts[sampleVert] - ds[sampleVert] * normal;

	// Initialize volume and centroid accumulators.
	float volume = 0;
	c.SetZero();

	// Compute the contribution of each triangle.
	for (int i = 0; i < poly.faces.size(); ++i)
	{
		int i1 = poly.faces[i].i1;
		int i2 = poly.faces[i].i2;
		int i3 = poly.faces[i].i3;

		Vec3 v1 = poly.verts[i1];
		float d1 = ds[i1];

		Vec3 v2 = poly.verts[i2];
		float d2 = ds[i2];

		Vec3 v3 = poly.verts[i3];
		float d3 = ds[i3];

		if (d1 * d2 < 0)
		{
			// v1-v2 crosses the plane
			volume += ClipTriangle(c, p, v1, v2, v3, d1, d2, d3);
		}
		else if (d1 * d3 < 0)
		{
			// v1-v3 crosses the plane
			volume += ClipTriangle(c, p, v3, v1, v2, d3, d1, d2);
		}
		else if (d2 * d3 < 0)
		{
			// v2-v3 crosses the plane
			volume += ClipTriangle(c, p, v2, v3, v1, d2, d3, d1);
		}
		else if (d1 < 0 || d2 < 0 || d3 < 0)
		{
			// fully submerged
			volume += TetrahedronVolume(c, p, v1, v2, v3);
		}
	}

	// Small submerged slivers may have roundoff error leading to a zero or negative
	// volume. If so, then return a result of zero.
	float TINY_VOLUME = 1e-6f;
	if (volume <= TINY_VOLUME)
	{
		c.SetZero();
		return 0;
	}

	// Normalize the centroid by the total volume.
	c *= 1.0f / volume;

	// Transform the centroid into world coordinates.
	c = x + q.Rotate(c);

	return volume;
}


float ComputeVolume(Polyhedron& poly)
{
	float volume = 0;
	Vec3 c, zero;
	c.SetZero();
	zero.SetZero();

	// Compute the contribution of each triangle.
	for (int i = 0; i < poly.faces.size(); ++i)
	{
		int i1 = poly.faces[i].i1;
		int i2 = poly.faces[i].i2;
		int i3 = poly.faces[i].i3;

		Vec3 v1 = poly.verts[i1];
		Vec3 v2 = poly.verts[i2];
		Vec3 v3 = poly.verts[i3];

		volume += TetrahedronVolume(c, zero, v1, v2, v3);
	}
	return volume;
}


// Compute the buoyancy and drag forces.
bool ComputeBuoyancy(RigidBody& body, Polyhedron& poly,
					 WaterVolume& water, float gravity)
{
	Vec3 c;
	float volume = SubmergedVolume(c, body.x, body.q, poly, water.plane);

	if (volume <= 0)
		return false;

	Vec3 buoyancyForce = (water.density * volume * gravity) * water.plane.normal;

	float partialMass = body.mass * volume / poly.volume;
	Vec3 rc = c - body.x;
	Vec3 vc = body.v + body.omega % rc;
	Vec3 dragForce = (partialMass * water.linearDrag) * (water.velocity - vc);

	Vec3 totalForce = buoyancyForce + dragForce;
	body.F += totalForce;
	if (water.linearDrag2 > 0.001f)
	{
		Vec3 vc2 = Vec3(-body.v.x * body.v.x, -body.v.y * body.v.y, -body.v.z * body.v.z);
		body.F += (volume / poly.volume * water.linearDrag2) * vc2;
	}
	body.T += rc % totalForce;

	float length2 = poly.length * poly.length;
	Vec3 dragTorque = (-partialMass * water.angularDrag * length2) * body.omega;
	body.T += dragTorque;

	return true;
}
