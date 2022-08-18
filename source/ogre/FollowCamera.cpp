#include "pch.h"
#include "../vdrift/par.h"
#include "common/Def_Str.h"
#include "FollowCamera.h"
#include "../settings.h"

#include "tinyxml2.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/mathvector.h"
// ray cast
#include "../vdrift/collision_contact.h"
#include "../vdrift/collision_world.h"
#include "../btOgre/BtOgreDebug.h"
#include "btBulletCollisionCommon.h"
#include "CarPosInfo.h"

#include <Ogre.h>
// #include <OgreCamera.h>
// #include <OgreSceneNode.h>
// #include <OgreTerrainGroup.h>
// #include <OgreVector3.h>

#if OGRE_VERSION_MAJOR >= 13
#include <OgreDeprecated.h>
#endif

#include <MyGUI.h>
using namespace Ogre;
using namespace tinyxml2;


static float GetAngle(float x, float y)
{
	if (x == 0.f && y == 0.f)
		return 0.f;

	if (y == 0.f)
		return (x < 0.f) ? PI_d : 0.f;
	else
		return (y < 0.f) ? atan2f(-y, x) : (-atan2f(y, x));
}

///  Update
//-----------------------------------------------------------------------------------------------------

void FollowCamera::update(Real time, const PosInfo& posIn, PosInfo* posOut, COLLISION_WORLD* world, bool bounce, bool sphere)
{
	if (!ca || !posOut)  return;

	///  input from car posInfoIn
	Vector3 posGoal = posIn.pos;
	Quaternion orientGoal = posIn.rot;
	///  output saved back to car posInfoOut
	Quaternion camRotFinal;

	const static Quaternion
		qOrFix = Quaternion(Degree(180), Vector3::UNIT_Z) * Quaternion(Degree(-90), Vector3::UNIT_Y),
		qOrRot = Quaternion(Degree(90), Vector3::UNIT_Y),
		qTop = Quaternion(Degree(-90), Vector3::UNIT_X), qSphFix = Quaternion(Degree(180), Vector3::UNIT_Z);

	Quaternion  orient = orientGoal * qOrFix;
	Vector3  caOfs = (ca->mType == CAM_Car && sphere ? Vector3(-ca->mOffset.x, -ca->mOffset.y, ca->mOffset.z) : ca->mOffset);
	Vector3  ofs = orient * caOfs, 	goalLook = posGoal + ofs;

	first = iFirst < 2;  ///par few first frames after reset
	if (iFirst < 10)  // after reset
	{	++iFirst;
		mDistReduce = 0.f;  mATilt = 0.f;
	#if 0  // todo: during loading ..
		camPosFinal = Vector3(0, iFirst * 200, 0);  // make evth visible, load mesh,tex etc
		camRotFinal = qTop;

		posOut->camPos = camPosFinal;
		posOut->camRot = camRotFinal;
		return;
	#endif
	}

	///  Camera Tilt from terrain/road slope under car
	//-------------------------------------------------------------------------------------------
	const float			//  params  . . .
		Rdist = 1.f,     // dist from car to ray (front or back)
		HupCar = 1.5f,	  // car up dir dist - for pipes - so pos stays inside pipe
		Habove = 1.5f,    // up axis dist, above car - for very high terrain angles
		HMaxDepth = 12.f;  // how far down can the ray goes
	const static Radian r0(0.f),
		angMin = Degree(10.f),   // below this angle, tilt has no effect - terrain bumps
		maxDiff = Degree(1.4f);  // max diff of tilt - no sudden jumps
	const float smoothSpeed = 14.f;  // how fast to apply tilt change

	bool bUseTilt = ca->mType == CAM_ExtAng || ca->mType == CAM_Follow;
	Radian tilt(0.f);
	if (pSet->cam_tilt && bUseTilt)
	{
		//  car pos
		Vector3 carUp = posIn.pos - HupCar * posIn.carY;
		MATHVECTOR<float,3> pos(carUp.x, -carUp.z, carUp.y + Habove);  // to vdr/blt
		const static MATHVECTOR<float,3> dir(0,0,-1);  // cast rays down
		
		//  car rot, yaw angle
		Quaternion q = posIn.rot * Quaternion(Degree(90),Vector3(0,1,0));
		float angCarY = q.getYaw().valueRadians() + PI_d/2.f;
		float ax = cosf(angCarY)*Rdist, ay = sinf(angCarY)*Rdist;
		//LogO("pos: "+fToStr(pos[0],2,4)+" "+fToStr(pos[1],2,4)+"  a: "+fToStr(angCarY,2,4)+"  dir: "+fToStr(ax,2,4)+" "+fToStr(ay,2,4));
		
		//  cast 2 rays - 2 times, average 2 angles
		COLLISION_CONTACT ct0,ct1,ct20,ct21;
		MATHVECTOR<float,3> ofs(ax*0.5f,ay*0.5f,0),ofs2(ax,ay,0);
		world->CastRay(pos+ofs, dir, HMaxDepth,chassis, ct0,  0,0, true, true);
		world->CastRay(pos-ofs, dir, HMaxDepth,chassis, ct1,  0,0, true, true);
		world->CastRay(pos+ofs2,dir, HMaxDepth,chassis, ct20, 0,0, true, true);
		world->CastRay(pos-ofs2,dir, HMaxDepth,chassis, ct21, 0,0, true, true);

		#ifdef CAM_TILT_DBG
			MATHVECTOR<float,3> v;
			v = pos+ofs;  posHit[0] = Vector3(v[0],v[2]- ct0.GetDepth(), -v[1]);
			v = pos-ofs;  posHit[1] = Vector3(v[0],v[2]- ct1.GetDepth(), -v[1]);
			v = pos+ofs2; posHit[2] = Vector3(v[0],v[2]- ct20.GetDepth(),-v[1]); 
			v = pos-ofs2; posHit[3] = Vector3(v[0],v[2]- ct21.GetDepth(),-v[1]);
		#endif

		if (ct0.GetColObj() && ct1.GetColObj() && ct20.GetColObj() && ct21.GetColObj() )
			tilt = (GetAngle(Rdist, ct1.GetDepth() - ct0.GetDepth()) +
				GetAngle(2.f*Rdist, ct21.GetDepth() - ct20.GetDepth())) * 0.5f;
		//else  LogO(String("no hit: ")+(ct0.col?"1":"0")+(ct1.col?" 1":" 0"));

		//if (tilt < angMin && tilt > -angMin)  tilt = 0.f;
		if (tilt < r0 && tilt >-angMin) {  Radian d = tilt-angMin;  tilt = std::min(r0, tilt + d*d*5.f);  }
		if (tilt > r0 && tilt < angMin) {  Radian d =-angMin-tilt;  tilt = std::max(r0, tilt - d*d*5.f);  }

		//LogO("a "+fToStr(angCarY,3,5)+" d  "+fToStr(ct0.GetDepth(),3,5)+" "+fToStr(ct1.GetDepth(),3,5)+"  t "+fToStr(tilt.valueDegrees(),3,5));
	}
	//  smooth tilt angle
	mATilt += std::min(maxDiff, std::max(-maxDiff, tilt - mATilt)) * time * smoothSpeed;
	

	//-------------------------------------------------------------------------------------------
    if (ca->mType == CAM_Car)	/* 3 Car - car pos & rot full */
    {
		camPosFinal = goalLook;
		camRotFinal = sphere ? orient * qSphFix : orient;

		posOut->camPos = camPosFinal;  // save result in out posInfo
		posOut->camRot = camRotFinal;
		return;
	}
	
    if (ca->mType == CAM_Follow)  ofs = ca->mOffset;
    
	Vector3  pos,goalPos;
	pos     = camPosFinal - ofs;
	goalPos = posGoal;
	
	Vector3  xyz;
	if (ca->mType != CAM_Arena)
	{
		Real x,y,z,xz;   // pitch & yaw to direction vector
		Real ap = bUseTilt ? (ca->mPitch.valueRadians() + mATilt.valueRadians()) : ca->mPitch.valueRadians(),
			 ay = ca->mYaw.valueRadians();
		y = sin(ap), xz = cos(ap);
		x = sin(ay) * xz, z = cos(ay) * xz;
		xyz = Vector3(x,y,z);  xyz *= ca->mDist;
	}
	
	bool manualOrient = false;
	switch (ca->mType)
	{
		case CAM_Arena:		/* 2 Arena - free pos & rot */
		    goalPos = ca->mOffset - ofs;
		    break;
		    
		case CAM_Free:		/* 1 Free - free rot, pos from car */
			goalPos += xyz;
			break;
			
		case CAM_Follow:	/* 0 Follow - car rotY & pos from behind car, smooth */
		{	Quaternion  orient = orientGoal * qOrRot;
			orient.FromAngleAxis(orient.getYaw(), Vector3::UNIT_Y);
			goalPos += orient * xyz;
		}	break;
		
		case CAM_ExtAng:    /* 4 Extended Angle - car in center, angle smooth */
		{	Quaternion  orient = orientGoal * qOrRot;
			Quaternion  ory;  ory.FromAngleAxis(orient.getYaw(), Vector3::UNIT_Y);

			if (first)  {  qq = ory;  }
			else  qq = orient.Slerp(ca->mSpeed * time, qq, ory, true);

			//  smooth dist from vel
			#if 0
			{
				if (first)  {  mPosNodeOld = posGoal;  }
				Real vel = (posGoal - mPosNodeOld).length() / std::max(0.002f, std::min(0.1f, time));
				mPosNodeOld = posGoal;
				if (first)  mVel = 0.f;  else
					mVel += (vel - mVel) * time * 8.f;  //par-  vel smooth speed
				if (!first)
					xyz *= 1.f + std::min(100.f, mVel) * 0.01f;  //par-  vel dist factor
			}
			#endif

			Quaternion  qy = Quaternion(ca->mYaw,Vector3(0,1,0));
			goalPos += qq * (xyz + ca->mOffset);
			
			camPosFinal = goalPos;
			camRotFinal = qq * qy * Quaternion(Degree(-ca->mPitch - mATilt), Vector3(1,0,0));
			manualOrient = true;
		}	break;
	}

	if (!manualOrient)  // if !CAM_ExtAng
	{
		float dtmul = ca->mSpeed == 0 ? 1.0f : ca->mSpeed * time;

		if (ca->mType ==  CAM_Arena)
		{
			Vector3  Pos(0,0,0), goalPos = ca->mOffset;
			Pos = camPosFinal;  //read last state (smooth)
			Pos += (goalPos - Pos) * dtmul;
			
			mAPitch += (ca->mPitch - mAPitch) * dtmul;
			mAYaw += (ca->mYaw - mAYaw) * dtmul;
			
			if (first)  {  Pos = goalPos;  mAPitch = ca->mPitch;  mAYaw = ca->mYaw;  }
			camPosFinal = Pos;
			camRotFinal = Quaternion(Degree(mAYaw),Vector3(0,1,0)) * Quaternion(Degree(mAPitch),Vector3(1,0,0));
			manualOrient = true;
		}
		else
		{
			if (first)  pos = goalPos;
			Vector3  addPos,addLook;
			addPos = (goalPos - pos).normalisedCopy() * (goalPos - pos).length() * dtmul;
			if (addPos.squaredLength() > (goalPos - pos).squaredLength())  addPos = goalPos - pos;
			pos += addPos;
			camPosFinal = pos + ofs;
		
			goalLook = posGoal + ofs;
			if (first)	{	mLook = goalLook;  }

			addLook = (goalLook - mLook) * dtmul;//Rot;
			mLook += addLook;
		}
	}

	//camLookFinal = mLook;
	if (!manualOrient)  // CAM_Free or CAM_Follow
	{
		Vector3 zdir = camPosFinal - mLook;  zdir.normalise();
        Vector3 xVec = Vector3::UNIT_Y.crossProduct(zdir);  xVec.normalise();
        Vector3 yVec = zdir.crossProduct(xVec);  yVec.normalise();
        Quaternion q;  q.FromAxes(xVec, yVec, zdir);
		camRotFinal = q;
	}
	
	//  cast ray from car to camera, reduce dist if hit
	//-------------------------------------------------------------------------------------------
	Vector3 pp = camPosFinal;
	if (bounce)
		pp += posIn.camOfs * ca->mOfsMul
			* gPar.camBncScale * pSet->cam_bnc_mul;
	
	Vector3 p = posGoal;  p.y += 1.f;  //up
	//Vector3 d = camRotFinal * Vector3::UNIT_Z;  d.normalise();
	Vector3 d = pp - p;  d.normalise();
	
	if (!first && ca->mType != CAM_Arena)
	{
		MATHVECTOR<float,3> pos1(p.x,-p.z,p.y), dir(d.x,-d.z,d.y);  //dir = dir.Normalize();
		COLLISION_CONTACT ct;
		float maxLen = (p - pp).length();  //cam near
		world->CastRay(pos1, dir, maxLen,chassis, ct,  0,0, true, true, true/*+*/);
		//dbgLen = -maxLen;
	
		if (ct.GetColObj())
		{
			float len = ct.GetDepth();  //dbgLen = len;
			len -= 0.2f + ct.GetNormal()[2];  ///par  normal up, flat terrain, move closer
			if (len < maxLen)
			{
				Real dist = maxLen - len;
				if (dist > mDistReduce)
					mDistReduce = dist;
		}	}
	}

	//  save result in out posInfo
	posOut->camPos = mDistReduce > 0.0001f ? (pp - d * mDistReduce) : pp;
	posOut->camRot = camRotFinal;

	//  smooth, back to normal dist
	if (mDistReduce > 0.f)
		mDistReduce -= time * 10.f;
}


