#ifndef _COLLISION_CONTACT_H
#define _COLLISION_CONTACT_H

#include "tracksurface.h"
class BEZIER;
class btCollisionObject;

class COLLISION_CONTACT
{
private:
	MATHVECTOR <float,3> position, normal;
	float depth;
	const TRACKSURFACE * surface;
	const BEZIER * patch;
	const btCollisionObject * col;
public:
	COLLISION_CONTACT() : depth(0), surface(TRACKSURFACE::None()), patch(NULL), col(NULL)
	{	}
	
	const MATHVECTOR <float,3> & GetPosition() const	{	return position;	}
	const MATHVECTOR <float,3> & GetNormal() const		{	return normal;		}
	float GetDepth() const								{	return depth;	}
		
	const TRACKSURFACE * GetSurfacePtr() const	{	return surface;		}
	const TRACKSURFACE & GetSurface() const		{	return *surface;	}
	
	const BEZIER * GetPatch() const				{	return patch;	}
	const btCollisionObject * GetObject() const	{	return col;	}

	
	//  set contact data  (used by ray cast)
	void Set(const MATHVECTOR <float,3> & p, const MATHVECTOR <float,3> & n, float d,
		const TRACKSURFACE * s,	const BEZIER * b, const btCollisionObject * c)
	{
		assert(s != NULL);
		position = p;  normal = n;  depth = d;
		patch = b;  surface = s;  col = c;
	}
	
	//  update/interpolate contact
	bool CastRay(const MATHVECTOR <float,3> & origin, const MATHVECTOR <float,3> & direction, float length)
	{
		// plane-based approximation
		float nd = normal.dot(direction);
		if (nd < 0)
		{
			depth = normal.dot(position - origin) / nd;
			position = origin + direction * depth;
			return true;
		}
		position = origin + direction * length;
		depth = length; 
		return false;
	}
};

#endif // _COLLISION_CONTACT_H
