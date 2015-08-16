#include "pch.h"
/*	   Filename:  BtOgreDebug.h
 *	Description:  Contains the Ogre Mesh to Bullet Shape converters.
 *		Version:  1.1  (1.0 modified by CrystalH)
 *		Created:  27/12/2008 01:45:56 PM
 *		 Author:  Nikhilesh (nikki)
 * =====================================================================================*/

#include "DynamicLines.h"
#include "BtOgreDebug.h"

#include <math.h>

#include <OgreSceneNode.h>
#include <OgreResourceGroupManager.h>
#include <OgreMaterial.h>
#include <OgreMaterialManager.h>
#include <OgreColourValue.h>
#include <OgreLogManager.h>

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif


namespace BtOgre  {


DebugDrawer::DebugDrawer(Ogre::SceneNode *node, btDynamicsWorld *world) :
	mNode(node), mWorld(world), m_debugMode(0)
{
	mLineDrawer = new DynamicLines(Ogre::RenderOperation::OT_LINE_LIST);
	mLineDrawer->setCastShadows(false); //`
	mNode->attachObject(mLineDrawer);

	Ogre::ResourceGroupManager::getSingleton().createResourceGroup("BtOgre");
	Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().create("BtOgre/DebugLines", "BtOgre");
	mat->setReceiveShadows(false);
	mat->setLightingEnabled(false);
	//mat->setSelfIllumination(1,1,1);
	mLineDrawer->setMaterial("BtOgre/DebugLines");/**/  //crash debug..
	mLineDrawer->setVisibilityFlags(2/*RV_Hud*/);  // not in reflection
}

DebugDrawer::~DebugDrawer() 
{
    Ogre::MaterialManager::getSingleton().remove("BtOgre/DebugLines");
    Ogre::ResourceGroupManager::getSingleton().destroyResourceGroup("BtOgre");
	delete mLineDrawer;
}

void DebugDrawer::step()
{
	if (m_debugMode > 0)
	{
		mWorld->debugDrawWorld();
		mLineDrawer->update();
		mNode->needUpdate();
		mLineDrawer->clear();
	}else{
		mLineDrawer->clear();
		mLineDrawer->update();
		mNode->needUpdate();
	}
}

void DebugDrawer::reportErrorWarning(const char* warningString)
{
	Ogre::LogManager::getSingleton().logMessage(warningString);
}

void DebugDrawer::draw3dText(const btVector3& location,const char* textString)
{
}

//  mode
void DebugDrawer::setDebugMode(int debugMode)
{
	m_debugMode = debugMode;
	//if (mWorld && mWorld->getDebugDrawer())
	//	mWorld->getDebugDrawer()->setDebugMode(debugMode);	
	if (m_debugMode==0)
		mLineDrawer->clear();
}
	
int	DebugDrawer::getDebugMode() const
{
	return m_debugMode;
}


///  Draw
//---------------------------------------------------------------------------------------------------------------------

//  Quality params  //par
#if 1    // low 1 1
const int sphere = 20, circle = 20, capSide = 90, cylSide = 90;  const btScalar capSph = 90, cylO = 20;
#elif 1  // med 0 1 
const int sphere = 15, circle = 15, capSide = 45, cylSide = 60;  const btScalar capSph = 15, cylO = 15;
#else    // hq  0 0
const int sphere = 10, circle = 10, capSide = 30, cylSide = 30;  const btScalar capSph = 10, cylO = 10;
#endif


void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	Ogre::ColourValue clr(color.getX(),color.getY(),color.getZ(),1);
	mLineDrawer->addLine(Convert::toOgre(from), Convert::toOgre(to), clr);
}

void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB,
	btScalar distance, int lifeTime, const btVector3& color)
{
	Ogre::ColourValue clr(color.getX(),color.getY(),color.getZ(),1);
	mLineDrawer->addLine(Convert::toOgre(PointOnB),
		Convert::toOgre(PointOnB) + (Convert::toOgre(normalOnB) * distance * 20), clr);
}

void DebugDrawer::drawSphere(btScalar radius, const btTransform& transform, const btVector3& color/*, int half*/)
{
	btVector3 p = transform.getOrigin();
	const btMatrix3x3& m = transform.getBasis();

	const btVector3 x = m * btVector3(radius,0,0);
	const btVector3 y = m * btVector3(0,radius,0);
	const btVector3 z = m * btVector3(0,0,radius);
	btVector3 clr = color;  if (clr.getY() <= 0.1f)  {  clr.setZ(1.f);  clr.setY(1.f);  }  // blue to white
	//TODO: render mesh (instancing)...

	const float PI2 = SIMD_2_PI, ad = SIMD_RADS_PER_DEG * sphere;  //par  steps-quality
	{
		btVector3 p1, p1o = p + y, p2o = p + x, p3o = p + y;
		for (float a = ad; a <= PI2+ad; a += ad)
		{
			float s = sinf(a), c = cosf(a);
			p1 = p - s*x + c*y;  drawLine(p1o, p1, clr);	p1o = p1;
			p1 = p - s*z + c*x;  drawLine(p2o, p1, clr);	p2o = p1;
			p1 = p - s*z + c*y;  drawLine(p3o, p1, clr);	p3o = p1;
		}
	}
}

