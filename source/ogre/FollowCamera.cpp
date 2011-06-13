#include "pch.h"
#include "Defines.h"
#include "FollowCamera.h"
#include "../tinyxml/tinyxml.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/mathvector.h"
#include "../vdrift/collision_contact.h"
#include "../bullet/btBulletCollisionCommon.h"
#include "../btOgre/BtOgreDebug.h"

#include <OgreCamera.h>
#include <OgreSceneNode.h>
#include <OgreOverlayElement.h>
#include <OgreOverlayManager.h>
#include <OgreTerrainGroup.h>

#include <MyGUI.h>
using namespace Ogre;


///  Update
//-----------------------------------------------------------------------------------------------------

void FollowCamera::update( Real time )
{
	if (!mGoalNode)  return;

	const Quaternion qo = Quaternion(Degree(180),Vector3(0,0,1)) * Quaternion(Degree(-90),Vector3(0,1,0));
	Quaternion  orient = mGoalNode->getOrientation() * qo;
	Vector3  ofs = orient * ca.mOffset,  goalLook = mGoalNode->getPosition() + ofs;
	
    if (ca.mType == CAM_Car)	/* 3 Car - car pos & rot full */
    {
		mCamera->setPosition( goalLook );
		mCamera->setOrientation( orient );
		updInfo(time);
		return;
	}
    if (ca.mType == CAM_Follow)  ofs = ca.mOffset;
    
	Vector3  pos,goalPos, addPos,addLook;
	pos     = mCamera->getPosition() - ofs;
	goalPos = mGoalNode->getPosition();
	
	Real x,y,z,xz;   // get the Y offset and the XZ plane offset from Pitch
	y = sin( ca.mPitch.valueRadians() ) * ca.mDist;
	xz= cos( ca.mPitch.valueRadians() ) * ca.mDist;  // get the x any z values from Yaw
	x = sin( ca.mYaw.valueRadians() ) * xz;
	z = cos( ca.mYaw.valueRadians() ) * xz;
	Vector3 xyz(x,y,z);
	
	bool manualOrient = false;
	switch (ca.mType)
	{
		case CAM_Arena:		/* 2 Arena - free pos & rot */
		    goalPos = ca.mOffset - ofs;
		    break;
		    
		case CAM_Free:		/* 1 Free - free rot, pos from car */
			goalPos += xyz;
			break;
			
		case CAM_Follow:	/* 0 Follow - car rotY & pos from behind car, smooth */
		{	Quaternion  orient = mGoalNode->getOrientation() * Quaternion(Degree(90),Vector3(0,1,0));
			orient.FromAngleAxis(orient.getYaw(), Vector3::UNIT_Y);
			goalPos += orient * xyz;
		}	break;
		
		case CAM_ExtAng:
		{
			///  new  ---
			Quaternion  orient = mGoalNode->getOrientation() * Quaternion(Degree(90),Vector3(0,1,0));
			Quaternion  ory;  ory.FromAngleAxis(orient.getYaw(), Vector3::UNIT_Y);

			if (first)  {  qq = ory;  first = false;  }
			else
				qq = orient.Slerp(ca.mSpeed * time, qq, ory, true);

			goalPos += qq * (xyz + ca.mOffset);
			
			mCamera->setPosition( goalPos );
			mCamera->setOrientation( qq * Quaternion(Degree(-ca.mPitch),Vector3(1,0,0)) );
			manualOrient = true;
		}	break;
	}
	if (!manualOrient)
	{	/* Move */
		float dtmul = ca.mSpeed == 0 ? 1.0f : ca.mSpeed * time;
		//float dtmulRot = ca.mSpeedRot == 0 ? 1.0f : ca.mSpeedRot * time;
		//if (ca.mSpeed == ca.mSpeedRot || 1)  dtmulRot = dtmul;

		if (first)	{	pos = goalPos;  }

		addPos = (goalPos - pos).normalisedCopy() * (goalPos - pos).length() * dtmul;
		if (addPos.squaredLength() > (goalPos - pos).squaredLength())  addPos = goalPos - pos;
		pos += addPos;
		mCamera->setPosition( pos + ofs );  //if (mypos.y<-3.5)  mypos.y=-3.5;

		/* Look */
		if (ca.mType == CAM_Arena)
			goalLook = ca.mOffset + Vector3(x,-y,z);
		else
		if (mGoalNode)
			goalLook = mGoalNode->getPosition() + ofs;

		if (first)	{	mLook = goalLook;  first = false;  }

		addLook = (goalLook - mLook) * dtmul;//Rot;
		mLook += addLook;
	}

	/// cast ray from car to camera, to prevent objects blocking the camera's sight
	#if 0
	// update sphere pos
	btVector3 carPos = BtOgre::Convert::toBullet(mGoalNode->getPosition());
	state->setWorldTransform( btTransform(btQuaternion(0,0,0,1), carPos ));
	
	// calculate origin & direction of the ray, convert to vdrift coordinates
	MATHVECTOR<float,3> origin;
	origin.Set( carPos.x(), carPos.y(), carPos.z() );
	MATHVECTOR<float,3> direction;
	btVector3 dir = BtOgre::Convert::toBullet(mCamera->getPosition()-mGoalNode->getPosition());
	direction.Set(dir.x(), dir.y(), dir.z());
	Real distance = (mCamera->getPosition()-mGoalNode->getPosition() ).length();
	int pOnRoad;
	
	// shoot our ray
	COLLISION_CONTACT contact;
	mWorld->CastRay( origin, direction, distance, body, contact, &pOnRoad );
	
	if (contact.col != NULL)
	{
		LogO("Collision occured");
		// collision occured - update cam pos
		mCamera->setPosition( BtOgre::Convert::toOgre( btVector3(contact.GetPosition()[0], contact.GetPosition()[1], contact.GetPosition()[2]) ) );
	}
	#endif
	
	moveAboveTerrain();
	if (!manualOrient)
		mCamera->lookAt( mLook );
	updInfo(time);
}



