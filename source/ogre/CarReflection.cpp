#include "stdafx.h"
#include "CarReflection.h"

CarReflection::CarReflection(SETTINGS* set, Ogre::SceneManager* sceneMgr, unsigned int index) :
	bFirstFrame(true), iCam(0), iCounter(0)
{
	pSet = set;
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
		try{
			Camera* cam = pSceneMgr->getCamera("Reflect_" + toStr(iIndex) + "_" + toStr(face));
			if (cam) {	pSceneMgr->destroyCamera(cam);
				Log("destroy refl cam ok");  }
		}catch(...) {
			Log("destroy refl cam err");  }
	}

	// destroy cube tex
	Ogre::TextureManager::getSingleton().remove(cubetexName);
}

void CarReflection::Create()
{
	///TODO (optional) only one cubemap, no cubemaps (static) 
	
	cubetexName = "ReflectionCube" + toStr(iIndex);
	// first cubemap: no index
	if (cubetexName == "ReflectionCube0")
		cubetexName = "ReflectionCube";
	
	Ogre::TextureManager* tm = Ogre::TextureManager::getSingletonPtr();
	int size = ciShadowSizesA[pSet->refl_size];

	//  create cube render texture
	cubetex = tm->createManual(cubetexName, 
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_CUBE_MAP, 
		size,size, 0/*mips*/, PF_R8G8B8, TU_RENDERTARGET);
		Log("created rt cube");

	for (int face = 0; face < 6; face++)
	{
		Camera* mCam = pSceneMgr->createCamera("Reflect_" + toStr(iIndex) + "_" + toStr(face));
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
		pCams[face] = mCam;
		pRTs[face] = mRT;
	}
	
	// Iterate through our materials and add an index to ReflectionCube texture reference
	for (int i=0; i < NumMaterials; i++)
	{
		MaterialPtr mtr = (MaterialPtr)MaterialManager::getSingleton().getByName(sMtr[i]);
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
					}
				}	
			}
		}	
	}

}

void CarReflection::Update()
{
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

			if (cam) cam->setPosition ( camPosition );
				else  Log("upd cam 0");
			if (rt)  rt->update();
				else  Log("upd rt 0");
		}
	}
	bFirstFrame = false;
}
