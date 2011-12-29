#include "pch.h"
#include "Compositor.h"
#include "OgreGame.h"

#include <OgreCompositorInstance.h>
#include <OgreCompositorChain.h>
#include <OgreCompositionTechnique.h>
#include <OgreViewport.h>
#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgreCamera.h>
#include <OgreGpuProgramParams.h>
#include "SplitScreen.h"

class MotionBlurListener : public Ogre::CompositorInstance::Listener
{
public:
	MotionBlurListener(BaseApp* app);
	virtual ~MotionBlurListener();
	
	BaseApp* pApp;

	virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
	virtual void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
};

MotionBlurLogic::MotionBlurLogic(BaseApp* app)
{
	pApp = app;
}

Ogre::CompositorInstance::Listener* MotionBlurLogic::createListener(Ogre::CompositorInstance*  instance)
{
	MotionBlurListener* listener = new MotionBlurListener(pApp);
	return listener;
}

MotionBlurListener::MotionBlurListener(BaseApp* app) : pApp(0)
{
	pApp = app;
}

MotionBlurListener::~MotionBlurListener()
{
}

void MotionBlurListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
}

void MotionBlurListener::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
	if (pass_id != 120) return;
	//Ogre::LogManager::getSingleton().logMessage("notifyMaterialRender");
	try
	{	mat->load();
		Ogre::GpuProgramParametersSharedPtr fparams =
			mat->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
		fparams->setNamedConstant("blur", pApp->motionBlurIntensity);
	}catch(Ogre::Exception& e)
	{
		Ogre::LogManager::getSingleton().logMessage("Error setting motion blur");
	}
}


class HDRListener: public Ogre::CompositorInstance::Listener
{
protected:
	int mVpWidth, mVpHeight;
	int mBloomSize;
	// Array params - have to pack in groups of 4 since this is how Cg generates them
	// also prevents dependent texture read problems if ops don't require swizzle
	float mBloomTexWeights[15][4];
	float mBloomTexOffsetsHorz[15][4];
	float mBloomTexOffsetsVert[15][4];
public:
	HDRListener();
	virtual ~HDRListener();
	void notifyViewportSize(int width, int height);
	void notifyCompositor(Ogre::CompositorInstance* instance);
	virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
	virtual void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
};

Ogre::CompositorInstance::Listener* HDRLogic::createListener(Ogre::CompositorInstance* instance)
{
	HDRListener* listener = new HDRListener;
	Ogre::Viewport* vp = instance->getChain()->getViewport();
	listener->notifyViewportSize(vp->getActualWidth(), vp->getActualHeight());
	listener->notifyCompositor(instance);
	return listener;
}


HDRListener::HDRListener()
{
}
HDRListener::~HDRListener()
{
}

void HDRListener::notifyViewportSize(int width, int height)
{
	mVpWidth = width;
	mVpHeight = height;
}

