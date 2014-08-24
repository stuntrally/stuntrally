#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "BaseApp.h"
#include "settings.h"
#include "../road/Road.h"

#include <OgreOverlayElement.h>
#include <OgreCamera.h>
#include <OgreRenderTarget.h>
#include <OgreRenderWindow.h>
#include <OgreTextureManager.h>
#include <OgreMeshManager.h>
#include <OgreSceneNode.h>
#include <MyGUI_TextBox.h>
using namespace Ogre;


///  Fps stats
//------------------------------------------------------------------------
void BaseApp::updateStats()
{
	//  camera pos, rot
	if (pSet->camPos)
	{
		const Vector3& pos = /*road ? road->posHit :*/ mCamera->getDerivedPosition();
		Vector3 dir = mCamera->getDirection();  //const Quaternion& rot = mCamera->getDerivedOrientation();
		String s =  TR("#{Obj_Pos}: ")+fToStr(pos.x,1)+" " + fToStr(pos.y,1) + " " + fToStr(pos.z,1)+
				TR(", | #{Obj_Rot}: ")+fToStr(dir.x,2) + " "+fToStr(dir.y,2)+" "+fToStr(dir.z,2);
		txCamPos->setCaption(s);
	}

	//  Fps
	const RenderTarget::FrameStats& stats = mWindow->getStatistics();
	size_t mem = TextureManager::getSingleton().getMemoryUsage() + MeshManager::getSingleton().getMemoryUsage();

	txFps->setCaption(
		"#E0F0FF"+(stats.lastFPS >= 200.f ? fToStr(stats.lastFPS,0,4)+"." : fToStr(stats.lastFPS,1,5))+
		"#B0C0D0"+iToStr(int(stats.triangleCount/1000.f),4)+"k"+
		" #C8E0FF"+iToStr(stats.batchCount,3)+
		" #A0B0C8"+iToStr(mem/1024/1024,3)+"M" );
}


//  rendering
//-------------------------------------------------------------------------------------
bool BaseApp::frameStarted(const FrameEvent& evt)
{	
	updateStats();

	if (ndSky)  ///o-
		ndSky->setPosition(mCamera->getPosition());

	return true;
}

bool BaseApp::frameRenderingQueued(const FrameEvent& evt)
{
	if (mWindow->isClosed())
		return false;

	if (mShutDown)
		return false;

	return true;
}

bool BaseApp::frameEnded(const FrameEvent& evt)
{
	//(void)evt;
	//updateStats();
	return true;
}

void BaseApp::UpdWireframe()
{
	mCamera->setPolygonMode(mbWireFrame ? PM_WIREFRAME : PM_SOLID);
	if (ndSky)	ndSky->setVisible(!mbWireFrame);  // hide sky
}
