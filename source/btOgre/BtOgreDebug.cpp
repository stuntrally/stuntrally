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

//  draw line
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

//  draw sphere
void DebugDrawer::drawSphere(btScalar radius, const btTransform& transform, const btVector3& color, int half)
{
	btVector3 p = transform.getOrigin();
	const btMatrix3x3& m = transform.getBasis();

	const btVector3 x = m * btVector3(radius,0,0);
	const btVector3 y = m * btVector3(0,radius,0);
	const btVector3 z = m * btVector3(0,0,radius);

	const float PI2 = 2*M_PI, ad = M_PI/15.f;
	{
		btVector3 p1, p1o = p + y, p2o = p + x, p3o = p + y;
		for (float a = ad; a <= PI2+ad; a += ad)
		{
			float s = sinf(a), c = cosf(a);
			p1 = p - s*x + c*y;  drawLine(p1o, p1, color);	p1o = p1;
			p1 = p - s*z + c*x;  drawLine(p2o, p1, color);	p2o = p1;
			p1 = p - s*z + c*y;  drawLine(p3o, p1, color);	p3o = p1;
		}
	}
}

void DebugDrawer::drawCircleZ(btScalar radius, const btVector3& p, const btVector3& x, const btVector3& y, const btVector3& color)
{
	const float PI2 = 2*M_PI, ad = M_PI/15.f;
	btVector3 p1, p1o = p + y;
	for (float a = ad; a <= PI2+ad; a += ad)
	{
		float s = sinf(a), c = cosf(a);
		p1 = p - s*x + c*y;  drawLine(p1o, p1, color);	p1o = p1;
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
	if (m_debugMode==0)
		mLineDrawer->clear();
}
	
int	DebugDrawer::getDebugMode() const
{
	return m_debugMode;
}

}
