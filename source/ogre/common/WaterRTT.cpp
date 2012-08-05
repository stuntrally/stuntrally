#include "pch.h"
#include "WaterRTT.h"

#include "RenderConst.h"

#include <OgreSceneManager.h>
#include <OgrePlane.h>
#include <OgreHardwarePixelBuffer.h>

#include "../../shiny/Main/Factory.hpp"

using namespace Ogre;

WaterRTT::WaterRTT() : 
	mCamera(0), mReflectionTarget(0),
	mRTTSize(512), mReflect(true),
	mViewerCamera(0), mChangedSettings(1), mSceneMgr(0),
	mNdFluidsRoot(0)
{
	mWaterPlane = Plane(Vector3::UNIT_Y, 0);
}

void WaterRTT::create()
{
	if (!mSceneMgr)  return;
	mCamera = mSceneMgr->createCamera("PlaneReflection");
	if (mViewerCamera)
	{
		mCamera->setFarClipDistance(mViewerCamera->getFarClipDistance());
		mCamera->setNearClipDistance(mViewerCamera->getNearClipDistance());
		mCamera->setAspectRatio(mViewerCamera->getAspectRatio());
	}
			
	TexturePtr tex = TextureManager::getSingleton().createManual("PlaneReflection",
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, mRTTSize, mRTTSize, 0, PF_R8G8B8, TU_RENDERTARGET);

	RenderTarget* rtt = tex->getBuffer()->getRenderTarget();
	Viewport* vp = rtt->addViewport(mCamera);
	vp->setOverlaysEnabled(false);
	vp->setBackgroundColour(ColourValue(0.8f, 0.9f, 1.0f));
	vp->setShadowsEnabled(false);
	vp->setMaterialScheme ("reflection");
	vp->setVisibilityMask(RV_WaterReflect);
	rtt->addListener(this);

	mReflectionTarget = rtt;

	sh::Factory::getInstance ().setTextureAlias ("WaterReflection", "PlaneReflection");
}

void WaterRTT::setViewerCamera(Ogre::Camera* cam)
{
	mViewerCamera = cam;
	if (mCamera)
	{
		mCamera->setFarClipDistance(mViewerCamera->getFarClipDistance());
		mCamera->setNearClipDistance(mViewerCamera->getNearClipDistance());
		mCamera->setAspectRatio(mViewerCamera->getAspectRatio());
	}
}

void WaterRTT::setActive(const bool active)
{
	if (mReflectionTarget) mReflectionTarget->setActive(active);
}

void WaterRTT::destroy()
{
	if (mCamera && mSceneMgr) mSceneMgr->destroyCamera(mCamera);
	
	if (mReflectionTarget)
	{
		TextureManager::getSingleton().remove("PlaneReflection");
		mReflectionTarget = 0;
	}
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
	
	if (mNdFluidsRoot)  mNdFluidsRoot->setVisible(false);
		
	if (evt.source == mReflectionTarget)
	{
		if (mCamera->getPosition ().y > -mWaterPlane.d)
			mCamera->enableCustomNearClipPlane(mWaterPlane);
		mCamera->enableReflection(mWaterPlane);
	}
}

void WaterRTT::postRenderTargetUpdate(const RenderTargetEvent& evt)
{
	if (mNdFluidsRoot)  mNdFluidsRoot->setVisible(true);
	
	if (evt.source == mReflectionTarget)
	{
		mCamera->disableReflection();
		mCamera->disableCustomNearClipPlane();
	}
}
