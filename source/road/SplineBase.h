#ifndef _SplineBase_h_
#define _SplineBase_h_

#include <OgreVector3.h>


enum AngType {  AT_Manual=0, AT_Auto, AT_Both, AT_ALL  };
const static std::string csAngType[AT_ALL] = {"Manual", "Auto", "Both"};

class SplinePoint
{
public:
	Ogre::Vector3 pos, tan;  // position, tangent (computed)
	Ogre::Real width, wtan;  // road width

	Ogre::Real mYaw,mRoll;  // manual angles, if not auto
	AngType aType;
	Ogre::Real aYaw,aRoll;  // working angles (from auto)
	Ogre::Real tYaw,tRoll;  //- angles+tan interp, not used yet
	Ogre::Real aY,aR;   // after prepass+

	bool onTer;   // sticked on terrain
	int  cols;    // on/off columns

	Ogre::Real pipe;    // pipe width
	int idMtr;    // material id road/pipe
	
	Ogre::Real chkR;    // checkpoint sphere radius (0-none)

	SplinePoint();
	void SetDefault();
};


class CheckSphere
{
public:
	Ogre::Vector3 pos;
	Ogre::Real r,r2;  // radius, r*r
};


class SplineBase
{
public:
	SplineBase();
	~SplineBase();


	void clear();
	void addPoint(const Ogre::Vector3& p);
	int getNumPoints() const;

	const Ogre::Vector3& getPos(int index) const;
	void setPos(int index, const Ogre::Vector3& value);


	//  get value over the whole spline
	Ogre::Vector3 interpolate(Ogre::Real t) const;
	//  get value at a single segment of the spline
	Ogre::Vector3 interpolate(int id, Ogre::Real t) const;

	//  interp 1dim vars
	Ogre::Real interpWidth(int id, Ogre::Real t) const;
	Ogre::Real interpAYaw(int id, Ogre::Real t) const;
	Ogre::Real interpARoll(int id, Ogre::Real t) const;
	
	void recalcTangents(), preAngle(int i);
	void setAutoCalculate(bool autoCalc);
	
	SplinePoint& getPoint(int index);


protected:
	bool mAutoCalc, isLooped;  //=closed

	std::deque<SplinePoint> mP;  // points
	static std::deque<SplinePoint> mPc;  // copy points
};

#endif