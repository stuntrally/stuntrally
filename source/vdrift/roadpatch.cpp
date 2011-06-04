#include "pch.h"
#include "roadpatch.h"

bool ROADPATCH::Collide(
	const MATHVECTOR <float, 3> & origin,
	const MATHVECTOR <float, 3> & direction,
	float seglen, MATHVECTOR <float, 3> & outtri,
	MATHVECTOR <float, 3> & normal) const
{
	bool col = patch.CollideSubDivQuadSimpleNorm(origin, direction, outtri, normal);
	
	if (col && (outtri - origin).Magnitude() <= seglen)
	{
		return true;
	}
	else
	{
		return false;
	}
}
