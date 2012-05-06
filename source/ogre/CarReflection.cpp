#include "pch.h"
#include "common/Defines.h"
#include "CarReflection.h"
#include "../vdrift/settings.h"
#include "../ogre/common/RenderConst.h"

#include "OgreGame.h"

#include <OgreSceneManager.h>
#include <OgreLogManager.h>
#include <OgreTextureManager.h>
#include <OgreMaterialManager.h>
#include <OgreHardwarePixelBuffer.h>
using namespace Ogre;


CarReflection::CarReflection(SETTINGS* set, App* app, SceneManager* sceneMgr, unsigned int index) :
	bFirstFrame(true), iCam(0), iCounter(0)
{
	pSet = set;
	pApp = app;
	iIndex = index;
	pSceneMgr = sceneMgr;
	iCounter = pSet->refl_skip;
}

CarReflection::~CarReflection()
{
	for (int i=0; i < 6; ++i)
	{	pCams[i] = 0;  pRTs[i] = 0;  }
	
	for (int face = 0; face < 6; face++)
	{
		std::string camName = "Reflect_" + toStr(iIndex) + "_" + toStr(face);
		if (pSceneMgr->hasCamera(camName))
		{
			Camera* cam = pSceneMgr->getCamera("Reflect_" + toStr(iIndex) + "_" + toStr(face));
			pSceneMgr->destroyCamera(cam);
		}
	}

	// destroy cube tex - only if created by ourself
	if ( !(pSet->refl_mode == 1 && iIndex != 0) )
		TextureManager::getSingleton().remove(cubetexName);
}

void CarReflection::Create()
{
	bFirstFrame = true;
	if (pSet->refl_mode == 1)  cubetexName = "ReflectionCube"; // single: use 1st cubemap
	else if (pSet->refl_mode == 2)
	{
		cubetexName = "ReflectionCube" + toStr(iIndex);
		// first cubemap: no index
		if (cubetexName == "ReflectionCube0")
			cubetexName = "ReflectionCube";
	}
	else /* static */
		cubetexName = "ReflectionCube";
	
	TextureManager* tm = TextureManager::getSingletonPtr();
	int size = ciShadowSizesA[pSet->refl_size];  // /2 ?

	//  create cube render texture
	if (! (pSet->refl_mode == 1 && iIndex != 0) )
	{
		cubetex = tm->createManual(cubetexName, 
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_CUBE_MAP, 
			size,size, 0/*mips*/, PF_R8G8B8, TU_RENDERTARGET);
			//LogO("created rt cube");

		for (int face = 0; face < 6; face++)
		{
			Camera* mCam = pSceneMgr->createCamera("Reflect_" + toStr(iIndex) + "_" + toStr(face));
			mCam->setAspectRatio(1.0f);  mCam->setFOVy(Degree(90));
			mCam->setNearClipDistance(0.1);
			mCam->setFarClipDistance(pSet->refl_dist * 1.1f);

			RenderTarget* mRT = cubetex->getBuffer(face)->getRenderTarget();
			//LogO( "rt face Name: " + mRT->getName() );
			mRT->removeAllViewports();
			Viewport* vp = mRT->addViewport(mCam);
			vp->setOverlaysEnabled(false);
			vp->setVisibilityMask(RV_MaskReflect);
			vp->setShadowsEnabled(false);
			mRT->setAutoUpdated(false);
			//mRT->addListener(this);  //-
			mCam->setPosition(Vector3::ZERO);

			Vector3 lookAt(0,0,0), up(0,0,0), right(0,0,0);  switch(face)
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
	
	// Iterate through our materials and add an index to ReflectionCube texture reference
	for (int i=0; i < NumMaterials; i++)
	{
		MaterialPtr mtr = MaterialManager::getSingleton().getByName(sMtr[i]);
		if (!mtr.isNull())
		{	Material::TechniqueIterator techIt = mtr->getTechniqueIterator();
			while (techIt.hasMoreElements())
			{	Technique* tech = techIt.getNext();
				Technique::PassIterator passIt = tech->getPassIterator();
				while (passIt.hasMoreElements())
				{	Pass* pass = passIt.getNext();
					Pass::TextureUnitStateIterator tusIt = pass->getTextureUnitStateIterator();
					while (tusIt.hasMoreElements())
					{	
						TextureUnitState* tus = tusIt.getNext();
						if (tus->getTextureName() == "ReflectionCube")
							tus->setTextureName(cubetexName);
	}	}	}	}	}
}

void CarReflection::Update()
{
	// update only if we created
	if ( pSet->refl_mode == 1 && iIndex != 0 ) return;
	// static: only 1st frame
	if ( pSet->refl_mode == 0 && bFirstFrame == false ) return;
		
	//  skip frames
	if (++iCounter >= pSet->refl_skip || bFirstFrame)
	{
		iCounter = 0;
		//  cube faces at once
		int fc = bFirstFrame ? 6 : pSet->refl_faces;
		for (int i=0; i < fc; ++i)
		{
			++iCam;  if (iCam > 5)  iCam = 0;  // next

			Camera* cam = pCams[iCam];
			RenderTarget* rt = pRTs[iCam];
			
			Vector3 origScale, origPos;
			
			if (cam)
			{	
				cam->setPosition ( camPosition );
				
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

	bFirstFrame = false;
}
