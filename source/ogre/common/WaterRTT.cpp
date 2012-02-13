#include "WaterRTT.h"

#include <OgreSceneManager.h>
#include <OgrePlane.h>
#include <OgreHardwarePixelBuffer.h>

using namespace Ogre;

WaterRTT::WaterRTT() : 
	mCamera(0), mReflectionTarget(0), mRefractionTarget(0),
	mRTTSize(512), mReflect(true), mRefract(true),
	mViewerCamera(0), mChangedSettings(1)
{
	
}

void WaterRTT::create()
{
	mCamera = mSceneMgr->createCamera("PlaneReflectionRefraction");
	if (mViewerCamera)
	{
		mCamera->setFarClipDistance(mViewerCamera->getFarClipDistance());
		mCamera->setNearClipDistance(mViewerCamera->getNearClipDistance());
	}
	
	mWaterPlane = Plane(Vector3::UNIT_Y, 0);
	
	for (unsigned int i = 0; i < 2; ++i)
	{
		TexturePtr tex = TextureManager::getSingleton().createManual(i == 0 ? "PlaneReflection" : "PlaneRefraction",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, mRTTSize, mRTTSize, 0, PF_R8G8B8, TU_RENDERTARGET);

		RenderTarget* rtt = tex->getBuffer()->getRenderTarget();
		Viewport* vp = rtt->addViewport(mCamera);
		vp->setOverlaysEnabled(false);
		vp->setShadowsEnabled(false);
		//vp->setVisibilityMask(...
		rtt->addListener(this);

		if (i == 0) mRefractionTarget = rtt;
		else mReflectionTarget = rtt;
	}
}

void WaterRTT::setViewerCamera(Ogre::Camera* cam)
{
	mViewerCamera = cam;
	if (mCamera)
	{
		mCamera->setFarClipDistance(mViewerCamera->getFarClipDistance());
		//mCamera->setNearClipDistance(mViewerCamera->getNearClipDistance());
		mCamera->setNearClipDistance(10.f); // Need to set this bit higher to hide terrain skirts
	}
}

void WaterRTT::setActive(const bool active)
{
	if (mReflectionTarget) mReflectionTarget->setActive(active);
	if (mRefractionTarget) mRefractionTarget->setActive(active);
}

void WaterRTT::destroy()
{
	if (mCamera) mSceneMgr->destroyCamera(mCamera);
	
	if (mReflectionTarget) TextureManager::getSingleton().remove("PlaneReflection");
	if (mRefractionTarget) TextureManager::getSingleton().remove("PlaneRefraction");
}

void WaterRTT::recreate()
{
	if (!mChangedSettings) return;
	mChangedSettings = false;
	destroy();
	create();
}

WaterRTT::~WaterRTT()
{
	destroy();
}

void WaterRTT::preRenderTargetUpdate(const RenderTargetEvent& evt)
{
	mCamera->setOrientation(mViewerCamera->getRealOrientation());
	mCamera->setPosition(mViewerCamera->getRealPosition());
	
	if (mSceneMgr->hasSceneNode("FluidsRootNode")) mSceneMgr->getSceneNode("FluidsRootNode")->setVisible(false);
	
	if (evt.source == mReflectionTarget)
		mCamera->enableReflection(mWaterPlane);
}

void WaterRTT::postRenderTargetUpdate(const RenderTargetEvent& evt)
{
	if (mSceneMgr->hasSceneNode("FluidsRootNode")) mSceneMgr->getSceneNode("FluidsRootNode")->setVisible(true);

	
	if (evt.source == mReflectionTarget)
		mCamera->disableReflection();
}