void FollowCamera::Apply(const PosInfo& posIn)
{
	//boost::this_thread::sleep(boost::posix_time::milliseconds(rand()%20));
	if (!mCamera)  return;

	mCamera->setPosition(posIn.camPos);
	mCamera->setOrientation(posIn.camRot);
}


///  mouse Move
//-----------------------------------------------------------------------------------------------------

void FollowCamera::Move( bool mbLeft, bool mbRight, bool mbMiddle, bool shift, Real mx, Real my, Real mz )
{
	if (!ca)  return;
	fMoveTime = 0;
	bool arena = ca->mType == CAM_Arena;
	Real myH = my * -0.01, mzH = mz/120.f;
	mx *= 0.005;  my *= 0.005;

	if (shift && mbMiddle)
	{
		ca->mSpeed += my*5;
		if (ca->mSpeed < 0.f)  ca->mSpeed = 0.f;
		return;
	}

	//----------------------------------------------
	if (arena)  // Arena - free camera
	{
		Real a = ca->mYaw.valueRadians(), sx = cosf(a), sy = sinf(a);
		Vector3 vx(sx,0,-sy), vy(sy,0,sx);

		if (mbMiddle)
		{
			const Real s = -20;
			ca->mOffset += s*mx *vy + Vector3(0, s*my, 0);
		}
		if (mbRight)
		{
			const Real s = 20;
			if (shift)
				ca->mOffset += Vector3(0, s*myH, 0);
			else
				ca->mOffset += s*mx*vx + s*my*vy;
		}
		if (mbLeft)
		{
			const Real s = 0.5;
			ca->mPitch -= Radian(s*my);
			if (!shift)
				ca->mYaw -= Radian(s*mx);
		}
		//  wheel
		ca->mPitch  += Radian(mzH * 3.f*PI_d/180.f);
		return;
	}
	//----------------------------------------------
	if (ca->mType == CAM_ExtAng)
	{
		if (mbMiddle)
		{	ca->mOffset.x = 0;  ca->mOffset.z = 0;  ca->mYaw = 0.f;  }
		if (mbLeft)
		{
			ca->mPitch -= Radian(my);
			if (shift)
				ca->mYaw += Radian(mx);
			else
				ca->mDist  *= 1.0 - mx * 0.4;
		}
		if (mbRight)
		if (shift)	ca->mOffset += Vector3(mx, 0, my);
		else		ca->mOffset += Vector3(0, -my, 0);

		ca->mDist  *= 1.0 - mzH * 0.1;
		return;
	}
	if (ca->mType == CAM_Car)
	{
		if (mbMiddle)
			ca->mOffset.x = 0;
		if (mbLeft)
		{
			ca->mOffset += Vector3(0, -my, 0);
			ca->mDist   *= 1.0 - mx * 0.4;
		}
		if (mbRight)
			ca->mOffset += Vector3(mx, 0, my);

		ca->mOffset += Vector3(0, mzH * 0.004, 0);
		return;
	}
	//----------------------------------------------
	if (mbMiddle)
	{
		if (ca->mType == CAM_Follow)
			ca->mYaw = Degree(0);
		else
			ca->mOffset = Vector3(0, 0.0, 0);
	}
	if (mbRight)
	{
		if (!shift)
			ca->mOffset += Vector3(0, -my, 0);
		else
		{	ca->mOffset += Vector3(0, myH, 0);
			ca->mDist   *= 1.0 - mx * 0.3;
		}
	}
	if (mbLeft)
	{
		if (!shift)
		{	ca->mPitch -= Radian(my);
			//ca->mDist *= 1.0 - mzH * 0.1;
		} else {
			ca->mYaw   += Radian(mx);
			ca->mPitch -= Radian(my);
		}
		if (ca->mDist < 1.5)
			ca->mDist = 1.5;
	}
	ca->mDist  *= 1.0 - mzH * 0.1;
}