void HDRListener::notifyCompositor(Ogre::CompositorInstance* instance)
{
	// Get some RTT dimensions for later calculations
	Ogre::CompositionTechnique::TextureDefinitionIterator defIter =
		instance->getTechnique()->getTextureDefinitionIterator();
	while (defIter.hasMoreElements())
	{
		Ogre::CompositionTechnique::TextureDefinition* def =
			defIter.getNext();
		if(def->name == "rt_bloom0")
		{
			mBloomSize = (int)def->width; // should be square
			// Calculate gaussian texture offsets & weights
			float deviation = 3.0f;
			float texelSize = 1.0f / (float)mBloomSize;

			// central sample, no offset
			mBloomTexOffsetsHorz[0][0] = 0.0f;
			mBloomTexOffsetsHorz[0][1] = 0.0f;
			mBloomTexOffsetsVert[0][0] = 0.0f;
			mBloomTexOffsetsVert[0][1] = 0.0f;
			mBloomTexWeights[0][0] = mBloomTexWeights[0][1] =
				mBloomTexWeights[0][2] = Ogre::Math::gaussianDistribution(0, 0, deviation);
			mBloomTexWeights[0][3] = 1.0f;

			// 'pre' samples
			for(int i = 1; i < 8; ++i)
			{
				mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
					mBloomTexWeights[i][2] = 1.25f * Ogre::Math::gaussianDistribution(i, 0, deviation);
				mBloomTexWeights[i][3] = 1.0f;
				mBloomTexOffsetsHorz[i][0] = i * texelSize;
				mBloomTexOffsetsHorz[i][1] = 0.0f;
				mBloomTexOffsetsVert[i][0] = 0.0f;
				mBloomTexOffsetsVert[i][1] = i * texelSize;
			}
			// 'post' samples
			for(int i = 8; i < 15; ++i)
			{
				mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
					mBloomTexWeights[i][2] = mBloomTexWeights[i - 7][0];
				mBloomTexWeights[i][3] = 1.0f;

				mBloomTexOffsetsHorz[i][0] = -mBloomTexOffsetsHorz[i - 7][0];
				mBloomTexOffsetsHorz[i][1] = 0.0f;
				mBloomTexOffsetsVert[i][0] = 0.0f;
				mBloomTexOffsetsVert[i][1] = -mBloomTexOffsetsVert[i - 7][1];
			}

		}
	}
}

void HDRListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
	//  Prepare the fragment params offsets
	switch (pass_id)
	{
	//case 994: // rt_lum4
	case 993: // rt_lum3
	case 992: // rt_lum2
	case 991: // rt_lum1
	case 990: // rt_lum0
		break;
	case 800: // rt_brightpass
		break;
	case 701: // rt_bloom1
		{
			// horizontal bloom
		try
		{	mat->load();
			Ogre::GpuProgramParametersSharedPtr fparams =
				mat->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
			fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsHorz[0], 15);
			fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);
		}catch(...)
		{	}

			break;
		}
	case 700: // rt_bloom0
		{
			// vertical bloom
		try
		{	mat->load();
			Ogre::GpuProgramParametersSharedPtr fparams =
				mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
			fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsVert[0], 15);
			fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);
		}catch(...)
		{	}

			break;
		}
	}
}

void HDRListener::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
}



class SSAOListener: public Ogre::CompositorInstance::Listener
{
protected:
public:
	SSAOListener(BaseApp * app);
	virtual ~SSAOListener();
	BaseApp * mApp;
	virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
	virtual void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
};

Ogre::CompositorInstance::Listener* SSAOLogic::createListener(Ogre::CompositorInstance* instance)
{
	SSAOListener* listener = new SSAOListener(mApp);
	Ogre::Viewport* vp = instance->getChain()->getViewport();
	return listener;
}

void SSAOLogic::setApp(BaseApp* app)
{
	mApp = app;
}


SSAOListener::SSAOListener(BaseApp* app)
	:mApp(app)
{
}
SSAOListener::~SSAOListener()
{
}

void SSAOListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
}
void SSAOListener::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
	  if (pass_id != 42) // not SSAO, return
            return;

        // this is the camera you're using
        #ifndef ROAD_EDITOR
		Ogre::Camera *cam = mApp->mSplitMgr->mCameras.front();
		#else
		Ogre::Camera *cam = mApp->mCamera;
		#endif
        // calculate the far-top-right corner in view-space
        Ogre::Vector3 farCorner = cam->getViewMatrix(true) * cam->getWorldSpaceCorners()[4];

        // get the pass
        Ogre::Pass *pass = mat->getBestTechnique()->getPass(0);

        // get the vertex shader parameters
        Ogre::GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();
        // set the camera's far-top-right corner
        if (params->_findNamedConstantDefinition("farCorner"))
            params->setNamedConstant("farCorner", farCorner);

        // get the fragment shader parameters
        params = pass->getFragmentProgramParameters();
        // set the projection matrix we need
        static const Ogre::Matrix4 CLIP_SPACE_TO_IMAGE_SPACE(
            0.5,    0,    0,  0.5,
            0,   -0.5,    0,  0.5,
            0,      0,    1,    0,
            0,      0,    0,    1);
        if (params->_findNamedConstantDefinition("ptMat"))
            params->setNamedConstant("ptMat", CLIP_SPACE_TO_IMAGE_SPACE * cam->getProjectionMatrixWithRSDepth());
        if (params->_findNamedConstantDefinition("far"))
            params->setNamedConstant("far", cam->getFarClipDistance());
}