///  mouse Move
//-----------------------------------------------------------------------------------------------------

void FollowCamera::Move( bool mbLeft, bool mbRight, bool mbMiddle, bool shift, Real mx, Real my, Real mz )
{
	fMoveTime = 0;
	bool arena = ca.mType == CAM_Arena;
	Real myH = my * -0.01, mzH = mz/120.f;
	mx *= 0.005;  my *= 0.005;

	if (shift && mbMiddle)
	{
		ca.mSpeed += my*5;
		if (ca.mSpeed < 0.f)  ca.mSpeed = 0.f;
		return;
	}

	//----------------------------------------------
	if (arena)  // Arena - free camera
	{
		Real a = ca.mYaw.valueRadians(), sx = cosf(a), sy = sinf(a);
		Vector3 vx(sx,0,-sy), vy(sy,0,sx);

		if (mbMiddle)
		{
			const Real s = -20;
			ca.mOffset += s*mx *vy + Vector3(0, s*my, 0);
		}
		if (mbRight)
		{
			const Real s = 20;
			if (shift)
				ca.mOffset += Vector3(0, s*myH, 0);
			else
				ca.mOffset += s*mx*vx + s*my*vy;
		}
		if (mbLeft)
		{
			const Real s = 0.5;
			ca.mPitch -= Radian(s*my);
			if (!shift)
				ca.mYaw -= Radian(s*mx);
		}
		//  wheel
		ca.mPitch  += Radian(mzH * 3.f*PI_d/180.f);
		return;
	}
	//----------------------------------------------
	if (ca.mType == CAM_ExtAng)
	{
		if (mbMiddle)
		{	ca.mOffset.x = 0;  ca.mOffset.z = 0;  }
		if (mbLeft)
		{
			ca.mPitch -= Radian(my);
			ca.mDist  *= 1.0 - mx * 0.4;
		}
		if (mbRight)
		if (shift)	ca.mOffset += Vector3(0, -my, 0);
		else		ca.mOffset += Vector3(mx, 0, my);

		ca.mDist  *= 1.0 - mzH * 0.1;
		return;
	}
	if (ca.mType == CAM_Car)
	{
		if (mbMiddle)
			ca.mOffset.x = 0;
		if (mbLeft)
		{
			ca.mOffset += Vector3(0, -my, 0);
			ca.mDist   *= 1.0 - mx * 0.4;
		}
		if (mbRight)
			ca.mOffset += Vector3(mx, 0, my);

		ca.mOffset += Vector3(0, mzH * 0.004, 0);
		return;
	}
	//----------------------------------------------
	if (mbMiddle)
	{
		if (ca.mType == CAM_Follow)
			ca.mYaw = Degree(0);
		else
			ca.mOffset = Vector3(0, 0.0, 0);
	}
	if (mbRight)
	{
		if (!shift)
			ca.mOffset += Vector3(0, -my, 0);
		else
		{	ca.mOffset += Vector3(0, myH, 0);
			ca.mDist   *= 1.0 - mx * 0.3;
		}
	}
	if (mbLeft)
	{
		if (!shift)
		{	ca.mPitch -= Radian(my);
			//ca.mDist *= 1.0 - mzH * 0.1;
		} else {
			ca.mYaw   += Radian(mx);
			ca.mPitch -= Radian(my);
		}
		if (ca.mDist < 1.5)
			ca.mDist = 1.5;
	}
	ca.mDist  *= 1.0 - mzH * 0.1;
}


