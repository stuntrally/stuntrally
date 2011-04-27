#include "stdafx.h"
#include "FollowCamera.h"
#include "../tinyxml/tinyxml.h"
#include "../vdrift/pathmanager.h"
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
			moveAboveTerrain();
			updInfo(time);
			return;
		}	break;
	}

	/* Move */
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
	
	moveAboveTerrain();
	
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
		ca.mPitch  += Radian(mzH * 3.f*PI/180.f);
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

///   align with terrain
//-----------------------------------------------------------------------------------------------------
void FollowCamera::moveAboveTerrain()
{
	/*
	 * Prevent the camera from going under the ground.
	 * We do this by queueing the terrain height at the camera position
	 * and if the camera is under it, we move the camera a little above the terrain. 
	*/
	if (!mTerrain)  return;

	const float fMinHOfs = 0.2f;  //  minimum of 0.2 m distance to the ground
	Vector3 camPos = mCamera->getPosition();
	float h = mTerrain->getHeightAtWorldPosition(camPos);
	if (h != 0.f)
	if (h + fMinHOfs > camPos.y)
		mCamera->setPosition(camPos.x, h + fMinHOfs, camPos.z);
}

///  upd Info
//-----------------------------------------------------------------------------------------------------
void FollowCamera::updInfo(Real time)
{
	if (!ovInfo)  return;

	if (fMoveTime >= 1.0)	// hide after 1sec
	{
		ovInfo->setCaption("");
		return;
	}else
		fMoveTime += time;
	
    static char ss[256];
    switch (ca.mType)
    {
	case CAM_Follow: sprintf(ss,
		"Type: %d %s  Yaw:%5.1f Pitch:%5.1f  Dist:%5.1f  Height: %3.1f  Speed: %2.0f\n"
		"LEFT: Pitch  shift: Rotate | RIGHT: Height  shift: Dist,H | Middle: reset Yaw  shift: Speed | Wheel: Dist"  // | S: save"
		,ca.mType, CAM_Str[ca.mType], ca.mYaw.valueDegrees(), ca.mPitch.valueDegrees(), ca.mDist
		,ca.mOffset.y, ca.mSpeed);	break;
	case CAM_Free:   sprintf(ss,
		"Type: %d %s  Yaw:%5.1f Pitch:%5.1f  Dist:%5.1f  Height: %3.1f  Speed: %2.0f\n"
		"LEFT: Pitch  shift: Rotate | RIGHT: Height  shift: Dist,H | Middle: reset Height  shift: Speed | Wheel: Dist"
		,ca.mType, CAM_Str[ca.mType], ca.mYaw.valueDegrees(), ca.mPitch.valueDegrees(), ca.mDist
		,ca.mOffset.y, ca.mSpeed);	break;
	case CAM_ExtAng:   sprintf(ss,
		"Type: %d %s  Pitch:%5.1f  Dist:%5.1f  Height: %3.1f  Offset: %3.1f %3.1f  Speed: %3.1f\n"
		"LEFT: Pitch, Dist | RIGHT: offset  shift: Height | Middle: reset offset  shift: Speed | Wheel: Dist"
		,ca.mType, CAM_Str[ca.mType], ca.mPitch.valueDegrees(), ca.mDist
		,ca.mOffset.y, ca.mOffset.x, ca.mOffset.z, ca.mSpeed);	break;
	case CAM_Arena:  sprintf(ss,
		"Type: %d %s  Yaw:%5.1f Pitch:%5.1f  Dist:%5.1f  Pos: %3.1f %3.1f %3.1f  Speed: %2.0f\n"
		"LEFT: Rotate  shift: Pitch | RIGHT: move  shift: Height | Middle: move,H | Wheel: Pitch"
		,ca.mType, CAM_Str[ca.mType], ca.mYaw.valueDegrees(), ca.mPitch.valueDegrees(), ca.mDist
		,ca.mOffset.x, ca.mOffset.y, ca.mOffset.z, ca.mSpeed);	break;
	case CAM_Car:    sprintf(ss,
		"Type: %d %s  Height: %3.1f  Offset: %3.1f %3.1f\n"
		"LEFT: Height | RIGHT: offset | Middle: reset offsetX"
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
{  }

FollowCamera::~FollowCamera()
{  }

CameraAngle::CameraAngle() :
	mType(CAM_Follow), mName("Follow Default"), mMain(0),
	mDist(7), mSpeed(10), mSpeedRot(10),
	mYaw(0), mPitch(7),  mOffset(0,1.2,0), mHideGlass(0)
{  }



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
	return true;
}