void DebugDrawer::drawCircleZ(btScalar radius, const btVector3& p, const btVector3& x, const btVector3& y, const btVector3& color)
{
	const float PI2 = SIMD_2_PI, ad = SIMD_RADS_PER_DEG * circle;  //par
	btVector3 p1, p1o = p + y;
	for (float a = ad; a <= PI2+ad; a += ad)
	{
		float s = sinf(a), c = cosf(a);
		p1 = p - s*x + c*y;  drawLine(p1o, p1, color);	p1o = p1;
	}
}

void DebugDrawer::drawCapsule(btScalar radius, btScalar halfHeight, int upAxis, const btTransform& transform, const btVector3& color)
{
	btVector3 capStart(0.f,0.f,0.f);  capStart[upAxis] = -halfHeight;
	btVector3 capEnd(0.f,0.f,0.f);    capEnd[upAxis] = halfHeight;

	// Draw the ends
	btTransform tr = transform;
	tr.getOrigin() = transform * capStart;
	{
		btVector3 center = tr.getOrigin();
		btVector3 up = tr.getBasis().getColumn((upAxis+1)%3);
		btVector3 side = tr.getBasis().getColumn((upAxis+2)%3);
		btVector3 axis = -tr.getBasis().getColumn(upAxis);
		//drawSpherePatch(center, up, axis, radius, minTh, maxTh, minPs, maxPs, color, capSph);
		drawArc(center,up,axis,radius,radius,-SIMD_HALF_PI,SIMD_HALF_PI,color,false,capSph);
		drawArc(center,side,axis,radius,radius,-SIMD_HALF_PI,SIMD_HALF_PI,color,false,capSph);
		drawArc(center,axis,side,radius,radius,0,SIMD_2_PI,color,false,capSph);
	}

	tr = transform;
	tr.getOrigin() = transform * capEnd;
	{
		btVector3 center = tr.getOrigin();
		btVector3 up = tr.getBasis().getColumn((upAxis+1)%3);
		btVector3 side = tr.getBasis().getColumn((upAxis+2)%3);
		btVector3 axis = tr.getBasis().getColumn(upAxis);
		//drawSpherePatch(center, up, axis, radius, minTh, maxTh, minPs, maxPs, color, capSph);
		drawArc(center,up,axis,radius,radius,-SIMD_HALF_PI,SIMD_HALF_PI,color,false,capSph);
		drawArc(center,side,axis,radius,radius,-SIMD_HALF_PI,SIMD_HALF_PI,color,false,capSph);
		drawArc(center,axis,side,radius,radius,0,SIMD_2_PI,color,false,capSph);
	}

	//  side lines
	btVector3 start = transform.getOrigin();
	int up1 = (upAxis+1)%3, up2 = (upAxis+2)%3;
	for (int i=0; i<360; i+=capSide)
	{
		capEnd[up1] = capStart[up1] = btSin(btScalar(i)*SIMD_RADS_PER_DEG)*radius;
		capEnd[up2] = capStart[up2] = btCos(btScalar(i)*SIMD_RADS_PER_DEG)*radius;
		drawLine(start + transform.getBasis() * capStart, start + transform.getBasis() * capEnd, color);
	}
	
}

void DebugDrawer::drawCylinder(btScalar radius, btScalar halfHeight, int upAxis, const btTransform& transform, const btVector3& color)
{
	btVector3 start = transform.getOrigin(), offsetHeight(0,0,0);
	offsetHeight[upAxis] = halfHeight;

	btVector3 capStart(0.f,0.f,0.f);  capStart[upAxis] = -halfHeight;
	btVector3 capEnd(0.f,0.f,0.f);    capEnd[upAxis] = halfHeight;
	const btMatrix3x3& m = transform.getBasis();

	//  side lines
	int up1 = (upAxis+1)%3, up2 = (upAxis+2)%3;
	for (int i=0; i<360; i+=cylSide)
	{
		capEnd[up1] = capStart[up1] = btSin(btScalar(i)*SIMD_RADS_PER_DEG)*radius;
		capEnd[up2] = capStart[up2] = btCos(btScalar(i)*SIMD_RADS_PER_DEG)*radius;
		drawLine(start + m * capStart, start + m * capEnd, color);
	}
	//  top and bottom circles
	btVector3 yaxis(0,0,0);  yaxis[upAxis] = btScalar(1.0);
	btVector3 xaxis(0,0,0);  xaxis[up1] = btScalar(1.0);

	drawArc(start-m*offsetHeight, m*yaxis,m*xaxis, radius,radius,0,SIMD_2_PI,color,false,cylO);
	drawArc(start+m*offsetHeight, m*yaxis,m*xaxis, radius,radius,0,SIMD_2_PI,color,false,cylO);
}


}  // BtOgre