//  prevent camera from going under ground.
//-----------------------------------------------------------------------------------------------------
void FollowCamera::moveAboveTerrain()
{
	if (!mTerrain)  return;

	const static Real terOfs = 0.2f;  //  minimum distance above ground
	Vector3 camPos = mCamera->getPosition();
	float h = mTerrain->getHeightAtWorldPosition(camPos);
	if (h != 0.f)  // out of terrain
	if (h + terOfs > camPos.y)
		mCamera->setPosition(camPos.x, h + terOfs, camPos.z);
}


///  upd Info
//-----------------------------------------------------------------------------------------------------
void FollowCamera::updInfo(Real time)
{
	if (!ovInfo)  return;

	if (fMoveTime >= 1.0)	// hide after 1sec
	{	ovInfo->setCaption("");  return;  }
	else
		fMoveTime += time;
	
    static char ss[512];
    switch (ca.mType)
    {
	case CAM_Follow: sprintf(ss, sFmt_Follow.c_str()
		,ca.mType, CAM_Str[ca.mType], ca.mYaw.valueDegrees(), ca.mPitch.valueDegrees(), ca.mDist
		,ca.mOffset.y, ca.mSpeed);	break;
	case CAM_Free:   sprintf(ss, sFmt_Free.c_str()
		,ca.mType, CAM_Str[ca.mType], ca.mYaw.valueDegrees(), ca.mPitch.valueDegrees(), ca.mDist
		,ca.mOffset.y, ca.mSpeed);	break;
	case CAM_ExtAng:   sprintf(ss, sFmt_ExtAng.c_str()
		,ca.mType, CAM_Str[ca.mType], ca.mPitch.valueDegrees(), ca.mDist
		,ca.mOffset.y, ca.mOffset.x, ca.mOffset.z, ca.mSpeed);	break;
	case CAM_Arena:  sprintf(ss, sFmt_Arena.c_str()
		,ca.mType, CAM_Str[ca.mType], ca.mYaw.valueDegrees(), ca.mPitch.valueDegrees(), ca.mDist
		,ca.mOffset.x, ca.mOffset.y, ca.mOffset.z, ca.mSpeed);	break;
	case CAM_Car:    sprintf(ss, sFmt_Car.c_str()
		,ca.mType, CAM_Str[ca.mType], ca.mOffset.y, ca.mOffset.x, ca.mOffset.z);	break;
	}
	ovInfo->setCaption(ss);
}



///  Cameras
///-----------------------------------------------------------------------------------------------------

void FollowCamera::updAngle()
{
	if (miCount <= 0)	 return;
	CameraAngle* c = &mCameraAngles[miCurrent];
	if (ca.mType != c->mType)	first = true;  // changed type, reset
	
    ca = *c;  // copy

	if (ovName)  // ovName->setCaption(ca.mName);
		ovName->setCaption( toStr(miCurrent+1) + "/" + toStr(miCount)
			+ ((ca.mMain > 0) ? ". " : "  ") + ca.mName);
	updInfo();
}

void FollowCamera::saveCamera()
{
	CameraAngle* c = &mCameraAngles[miCurrent];
    c->mName = ca.mName;	c->mType = ca.mType;  c->mSpeed = ca.mSpeed;
    c->mYaw = ca.mYaw;		c->mPitch = ca.mPitch;
	c->mDist = ca.mDist;	c->mOffset = ca.mOffset;
}