///  upd Info
//-----------------------------------------------------------------------------------------------------
bool FollowCamera::updInfo(Real time)
{
	if (!ca)  return false;

	if (fMoveTime >= 1.0)	// hide after 1sec
		return false;
	else
		fMoveTime += time;
	
    switch (ca->mType)
    {
	case CAM_Follow: sprintf(ss, sFmt_Follow.c_str()
		,ca->mType, CAM_Str[ca->mType], ca->mYaw.valueDegrees(), ca->mPitch.valueDegrees(), ca->mDist
		,ca->mOffset.y, ca->mSpeed);	break;
	case CAM_Free:   sprintf(ss, sFmt_Free.c_str()
		,ca->mType, CAM_Str[ca->mType], ca->mYaw.valueDegrees(), ca->mPitch.valueDegrees(), ca->mDist
		,ca->mOffset.y, ca->mSpeed);	break;
	case CAM_ExtAng:   sprintf(ss, sFmt_ExtAng.c_str()
		,ca->mType, CAM_Str[ca->mType], ca->mPitch.valueDegrees(), ca->mDist
		,ca->mOffset.y, ca->mOffset.x, ca->mOffset.z, ca->mSpeed);	break;
	case CAM_Arena:  sprintf(ss, sFmt_Arena.c_str()
		,ca->mType, CAM_Str[ca->mType], ca->mYaw.valueDegrees(), ca->mPitch.valueDegrees(), ca->mDist
		,ca->mOffset.x, ca->mOffset.y, ca->mOffset.z, ca->mSpeed);	break;
	case CAM_Car:    sprintf(ss, sFmt_Car.c_str()
		,ca->mType, CAM_Str[ca->mType], ca->mOffset.z, ca->mOffset.x, ca->mOffset.y);	break;
	}
	return true;
}



