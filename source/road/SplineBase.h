#pragma once
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
public:  // for car checking
	Ogre::Vector3 pos;
	Ogre::Real r,r2;  // radius, r*r
	
	//  for drive progress %
	Ogre::Real dist[2];  // summed distances (cur to next)
		// [0] normal, [1] reversed track
};


class SplineBase
{
public:
	SplineBase();
	~SplineBase();
	friend class CGui;
	

	void clear();
	void addPoint(const Ogre::Vector3& p);
	inline int getNumPoints() const {  return (int)mP.size();  }


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
	
	//  get next, prev points
	inline int getPrev(int id) const {  int s = (int)mP.size();  return isLooped ?  (id-1+s) % s : std::max(0,   id-1);  }
	inline int getNext(int id) const {  int s = (int)mP.size();  return isLooped ?  (id+1) % s   : std::min(s-1, id+1);  }
	//inline int getPrev(int id, int sub=1) const {  int s = (int)mP.size();  return isLooped ?  (id-sub+s) % s : std::max(0, id-sub);    }
	//inline int getNext(int id, int add=1) const {  int s = (int)mP.size();  return isLooped ?  (id+add) % s   : std::min(s-1, id+add);  }


protected:
	bool mAutoCalc, isLooped;  //=closed

	std::deque<SplinePoint> mP;  // points
	static std::deque<SplinePoint> mPc;  // copy points
};
