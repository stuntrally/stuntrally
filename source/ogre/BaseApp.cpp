#include "pch.h"
#include "common/Defines.h"
#include "BaseApp.h"
#include "FollowCamera.h"
#include "../vdrift/pathmanager.h"
#include "SplitScreen.h"

#include "MyGUI_Prerequest.h"
#include "MyGUI_PointerManager.h"

#include <MyGUI.h>
#include <MyGUI_OgrePlatform.h>
using namespace Ogre;

#include "../sdl4ogre/sdlinputwrapper.hpp"


//  rendering
//-------------------------------------------------------------------------------------
bool BaseApp::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	if (mWindow->isClosed())
		return false;

	if (mShutDown)
		return false;

	//  update each device
	mInputWrapper->capture();

	mInputCtrl->update(evt.timeSinceLastFrame);
	for (int i=0; i<4; ++i)
		mInputCtrlPlayer[i]->update(evt.timeSinceLastFrame);

	   // key modifiers
	alt = mInputWrapper->isModifierHeld(SDL_Keymod(KMOD_ALT));
	ctrl = mInputWrapper->isModifierHeld(SDL_Keymod(KMOD_CTRL));
	shift = mInputWrapper->isModifierHeld(SDL_Keymod(KMOD_SHIFT));

	updateStats();
	
	// dt-
	Real time = evt.timeSinceLastFrame;
	if (time > 0.2f)  time = 0.2f;
	
	frameStart(time);
	//* */if (!frameStart())
	//	return false;
	
	return true;
}

bool BaseApp::frameEnded(const Ogre::FrameEvent& evt)
{
	// dt-
	Real time = evt.timeSinceLastFrame;
	if (time > 0.2f)  time = 0.2f;
	
	return frameEnd(time);
	//return true;
}


///  Fps stats
// ------------------------------------------------------------------------
void BaseApp::updateStats()
{
	// Print camera pos, rot

	// Only for 1 local player
	/*if (mSplitMgr && mSplitMgr->mNumViewports == 1 && mbShowCamPos)
	{
		const Vector3& pos = (*mSplitMgr->mCameras.begin())->getDerivedPosition();
		const Quaternion& rot = (*mSplitMgr->mCameras.begin())->getDerivedOrientation();
		mDebugText = "Pos: "+fToStr(pos.x,1,5)+" "+fToStr(pos.y,1,5)+" "+fToStr(pos.z,1,5);
		mDebugText += "  Rot: "+fToStr(rot.x,3,6)+" "+fToStr(rot.y,3,6)+" "+fToStr(rot.z,3,6)+" "+fToStr(rot.w,3,6);
	}/**/

	try {
		const RenderTarget::FrameStats& stats = mWindow->getStatistics();
		size_t mem = TextureManager::getSingleton().getMemoryUsage() + MeshManager::getSingleton().getMemoryUsage();

		int triCount = 0, batchCount = 0;
		if (AnyEffectEnabled())
		{
			CompositorInstance* c = NULL;
			CompositorChain* chain = CompositorManager::getSingleton().getCompositorChain (mSplitMgr->mViewports.front());

			// accumlate tris & batches from all compositors with all their render targets
			for (size_t i=0; i < chain->getNumCompositors(); ++i)
			if (chain->getCompositor(i)->getEnabled())
			{
				c = chain->getCompositor(i);
				RenderTarget* rt;
				for (size_t j = 0; j < c->getTechnique()->getNumTargetPasses(); ++j)
				{
					std::string textureName = c->getTechnique()->getTargetPass(j)->getOutputName();
					rt = c->getRenderTarget(textureName);
					triCount += rt->getTriangleCount();
					batchCount += rt->getBatchCount();
				}
			}
		}else
		{
			triCount = stats.triangleCount;
			batchCount = stats.batchCount;
		}
		/*int t = triCount, b = batchCount;

		//  add rtts
		RenderSystem::RenderTargetIterator iter = mRoot->getRenderSystem()->getRenderTargetIterator();
		while (iter.hasMoreElements())
		{
			RenderTarget* rt = iter.getNext();
			//*LogO(rt->getName());
			//if (rt->isAutoUpdated())
			{
				batchCount += rt->getBatchCount();
				triCount += rt->getTriangleCount();
			}
		}*/

		//  update
		mOvrFps->setCaption(fToStr(stats.lastFPS,1,5) );
		mOvrTris->setCaption(iToStr(int(triCount/1000.f),4)+"k");
		mOvrBat->setCaption(iToStr(batchCount,3));
		//mOvrTris->setCaption(iToStr(int(triCount/1000.f),4)+"k"+"\n"+iToStr(int(t/1000.f),4));
		//mOvrBat->setCaption(iToStr(batchCount,3)+"\n"+iToStr(batchCount,3) );
		mOvrMem->setCaption(iToStr(mem/1024/1024,3)+"M" );

		mOvrDbg->setCaption( mFilText + "  " + mDebugText );
	}
	catch(...) {  /*ignore*/  }
}


bool BaseApp::IsFocGui()
{
	return isFocGui || isFocRpl ||
		(mWndChampStage && mWndChampStage->getVisible()) ||
		(mWndChampEnd && mWndChampEnd->getVisible()) ||
		(mWndNetEnd && mWndNetEnd->getVisible()) ||
	(mWndTweak && mWndTweak->getVisible());
}

bool BaseApp::isTweak() 
{
	return mWndTweak && mWndTweak->getVisible();
}