///  Cameras
///-----------------------------------------------------------------------------------------------------

void FollowCamera::updAngle()
{
	if (miCount <= 0)  return;
	miCurrent = std::max(0, std::min(miCount-1, miCurrent));

	CameraAngle* c = mCameraAngles[miCurrent];
	if (ca->mType != c->mType)	First();  // changed type, reset
    *ca = *c;  // copy
    mDistReduce = 0.f;  //reset

	sName = toStr(miCurrent+1) + "/" + toStr(miCount)
		+ (ca->mMain > 0 ? ". " : "  ") + ca->mName;
	updName = true;
}

void FollowCamera::saveCamera()
{
	CameraAngle* c = mCameraAngles[miCurrent];
	c->mName = ca->mName;	c->mType = ca->mType;  c->mSpeed = ca->mSpeed;
	c->mYaw = ca->mYaw;		c->mPitch = ca->mPitch;
	c->mDist = ca->mDist;	c->mOffset = ca->mOffset;
}


//  change next,prev

void FollowCamera::incCur(int dir)
{
	miCurrent += dir;
	if (miCurrent >= miCount)	miCurrent = 0;  // -= miCount;
	if (miCurrent < 0)			miCurrent = miCount-1;
}

void FollowCamera::Next(bool bPrev, bool bMainOnly)
{
	int dir = bPrev ? -1 : 1;
	if (!bMainOnly)  // all
	{
		incCur(dir);  updAngle();  return;
	}else
	{	int cnt = 0, old = miCurrent;
		while (cnt < miCount)
		{
			cnt++;  incCur(dir);
			CameraAngle* c = mCameraAngles[miCurrent];
			if (c->mMain > 0)
			{	updAngle();  return;  }
		}
		miCurrent = old;
	}
}