//  change next,prev

void FollowCamera::incCur(int dir)
{
	miCurrent += dir;
	if (miCurrent >= miCount)	miCurrent = 0;
	if (miCurrent < 0)			miCurrent = miCount-1;
}

void FollowCamera::Next(bool bPrev, bool bMainOnly)
{
	int dir = bPrev ? -1 : 1;
	if (!bMainOnly)  // all
	{
		incCur(dir);
		updAngle();
		return;
	}else
	{
		int cnt = 0, old = miCurrent;
		while (cnt < miCount)
		{
			cnt++;
			incCur(dir);
			CameraAngle* c = &mCameraAngles[miCurrent];
			if (c->mMain > 0)
			{
				updAngle();
				return;
			}
		}
		miCurrent = old;
	}
}


void FollowCamera::setCamera(int ang)
{
	miCurrent = ang;
	updAngle();
}


//  ctors

FollowCamera::FollowCamera(Camera* cam) :
	ovInfo(0),ovName(0), first(true),
    mCamera(cam), mGoalNode(0), mTerrain(0),
    mLook(Vector3::ZERO)
    #if 0
    ,shape(0), body(0), state(0)
    #endif
{ 
	#if 0
	// create camera bullet col obj
    shape = new btSphereShape(0.1); // 10cm radius
    state = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0) ));
    btScalar mass = 1;
    btVector3 inertia;
    shape->calculateLocalInertia(mass, inertia);
    body = new btRigidBody(mass, state, shape, inertia);  // _\ delete !...
	#endif
}

FollowCamera::~FollowCamera()
{

}

CameraAngle::CameraAngle() :
	mType(CAM_Follow), mName("Follow Default"), mMain(0),
	mDist(7), mSpeed(10), mSpeedRot(10),
	mYaw(0), mPitch(7),  mOffset(0,1.2,0), mHideGlass(0)
{  

}



///  Load from xml
//-----------------------------------------------------------------------------------------------------
bool FollowCamera::loadCameras()
{
	ovInfo = OverlayManager::getSingleton().getOverlayElement("Car/CameraInfo");
	ovName = OverlayManager::getSingleton().getOverlayElement("Car/Camera");
	fMoveTime = 5.f;  // hide hint on start

	miCount = 0;  miCurrent = 0;
	mCameraAngles.clear();

	TiXmlDocument file;
	if (file.LoadFile((PATHMANAGER::GetCarPath() + "/cameras.xml").c_str()))
	{
		TiXmlElement* root = file.RootElement();
		if (!root) {  /*mErrorDialog->show(String("Error loading Cameras !!"), false );  return false;*/  }
		TiXmlElement* cam = root->FirstChildElement("Camera");
		if (!cam) {  /*mErrorDialog->show(String("Error loading Camera !!"), false );  return false;*/  }
		
		while (cam)
		{
			CameraAngle c;  const char* a = 0;
			c.mName = cam->Attribute("name");
			c.mType = (CamTypes)s2i(cam->Attribute("type"));
			c.mYaw = Degree(0);  c.mPitch = Degree(0);  c.mDist = 10;  c.mSpeed = 10;

			a = cam->Attribute("default");	if (a)  if (s2i(a)==1)  miCurrent = miCount;
			a = cam->Attribute("on");		if (a)  c.mMain = s2i(a)-1;
			a = cam->Attribute("hideGlass");	if (a)  c.mHideGlass = s2i(a);

			a = cam->Attribute("yaw");		if (a)  c.mYaw += Degree(s2r(a));
			a = cam->Attribute("pitch");	if (a)  c.mPitch = Degree(s2r(a));
			a = cam->Attribute("dist");		if (a)  c.mDist = s2r(a);
			a = cam->Attribute("offset");	if (a)  c.mOffset = s2v(a);
			a = cam->Attribute("speed");	if (a)  c.mSpeed = s2r(a);
			a = cam->Attribute("spRot");	if (a)  c.mSpeedRot = s2r(a);  else  c.mSpeedRot = c.mSpeed;

			if (c.mMain >= 0)  {
				mCameraAngles.push_back(c);
				miCount++;  }
			cam = cam->NextSiblingElement("Camera");
		}
	}
	
	miCount = mCameraAngles.size();
	if (miCount == 0)
	{
		CameraAngle c;  c.mName = "Follow Default";
		c.mType = CAM_Follow;  c.mYaw = Degree(0);  c.mPitch = Degree(14);
		c.mDist = 9;  c.mOffset = Vector3(0,2,0);  c.mSpeed = 15;
		mCameraAngles.push_back(c);
		miCount++;
	}

	updAngle();
	updFmtTxt();
	return true;
}


