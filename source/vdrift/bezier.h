#ifndef _BEZIER_H
#define _BEZIER_H

#include "mathvector.h"
#include "aabb.h"

//#include <fstream>

class AI;
class TRACK;
//class ROADPATCH;

class BEZIER
{
friend class AI;
friend class TRACK;
friend class ROADPATCH;

public:
	BEZIER();
	~BEZIER();
	BEZIER(const BEZIER & other) {CopyFrom(other);}
	BEZIER & operator=(const BEZIER & other) {return CopyFrom(other);}
	BEZIER & CopyFrom(const BEZIER &other);

	float GetDistFromStart() const {return dist_from_start;}
	void ResetDistFromStart() {dist_from_start = 0.0f;}
	void ResetNextPatch() {next_patch = NULL;}

	///initialize this bezier to the quad defined by the given corner points
	void SetFromCorners(const MATHVECTOR<float,3> & fl, const MATHVECTOR<float,3> & fr, const MATHVECTOR<float,3> & bl, const MATHVECTOR<float,3> & br);

	///attach this bezier and the other bezier by moving them and adjusting control points as necessary.
	/// note that the other patch will be modified
	void Attach(BEZIER & other, bool reverse);
	void Attach(BEZIER & other) {Attach(other, false);}

	///return true if the ray starting at the given origin going in the given direction intersects this bezier.
	/// output the contact point and normal to the given outtri and normal variables.
	bool CollideSubDivQuadSimple(const MATHVECTOR<float,3> & origin, const MATHVECTOR<float,3> & direction, MATHVECTOR<float,3> &outtri) const;
	bool CollideSubDivQuadSimpleNorm(const MATHVECTOR<float,3> & origin, const MATHVECTOR<float,3> & direction, MATHVECTOR<float,3> &outtri, MATHVECTOR<float,3> & normal) const;

	///read/write IO operations (ascii format)
	void ReadFrom(std::istream &openfile);
	void WriteTo(std::ostream &openfile) const;

	///flip points on both axes
	void Reverse();

	///a diagnostic function that checks for a twisted bezier.  returns true if there is a problem.
	bool CheckForProblems() const;

	///halve the bezier defined by the given size 4 points4 array into the output size 4 arrays left4 and right4
	void DeCasteljauHalveCurve(MATHVECTOR<float,3> * points4, MATHVECTOR<float,3> * left4, MATHVECTOR<float,3> * right4) const;

	///access corners of the patch (front left, front right, back left, back right)
	const MATHVECTOR<float,3> & GetFL() const {return points[0][0];}
	const MATHVECTOR<float,3> & GetFR() const {return points[0][3];}
	const MATHVECTOR<float,3> & GetBL() const {return points[3][0];}
	const MATHVECTOR<float,3> & GetBR() const {return points[3][3];}

	///get the AABB that encloses this BEZIER
	AABB <float> GetAABB() const;

	///access the bezier points where x = n % 4 and y = n / 4
	const MATHVECTOR<float,3> & operator[](const int n) const
	{
		assert(n < 16);
		int x = n % 4;
		int y = n / 4;
		return points[x][y];
	}

	const MATHVECTOR<float,3> & GetPoint(const unsigned int x, const unsigned int y) const
	{
		assert(x < 4);
		assert(y < 4);
		return points[x][y];
	}

	BEZIER* GetNextPatch() const
	{
		return next_patch;
	}

	MATHVECTOR< float, 3 > GetRacingLine() const
	{
		return racing_line;
	}

	float GetTrackRadius() const
	{
		return track_radius;
	}

	bool HasRacingline() const
	{
		return have_racingline;
	}

private:
	///return the bernstein given the normalized coordinate u (zero to one) and an array of four points p
	MATHVECTOR<float,3> Bernstein(float u, MATHVECTOR<float,3> *p) const;

	///return the bernstein tangent (normal) given the normalized coordinate u (zero to one) and an array of four points p
	MATHVECTOR<float,3> BernsteinTangent(float u, MATHVECTOR<float,3> *p) const;

	///return the 3D point on the bezier surface at the given normalized coordinates px and py
	MATHVECTOR<float,3> SurfCoord(float px, float py) const;

	///return the normal of the bezier surface at the given normalized coordinates px and py
	MATHVECTOR<float,3> SurfNorm(float px, float py) const;

	///return true if the ray at orig with direction dir intersects the given quadrilateral.
	/// also put the collision depth in t and the collision coordinates in u,v
	bool IntersectQuadrilateralF(
		const MATHVECTOR<float,3> & orig,
		const MATHVECTOR<float,3> & dir,
		const MATHVECTOR<float,3> & v_00,
		const MATHVECTOR<float,3> & v_10,
		const MATHVECTOR<float,3> & v_11,
		const MATHVECTOR<float,3> & v_01,
		float &t, float &u, float &v) const;

	MATHVECTOR<float,3> points[4][4];
	MATHVECTOR<float,3> center;
	float radius;
	float length;
	float dist_from_start;

	BEZIER *next_patch;
	float track_radius;
	int turn; //-1 - this is a left turn, +1 - a right turn, 0 - straight
	float track_curvature;
	MATHVECTOR<float,3> racing_line;
	bool have_racingline;
};

std::ostream & operator << (std::ostream &os, const BEZIER & b);

#endif