void FollowCamera::setCamera(int ang)
{
	miCurrent = ang;  updAngle();
}


//  ctors

FollowCamera::FollowCamera(Camera* cam,	SETTINGS* pSet1) :
	first(true), iFirst(0), ca(0), updName(0),
    mCamera(cam), mTerrain(0), chassis(0), pSet(pSet1),
    mLook(Vector3::ZERO), mPosNodeOld(Vector3::ZERO), mVel(0),
    mAPitch(0.f),mAYaw(0.f), mATilt(0.f), mDistReduce(0.f)
{ 
	ca = new CameraAngle();
	ss[0]=0;
}

void FollowCamera::First()
{
	first = true;  iFirst = 0;
}

FollowCamera::~FollowCamera()
{
	delete ca;  ca = 0;
	Destroy();
}

void FollowCamera::Destroy()
{
	for (std::vector<CameraAngle*>::iterator it = mCameraAngles.begin(); it != mCameraAngles.end(); ++it)
		delete *it;
	mCameraAngles.clear();
}

CameraAngle::CameraAngle()
	:mType(CAM_Follow), mName("Follow Default"), mMain(0)
	,mDist(7), mSpeed(10), mSpeedRot(10), mOfsMul(1)
	,mYaw(0), mPitch(7),  mOffset(0,1.2,0), mHideGlass(0)
{	}