//  update format texts, from translations
//-----------------------------------------------------------------------------------------------------
void FollowCamera::updFmtTxt()
{
	String sTR = TR("#{CamInfoStrings}");
	vector<String>::type vs = StringUtil::split(sTR,",");
	
	if (vs.size() != 16)
	{	LogO("==== Error in camera info translate string. Need 16 strings, have "+toStr(vs.size())+", using default English. " + sTR);
		sTR="Type,Yaw,Pitch,Dist,Height,Speed,Offset,LEFT,RIGHT,Middle,Wheel,shift,Rotate,reset,move,H,Pos";
		vs = StringUtil::split(sTR,",");  }

	String sType  =vs[0],
		sYaw   =vs[1],  sPitch =vs[2],  sDist  =vs[3],  sHeight=vs[4],  sSpeed =vs[5],  sOffset=vs[6],
		sLEFT  =vs[7],  sRIGHT =vs[8],  sMiddle=vs[9],  sWheel =vs[10], sshift =vs[11],
		sRotate=vs[12], sreset =vs[13], smove  =vs[14], sH     =vs[15], sPos   =vs[16];

	sFmt_Follow =
		sType+": %d %s  "+sYaw+":%5.1f "+sPitch+":%5.1f  "+sDist+":%5.1f  "+sHeight+": %3.1f  "+sSpeed+": %2.0f\n"+
		sLEFT+": "+sPitch+"  "+sshift+": "+sRotate+" | "+sRIGHT+": "+sHeight+"  "+sshift+": "+sDist+","+sH+" | "+
		sMiddle+": "+sreset+" "+sYaw+"  "+sshift+": "+sSpeed+" | "+sWheel+": "+sDist;  // | S: save"
	sFmt_Free =
		sType+": %d %s  "+sYaw+":%5.1f "+sPitch+":%5.1f  "+sDist+":%5.1f  "+sHeight+": %3.1f  "+sSpeed+": %2.0f\n"+
		sLEFT+": "+sPitch+"  "+sshift+": "+sRotate+" | "+sRIGHT+": "+sHeight+"  "+sshift+": "+sDist+","+sH+" | "+
		sMiddle+": "+sreset+" "+sHeight+"  "+sshift+": "+sSpeed+" | "+sWheel+": "+sDist;
	sFmt_ExtAng =
		sType+": %d %s  "+sPitch+":%5.1f  "+sDist+":%5.1f  "+sHeight+": %3.1f  "+sOffset+": %3.1f %3.1f  "+sSpeed+": %3.1f\n"+
		sLEFT+": "+sPitch+", "+sDist+" | "+sRIGHT+": "+sOffset+"  "+sshift+": "+sHeight+" | "+
		sMiddle+": "+sreset+" "+sOffset+"  "+sshift+": "+sSpeed+" | "+sWheel+": "+sDist;
	sFmt_Arena =
		sType+": %d %s  "+sYaw+":%5.1f "+sPitch+":%5.1f  "+sDist+":%5.1f  "+sPos+": %3.1f %3.1f %3.1f  "+sSpeed+": %2.0f\n"+
		sLEFT+": "+sRotate+"  "+sshift+": "+sPitch+" | "+sRIGHT+": "+smove+"  "+sshift+": "+sHeight+" | "+
		sMiddle+": "+smove+","+sH+" | "+sWheel+": "+sPitch;
	sFmt_Car =
		sType+": %d %s  "+sHeight+": %3.1f  "+sOffset+": %3.1f %3.1f\n"+
		sLEFT+": "+sHeight+" | "+sRIGHT+": "+sOffset+" | "+sMiddle+": "+sreset+" "+sOffset+"X";
}
