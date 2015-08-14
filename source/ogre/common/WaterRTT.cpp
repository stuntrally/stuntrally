#include "pch.h"
#include "WaterRTT.h"
#include "RenderConst.h"

#include <OgreSceneManager.h>
#include <OgrePlane.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreCamera.h>
#include <OgreTextureManager.h>
#include <OgreRenderTexture.h>
#include <OgreViewport.h>
#include <OgreSceneNode.h>

#include "../../shiny/Main/Factory.hpp"
using namespace Ogre;


WaterRTT::WaterRTT() : 
	mCamera(0), mReflectionTarget(0), mRefractionTarget(0),
	mRTTSize(512), mReflect(true), mRefract(true),
	mViewerCamera(0), mChangedSettings(1), mSceneMgr(0),
	mNdFluidsRoot(0)
{
	mWaterPlane = Plane(Vector3::UNIT_Y, 0);
}

void WaterRTT::create()
{
	if (!mSceneMgr)  return;
	mCamera = mSceneMgr->createCamera("PlaneReflectionRefraction");
	if (mViewerCamera)
	{
		mCamera->setFarClipDistance(mViewerCamera->getFarClipDistance());
		mCamera->setNearClipDistance(mViewerCamera->getNearClipDistance());
		mCamera->setAspectRatio(mViewerCamera->getAspectRatio());
	}
	
	for (int i = 0; i < 2; ++i)
	{
		if (i==0 && !mReflect) continue;
		if (i==1 && !mRefract) continue;
		
		TexturePtr tex = TextureManager::getSingleton().createManual(
			i == 0 ? "PlaneReflection" : "PlaneRefraction", rgDef, TEX_TYPE_2D,
			mRTTSize, mRTTSize, 0, PF_R8G8B8, TU_RENDERTARGET);

		RenderTarget* rtt = tex->getBuffer()->getRenderTarget();
		Viewport* vp = rtt->addViewport(mCamera);
		vp->setOverlaysEnabled(false);
		vp->setBackgroundColour(ColourValue(0.8f, 0.9f, 1.0f));
		vp->setShadowsEnabled(false);
		vp->setMaterialScheme("reflection");
		vp->setVisibilityMask(i == 0 ? RV_WaterReflect : RV_WaterRefract);
		rtt->addListener(this);

		if (i == 0)  mReflectionTarget = rtt;
		else  mRefractionTarget = rtt;
	}

	sh::Factory::getInstance().setTextureAlias("WaterReflection", "PlaneReflection");
	sh::Factory::getInstance().setTextureAlias("WaterRefraction", "PlaneRefraction");
}

void WaterRTT::setViewerCamera(Camera* cam)
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
	if (mRefractionTarget) mRefractionTarget->setActive(active);
}

void WaterRTT::destroy()
{
	if (mReflectionTarget)
	{
		mReflectionTarget->removeAllListeners();
		mReflectionTarget->removeAllViewports();
		TextureManager::getSingleton().remove("PlaneReflection");
		mReflectionTarget = 0;
	}
	if (mRefractionTarget)
	{
		mRefractionTarget->removeAllListeners();
		mRefractionTarget->removeAllViewports();
		TextureManager::getSingleton().remove("PlaneRefraction");
		mRefractionTarget = 0;
	}
	if (mCamera && mSceneMgr)
	{
		mSceneMgr->destroyCamera(mCamera);
		mCamera = 0;
	}
}

void WaterRTT::recreate()
{
	if (!mChangedSettings)  return;
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
		if (mCamera->getPosition().y > -mWaterPlane.d)
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