///  Load from xml
//-----------------------------------------------------------------------------------------------------
bool FollowCamera::loadCameras()
{
	fMoveTime = 5.f;  // hide hint on start

	miCount = 0;  miCurrent = 0;
	Destroy();

	XMLDocument doc;
	XMLError e = doc.LoadFile((PATHMANAGER::Cars() + "/cameras.xml").c_str());
	if (e == XML_SUCCESS)
	{
		XMLElement* root = doc.RootElement();
		if (!root) {  /*mErrorDialog->show(String("Error loading Cameras !!"), false );  return false;*/  }
		XMLElement* cam = root->FirstChildElement("Camera");
		if (!cam) {  /*mErrorDialog->show(String("Error loading Camera !!"), false );  return false;*/  }
		
		while (cam)
		{
			CameraAngle* c = new CameraAngle();  const char* a = 0;
			c->mName = cam->Attribute("name");
			c->mType = (CamTypes)s2i(cam->Attribute("type"));
			c->mYaw = Degree(0);  c->mPitch = Degree(0);  c->mDist = 10;  c->mSpeed = 10;

			a = cam->Attribute("default");	if (a)  if (s2i(a)==1)  miCurrent = miCount;
			a = cam->Attribute("on");		if (a)  c->mMain = s2i(a)-1;
			a = cam->Attribute("hideGlass");	if (a)  c->mHideGlass = s2i(a);

			a = cam->Attribute("yaw");		if (a)  c->mYaw += Degree(s2r(a));
			a = cam->Attribute("pitch");	if (a)  c->mPitch = Degree(s2r(a));
			a = cam->Attribute("dist");		if (a)  c->mDist = s2r(a);
			a = cam->Attribute("offset");	if (a)  c->mOffset = s2v(a);
			a = cam->Attribute("speed");	if (a)  c->mSpeed = s2r(a);
			a = cam->Attribute("spRot");	if (a)  c->mSpeedRot = s2r(a);  else  c->mSpeedRot = c->mSpeed;
			a = cam->Attribute("bounce");	if (a)  c->mOfsMul = s2r(a);

			if (c->mMain >= 0)  {
				mCameraAngles.push_back(c);  miCount++;  }
			else
				delete c;
			cam = cam->NextSiblingElement("Camera");
		}
	}
	
	miCount = mCameraAngles.size();
	if (miCount == 0)
	{
		CameraAngle* c = new CameraAngle();  c->mName = "Follow Default";
		c->mType = CAM_Follow;  c->mYaw = Degree(0);  c->mPitch = Degree(14);
		c->mDist = 9;  c->mOffset = Vector3(0,2,0);  c->mSpeed = 15;
		mCameraAngles.push_back(c);
		miCount++;
	}

	updAngle();  updFmtTxt();
	return true;
}


