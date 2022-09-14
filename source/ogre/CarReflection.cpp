#include "pch.h"
#include "common/Def_Str.h"
#include "CarReflection.h"
#include "../settings.h"
#include "../ogre/common/RenderConst.h"

#include "CGame.h"

#include <OgreSceneManager.h>
#include <OgreLogManager.h>
#include <OgreTextureManager.h>
#include <OgreMaterialManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreCamera.h>
#include <OgreRenderTarget.h>
#include <OgreViewport.h>
#include <OgreSceneNode.h>
using namespace Ogre;


CarReflection::CarReflection(SETTINGS* set, App* app, SceneManager* sceneMgr, unsigned int index, char suffix)
	: iCam(0), iCounter(0)
{
	pSet = set;  pApp = app;
	iIndex = index;  hSuffix = suffix;
	pSceneMgr = sceneMgr;
	iCounter = pSet->refl_skip;

	for (int i=0; i < 6; ++i)
	{	pCams[i] = 0;  pRTs[i] = 0;  }
}

CarReflection::~CarReflection()
{
	for (int i=0; i < 6; ++i)
	{	pCams[i] = 0;  pRTs[i] = 0;  }
	
	for (size_t i=0; i < vCams.size(); ++i)
	{
		pSceneMgr->destroyCamera(vCams[i]);
		//LogO("dest REFL_CAM: ");
	}
	vCams.clear();

	// destroy cube tex - only if created by ourself
	if ( !(pSet->refl_mode == 1 && iIndex != 0) )
		if (TextureManager::getSingleton().resourceExists(cubetexName))
			TextureManager::getSingleton().remove(cubetexName);
}

void CarReflection::Create()
{
	//bFirstFrame = true;
	if (pSet->refl_mode == 1)  cubetexName = "ReflectionCube"; // single: use 1st cubemap
	else if (pSet->refl_mode == 2)
	{
		cubetexName = "ReflectionCube" + toStr(iIndex);
		// first cubemap: no index
		if (cubetexName == "ReflectionCube0")
			cubetexName = "ReflectionCube";
	}
	else /* 0 static */
		cubetexName = "ReflectionCube";
	
	TextureManager* tm = TextureManager::getSingletonPtr();
	int size = ciShadowSizesA[pSet->refl_size];  // /2 ?

	//  create cube render texture
	if ( !(pSet->refl_mode == 1 && iIndex != 0) )
	{
		cubetex = tm->createManual(cubetexName, 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_CUBE_MAP, 
			size,size, 0/*mips*/, PF_R8G8B8, TU_RENDERTARGET);
			//LogO("created rt cube");

		for (int face = 0; face < 6; ++face)
		{
			String name = "Reflect_" + toStr(iIndex) + hSuffix + "_" + toStr(face);
			//LogO("REFL_CAM: " + name);
			Camera* mCam = pSceneMgr->createCamera(name);
			vCams.push_back(mCam);
			mCam->setAspectRatio(1.0f);  mCam->setFOVy(Degree(90));
			mCam->setNearClipDistance(0.1);
			mCam->setFarClipDistance(pSet->refl_dist * 1.1f);

			RenderTarget* mRT = (RenderTarget*)cubetex->getBuffer(face)->getRenderTarget();
			//LogO( "rt face Name: " + mRT->getName() );
			mRT->removeAllViewports();
			Viewport* vp = mRT->addViewport(mCam);
			vp->setOverlaysEnabled(false);
			vp->setVisibilityMask(RV_MaskReflect);
			vp->setShadowsEnabled(false);
			vp->setMaterialScheme ("reflection");
			mRT->setAutoUpdated(false);
			//mRT->addListener(this);  //-
			mCam->setPosition(Vector3::ZERO);

			Vector3 lookAt(0,0,0), up(0,0,0), right(0,0,0);
			switch (face)
			{
				case 0:  lookAt.x =-1;  up.y = 1;  right.z = 1;  break;  // +X
				case 1:  lookAt.x = 1;  up.y = 1;  right.z =-1;  break;	 // -X
				case 2:  lookAt.y =-1;  up.z = 1;  right.x = 1;  break;	 // +Y
				case 3:  lookAt.y = 1;  up.z =-1;  right.x = 1;  break;	 // -Y
				case 4:  lookAt.z = 1;  up.y = 1;  right.x =-1;  break;	 // +Z
				case 5:  lookAt.z =-1;  up.y = 1;  right.x =-1;  break;	 // -Z
			}
			Quaternion orient( right, up, lookAt );  mCam->setOrientation( orient );
			pCams[face] = mCam;
			pRTs[face] = mRT;
		}
	}
}

void CarReflection::Update(bool first)
{
	if (!first)
	{
		// update only if we created
		if (pSet->refl_mode == 1 && iIndex != 0)  return;
		// static: only 1st frame
		if (pSet->refl_mode == 0)  return;
	}
		
	//  skip frames
	if (++iCounter >= pSet->refl_skip || first)
	{
		iCounter = 0;
		//  all cube faces at once
		int fc = first ? 6 : pSet->refl_faces;
		for (int i=0; i < fc; ++i)
		{
			++iCam;  if (iCam > 5)  iCam = 0;  // next

			Camera* cam = pCams[iCam];
			RenderTarget* rt = pRTs[iCam];
			
			Vector3 origScale, origPos;
			
			if (cam)
			{	
				cam->setPosition( camPosition );
				
				// Set skydome position to camera
				if (pApp->ndSky)
				{
					origScale = pApp->ndSky->getScale();
					origPos = pApp->ndSky->getPosition();
					
					pApp->ndSky->setScale(pSet->refl_dist * Vector3::UNIT_SCALE);
					pApp->ndSky->setPosition(camPosition);
				}
			}
				//else  LogO("upd cam 0");
			if (rt)  rt->update();
				//else  LogO("upd rt 0");
				
			// restore previous skydome position
			if (pApp->ndSky)
			{				
				pApp->ndSky->setScale(origScale);
				pApp->ndSky->setPosition(origPos);
			}
		}
	}

	//Image im;
	//cubetex->convertToImage(im);
	//im.save("cube.dds");
}
