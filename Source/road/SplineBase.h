#ifndef _SplineBase_h_
#define _SplineBase_h_

using namespace Ogre;


class SplinePoint
{
public:
	Vector3 pos, tan;  // position, tangent (computed)
	Real width, wtan;  // road width

	Real aYaw,aRoll, tYaw,tRoll;   // yaw,pitch angles+tan
	Real aY,aR;   // yaw,pitch after prepass+

	bool onTer;   // sticked on terrain
	int  cols;    // on/off columns

	Real pipe;    // pipe width
	int idMtr;    // material id road/pipe
	
	Real chkR;    // checkpoint sphere radius (0-none)

	SplinePoint();
	void SetDefault();
};


class CheckSphere
{
public:
	Vector3 pos;
	Real r,r2;  // radius, r*r
};


class SplineBase
{
public:
	SplineBase();
	~SplineBase();


	void clear();
	void addPoint(const Vector3& p);
	int getNumPoints() const;

	const Vector3& getPos(int index) const;
	void setPos(int index, const Vector3& value);


	//  get value over the whole spline
	Vector3 interpolate(Real t) const;
	//  get value at a single segment of the spline
	Vector3 interpolate(int id, Real t) const;

	//  interp 1dim vars
	Real interpWidth(int id, Real t) const;
	Real interpAYaw(int id, Real t) const;
	Real interpARoll(int id, Real t) const;
	
	void recalcTangents(), preAngle(int i);
	void setAutoCalculate(bool autoCalc);
	
	SplinePoint& getPoint(int index);


protected:
	bool mAutoCalc, isLooped;  //=closed

	std::deque<SplinePoint> mP;  // points
	static std::deque<SplinePoint> mPc;  // copy points
};

#endif