//  update format texts, from translations
//-----------------------------------------------------------------------------------------------------
void FollowCamera::updFmtTxt()
{
	String sTR = TR("#{CamInfoStrings}");
	vector<String>::type vs = StringUtil::split(sTR,",");
	
	if (vs.size() < 17)
	{	LogO("==== Error in camera info translate string. Need 17 strings, have "+toStr(vs.size())+", using default English. " + sTR);
		sTR="Type,Yaw,Pitch,Dist,Height,Speed,Offset,LEFT,RIGHT,Middle,Wheel,shift,Rotate,reset,move,H,Pos";
		vs = StringUtil::split(sTR,",");  }

	String sType  =vs[0],
		sYaw   =vs[1],  sPitch =vs[2],  sDist  =vs[3],  sHeight=vs[4],  sSpeed =vs[5],  sOffset=vs[6],
		sLEFT  =vs[7],  sRIGHT =vs[8],  sMiddle=vs[9],  sWheel =vs[10], sshift =vs[11],
		sRotate=vs[12], sreset =vs[13], smove  =vs[14], sH     =vs[15], sPos   =vs[16];

	sFmt_Follow =
		sType+": %d %s  "+sYaw+":%5.1f "+sPitch+":%5.1f  "+sDist+":%5.1f  "+sHeight+": %3.1f  "+sSpeed+": %2.0f\n"+
		sLEFT+": "+sPitch+"  "+sshift+": "+sRotate+" | "+sRIGHT+": "+sHeight+"  "+sshift+": "+sDist+","+sH+"\n"+
		sMiddle+": "+sreset+" "+sYaw+"  "+sshift+": "+sSpeed+" | "+sWheel+": "+sDist;  // | S: save"
	sFmt_Free =
		sType+": %d %s  "+sYaw+":%5.1f "+sPitch+":%5.1f  "+sDist+":%5.1f  "+sHeight+": %3.1f  "+sSpeed+": %2.0f\n"+
		sLEFT+": "+sPitch+"  "+sshift+": "+sRotate+" | "+sRIGHT+": "+sHeight+"  "+sshift+": "+sDist+","+sH+"\n"+
		sMiddle+": "+sreset+" "+sHeight+"  "+sshift+": "+sSpeed+" | "+sWheel+": "+sDist;
	sFmt_ExtAng =
		sType+": %d %s  "+sPitch+":%5.1f  "+sDist+":%5.1f  "+sHeight+": %3.1f  "+sOffset+": %3.1f %3.1f  "+sSpeed+": %3.1f\n"+
		sLEFT+": "+sPitch+", "+sDist+"  "+sshift+": "+sRotate+" | "+sRIGHT+": "+sHeight+"  "+sshift+": "+sOffset+"\n"+
		sMiddle+": "+sreset+" "+sOffset+"  "+sshift+": "+sSpeed+" | "+sWheel+": "+sDist;
	sFmt_Arena =
		sType+": %d %s  "+sYaw+":%5.1f "+sPitch+":%5.1f  "+sDist+":%5.1f  "+sPos+": %3.1f %3.1f %3.1f  "+sSpeed+": %2.0f\n"+
		sLEFT+": "+sRotate+"  "+sshift+": "+sPitch+" | "+sRIGHT+": "+smove+"  "+sshift+": "+sHeight+"\n"+
		sMiddle+": "+smove+","+sH+" | "+sWheel+": "+sPitch;
	sFmt_Car =
		sType+": %d %s  "+sOffset+": %4.2f %4.2f  "+sHeight+": %4.2f\n"+
		sLEFT+": "+sHeight+" | "+sRIGHT+": "+sOffset+" | "+sMiddle+": "+sreset+" "+sOffset+"X";
}