class GodRaysListener: public Ogre::CompositorInstance::Listener
{
protected:
public:
	GodRaysListener(BaseApp * app);
	virtual ~GodRaysListener();
	BaseApp * mApp;
	virtual void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
	virtual void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);

	Ogre::Vector4 SunScreenSpacePosition;
private:
	Ogre::GpuProgramParametersSharedPtr params1;
	Ogre::GpuProgramParametersSharedPtr params2;
	Ogre::GpuProgramParametersSharedPtr params3;

};

Ogre::CompositorInstance::Listener* GodRaysLogic::createListener(Ogre::CompositorInstance* instance)
{
	GodRaysListener* listener = new GodRaysListener(mApp);
	Ogre::Viewport* vp = instance->getChain()->getViewport();
	return listener;
}

void GodRaysLogic::setApp(BaseApp* app)
{
	mApp = app;
}


GodRaysListener::GodRaysListener(BaseApp* app)
	:mApp(app)
{
	SunScreenSpacePosition = Ogre::Vector4(0,0,0,1);
}
GodRaysListener::~GodRaysListener()
{
}

void GodRaysListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
	if (pass_id == 1)
		params1 = mat->getTechnique(0)->getPass(0)->getVertexProgramParameters();
	else if (pass_id == 2)
		params2 = mat->getTechnique(0)->getPass(0)->getVertexProgramParameters();
	if (pass_id == 3)
		params3 = mat->getTechnique(0)->getPass(0)->getVertexProgramParameters();

}
void clamp(Ogre::Vector2 &v)  {
	v.x = v.x < -1 ? -1 : (v.x > 1 ? 1 : v.x);
	v.y = v.y < -1 ? -1 : (v.y > 1 ? 1 : v.y);
}

void GodRaysListener::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{

	 // this is the camera you're using
    #ifndef ROAD_EDITOR
	Ogre::Camera *cam = mApp->mSplitMgr->mCameras.front();
	#else
	Ogre::Camera *cam = mApp->mCamera;
	#endif

	//update the sun position
	Ogre::Light* sun =((App*)mApp)->sun;
	if(sun != NULL)
	{
		Ogre::Vector3 sunPosition = sun->getDirection() *100;//sun->_getDerivedOrientation() * sun->_getDerivedPosition();
		//if(cam->getPointExtrusionDistance(sun) > 0)
		{
			Ogre::Vector3 worldViewPosition = cam->getViewMatrix() * sunPosition;
			Ogre::Vector3 hcsPosition = cam->getProjectionMatrix() * worldViewPosition;
			Ogre::Vector2 sunScreenSpacePosition = Ogre::Vector2(0.5f + (0.5f * hcsPosition.x), 0.5f + (0.5f * -hcsPosition.y));
			clamp(sunScreenSpacePosition);
			SunScreenSpacePosition = Ogre::Vector4 ( sunScreenSpacePosition.x, sunScreenSpacePosition.y, 0, 1 );
		}
	//	else
		{
			//SunScreenSpacePosition =Ogre::Vector4 ( 0, 0, 0, 1 );
		}
	}
	if (pass_id == 1)
		params1->setNamedConstant("lightPosition", SunScreenSpacePosition);
	else if (pass_id == 2)
		params2->setNamedConstant("lightPosition", SunScreenSpacePosition);
	if (pass_id == 3)
		params3->setNamedConstant("lightPosition", SunScreenSpacePosition);	 
}
