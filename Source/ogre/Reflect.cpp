#include "stdafx.h"
#include "OgreGame.h"
#include "../vdrift/settings.h"


#define  Log(s)  //Ogre::LogManager::getSingleton().logMessage(String("^^^ Refl:  ") + s)

///  Reflection Cameras
//---------------------------------------------------------------------------------------------------------------
void App::createReflectCams()
{	
	reflAct = false;//
	for (int i=0; i < 6; ++i)
	{	mReflectCams[i] = 0;  mReflectRT[i] = 0;  }

	Ogre::TextureManager* tm = Ogre::TextureManager::getSingletonPtr();
	int size = ciShadowSizesA[pSet->refl_size];

	//  create cube render texture
	cubetex = tm->createManual("ReflectionCube",
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_CUBE_MAP, 
		size,size, 0/*mips*/, PF_R8G8B8, TU_RENDERTARGET);
		Log("created rt cube");

	for (int face = 0; face < 6; face++)
	{
		Camera* mCam = mSceneMgr->createCamera("Reflect_" + toStr(face));
		mCam->setAspectRatio(1.0f);  mCam->setFOVy(Degree(90));
		mCam->setNearClipDistance(0.1);
		//mCam->setFarClipDistance(pSet->refl_dist);  //sky-

		RenderTarget* mRT = cubetex->getBuffer(face)->getRenderTarget();
		Log( "rt face Name: " + mRT->getName() );
		mRT->removeAllViewports();
		Viewport* vp = mRT->addViewport(mCam);
		vp->setOverlaysEnabled(false);
		vp->setVisibilityMask(1+4+8);  // hide 2: hud, car,glass,tires
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
		mReflectCams[face] = mCam;
		mReflectRT[face] = mRT;
	}
	reflAct = true;
}

void App::destroyReflectCams()
{
	bool dn = reflAct;  // was inited
	reflAct = false;

	for (int i=0; i < 6; ++i)
	{	mReflectCams[i] = 0;  mReflectRT[i] = 0;  }
	
	for (int face = 0; face < 6; face++)
	{
		try{
			Camera* cam = mSceneMgr->getCamera("Reflect_" + toStr(face));
			if (cam) {	mSceneMgr->destroyCamera(cam);
				Log("destroy refl cam ok");  }
		}catch(...) {
			Log("destroy refl cam err");  }
	}
	/*while (mReflectCams.size() > 0)  // old
	{
		ReflectCam& it = mReflectCams.back();
		mSceneMgr->destroyCamera( it.mCam->getName() );
		//mRoot->getRenderSystem()->destroyRenderTexture( it.mRT->getName() );  //!err
		mReflectCams.pop_back();
	}/**/
}

///  Update Reflection
//---------------------------------------------------------------------------------------------------------------
void App::updateReflection()
{
	if (bLoading)  {  //Log("update - loading");
		return;  }
	if (!reflAct)  {  //Log("update - not active");
		return;  }

	//  skip frames
	if (--miReflectCntr <= 0)
	{
		miReflectCntr = pSet->refl_skip;
		//  cube faces at once
		int fc = mReflAll1st ? 6 : pSet->refl_faces;
		mReflAll1st = false;
		if (ndCar)
		for (int i=0; i < fc; ++i)
		{
			++miReflectCam;  if (miReflectCam > 5)  miReflectCam = 0;  // next

			Camera* cam = mReflectCams[miReflectCam];
			RenderTarget* rt = mReflectRT[miReflectCam];

			if (cam) cam->setPosition( ndCar->getPosition() );
				else  Log("upd cam 0");
			if (rt)  rt->update();
				else  Log("upd rt 0");
		}
	}
}

/*
void OgreGame::preRenderTargetUpdate(const RenderTargetEvent& evt)
{
	//  no shadows in reflections-
	//mOldShadowTech = mSceneMgr->getShadowTechnique();
	//mSceneMgr->setShadowTechnique( SHADOWTYPE_NONE );
	
	// hide what we don't want to render
	Vector3 scl = pSet->refl_dist*Vector3::UNIT_SCALE;
	//ndSky->setScale(scl*0.97);
}


void OgreGame::postRenderTargetUpdate(const RenderTargetEvent& evt)
{
	// restore shadows
	//mSceneMgr->setShadowTechnique( mOldShadowTech );

	//  show what we hid before
	Vector3 scl = pSet->view_distance*Vector3::UNIT_SCALE;
	//ndSky->setScale(scl);
}
/**/