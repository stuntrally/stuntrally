#include "pch.h"
#include "../ogre/common/Defines.h"
#include "BaseApp.h"
#include "../road/Road.h"
using namespace Ogre;


///  Fps stats
//------------------------------------------------------------------------
void BaseApp::updateStats()
{
	//  Focus  * * *
	static int fcOld = -2;
	int fc = bGuiFocus ? 2 : bMoveCam ? 0 : 1;
	if (ovFocus && fc != fcOld)
	{	if (fcOld < 0)  ++fcOld;
		else  fcOld = fc;
		
		const char* sFoc[3] = {"Cam", " Edit", "  Gui"};
		ColourValue cFoc[3] = {ColourValue(0.7,0.85,1.0), ColourValue(0.7,1.0,0.5), ColourValue(1.0,1.0,0.4)};
		const char* mFoc[3] = {"Cam", "Edit", "Gui"};

		ovFocus->setCaption(sFoc[fc]);
		ovFocus->setColour(cFoc[fc]);  if (ovFocBck)
		ovFocBck->setMaterialName(String("Menu/Focus") + mFoc[fc]);
	}

	//  camera pos, rot
	if (pSet->camPos)
	{
		const Vector3& pos = /*road ? road->posHit :*/ mCamera->getDerivedPosition();
		Vector3 dir = mCamera->getDirection();  //const Quaternion& rot = mCamera->getDerivedOrientation();
		String s = " Pos: "+fToStr(pos.x,1)+" " + fToStr(pos.y,1) + " " + fToStr(pos.z,1)
				+", | Dir: "+fToStr(dir.x,2) + " "+fToStr(dir.y,2)+" "+fToStr(dir.z,2);
		ovPos->setCaption(s);
	}

	//  Fps, Tri, Bat
	{	const RenderTarget::FrameStats& stats = mWindow->getStatistics();
		size_t mem = TextureManager::getSingleton().getMemoryUsage() + MeshManager::getSingleton().getMemoryUsage();

		ovFps->setCaption(fToStr(stats.lastFPS,1,5) );
		ovTri->setCaption(iToStr(int(stats.triangleCount/1000.f),4)+"k");
		ovBat->setCaption(iToStr(stats.batchCount,3) );
		ovMem->setCaption(iToStr(mem/1024/1024,3)+"M" );
	}
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
