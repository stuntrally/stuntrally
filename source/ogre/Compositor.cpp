#include "pch.h"
#include "Compositor.h"
#include "CGame.h"
#include "SplitScreen.h"
#include "../vdrift/settings.h"
#include "../vdrift/game.h"
#include "common/CScene.h"

#include <OgreCompositorInstance.h>
#include <OgreCompositorChain.h>
#include <OgreCompositionTechnique.h>
#include <OgreViewport.h>
#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgreCamera.h>
#include <OgreGpuProgramParams.h>
#include <OgreRenderTarget.h>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>
using namespace Ogre;


class MotionBlurListener : public CompositorInstance::Listener
{
public:
	MotionBlurListener(BaseApp* app);
	virtual ~MotionBlurListener();

	BaseApp* pApp;

	virtual void notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat);
	virtual void notifyMaterialRender(uint32 pass_id, MaterialPtr &mat);

private:
	Matrix4 mPreviousViewProjMatrix;
};

MotionBlurLogic::MotionBlurLogic(BaseApp* app)
{
	pApp = app;
}

CompositorInstance::Listener* MotionBlurLogic::createListener(CompositorInstance*  instance)
{
	MotionBlurListener* listener = new MotionBlurListener(pApp);
	return listener;
}

MotionBlurListener::MotionBlurListener(BaseApp* app) : pApp(0)
{
	mPreviousViewProjMatrix = Matrix4::IDENTITY;
	pApp = app;
}

MotionBlurListener::~MotionBlurListener()
{
}

void MotionBlurListener::notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat)
{
}

void MotionBlurListener::notifyMaterialRender(uint32 pass_id, MaterialPtr &mat)
{
	if (pass_id != 999)  return;
	try
	{
		// this is the camera you're using
		#ifndef SR_EDITOR
		Camera *cam = pApp->mSplitMgr->mCameras.front();
		#else
		Camera *cam = pApp->mCamera;
		#endif

		if (mPreviousViewProjMatrix == Matrix4::IDENTITY)
		{
			mPreviousViewProjMatrix = cam->getProjectionMatrix() * cam->getViewMatrix();
		}

		// calculate the far-top-right corner in view-space
		Vector3 farCorner = cam->getViewMatrix(true) * cam->getWorldSpaceCorners()[4];

		// get the pass
		Pass *pass = mat->getBestTechnique()->getPass(0);

		// get the vertex shader parameters
		GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();
		// set the camera's far-top-right corner
		if (params->_findNamedConstantDefinition("farCorner"))
			params->setNamedConstant("farCorner", farCorner);

		// get the fragment shader parameters
		params = pass->getFragmentProgramParameters();
		if (params->_findNamedConstantDefinition("far"))
			params->setNamedConstant("far", cam->getFarClipDistance());
		if (params->_findNamedConstantDefinition("invViewMat"))
			params->setNamedConstant("invViewMat", cam->getViewMatrix(true).inverse());
		if (params->_findNamedConstantDefinition("prevViewProjMat"))
			params->setNamedConstant("prevViewProjMat", mPreviousViewProjMatrix);
		if (params->_findNamedConstantDefinition("intensity"))
			params->setNamedConstant("intensity", pApp->pSet->blur_int);

		mPreviousViewProjMatrix = cam->getProjectionMatrix() * cam->getViewMatrix();
	}
	catch (Exception& e)
	{
		LogO("Error setting motion blur");
	}
}


class HDRListener: public CompositorInstance::Listener
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
	HDRListener(BaseApp * app);
	virtual ~HDRListener();
	void notifyViewportSize(int width, int height);
	void notifyCompositor(CompositorInstance* instance);
	virtual void notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat);
	virtual void notifyMaterialRender(uint32 pass_id, MaterialPtr &mat);
	BaseApp * mApp;
	int mViewportWidth,mViewportHeight;

};

CompositorInstance::Listener* HDRLogic::createListener(CompositorInstance* instance)
{
	HDRListener* listener = new HDRListener(mApp);
	Viewport* vp = instance->getChain()->getViewport();
	listener->notifyViewportSize(vp->getActualWidth(), vp->getActualHeight());
	listener->mViewportWidth = vp->getActualWidth();
	listener->mViewportHeight = vp->getActualHeight();
	listener->notifyCompositor(instance);
	return listener;
}
void HDRLogic::setApp(BaseApp* app)
{
	mApp = app;
}

HDRListener::HDRListener(BaseApp* app) : mApp(app)
	,mVpWidth(1024), mVpHeight(768), mBloomSize(4)
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

void HDRListener::notifyCompositor(CompositorInstance* instance)
{
	// Get some RTT dimensions for later calculations
	CompositionTechnique::TextureDefinitionIterator defIter =
		instance->getTechnique()->getTextureDefinitionIterator();
	while (defIter.hasMoreElements())
	{
		CompositionTechnique::TextureDefinition* def =
			defIter.getNext();
		if (def->name == "rt_bloom0")
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
				mBloomTexWeights[0][2] = Math::gaussianDistribution(0, 0, deviation);
			mBloomTexWeights[0][3] = 1.0f;

			// 'pre' samples
			for(int i = 1; i < 8; ++i)
			{
				mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
					mBloomTexWeights[i][2] = 1.25f * Math::gaussianDistribution(i, 0, deviation);
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

void HDRListener::notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat)
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
				GpuProgramParametersSharedPtr fparams =
					mat->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
				fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsHorz[0], 15);
				fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);
			}
			catch(...)
			{	}

			break;
		}
	case 700: // rt_bloom0
		{
			// vertical bloom
			try
			{	mat->load();
				GpuProgramParametersSharedPtr fparams =
					mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
				fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsVert[0], 15);
				fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);
			}
			catch(...)
			{	}

			break;
		}
	}
}

void HDRListener::notifyMaterialRender(uint32 pass_id, MaterialPtr &mat)
{

	if (pass_id == 600 || pass_id == 800)
	{
		Pass *pass = mat->getBestTechnique()->getPass(0);
		GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();

		if (params->_findNamedConstantDefinition("toneMapSettings"))
		{
			Vector4 toneMapSettings(1-mApp->pSet->hdrParam1, mApp->pSet->hdrParam2, mApp->pSet->hdrParam3, 1.0);
			params->setNamedConstant("toneMapSettings", toneMapSettings);
		}
		if (params->_findNamedConstantDefinition("bloomSettings"))
		{
			Vector4 bloomSettings(mApp->pSet->hdrBloomorig*2, mApp->pSet->hdrBloomint, 1.0, 1.0);
			params->setNamedConstant("bloomSettings", bloomSettings);
		}
		if (params->_findNamedConstantDefinition("vignettingSettings"))
		{
			Vector4 vignettingSettings(mApp->pSet->vignRadius, mApp->pSet->vignDarkness, 1.0, 1.0);
			params->setNamedConstant("vignettingSettings", vignettingSettings);
		}

	}
	else if(pass_id == 989)
	{
		Pass *pass = mat->getBestTechnique()->getPass(0);
		GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
		if (params->_findNamedConstantDefinition("AdaptationScale"))
		{
			params->setNamedConstant("AdaptationScale", mApp->pSet->hdrAdaptationScale);
		}
	}
}



class SSAOListener: public CompositorInstance::Listener
{
protected:
public:
	SSAOListener(BaseApp * app);
	virtual ~SSAOListener();
	BaseApp * mApp;
	virtual void notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat);
	virtual void notifyMaterialRender(uint32 pass_id, MaterialPtr &mat);
};

CompositorInstance::Listener* SSAOLogic::createListener(CompositorInstance* instance)
{
	SSAOListener* listener = new SSAOListener(mApp);
	Viewport* vp = instance->getChain()->getViewport();
	return listener;
}

void SSAOLogic::setApp(BaseApp* app)
{
	mApp = app;
}


SSAOListener::SSAOListener(BaseApp* app) : mApp(app)
{
}
SSAOListener::~SSAOListener()
{
}

void SSAOListener::notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat)
{
}
void SSAOListener::notifyMaterialRender(uint32 pass_id, MaterialPtr &mat)
{
	if (pass_id != 42) // not SSAO, return
		return;

	// this is the camera you're using
	#ifndef SR_EDITOR
	Camera *cam = mApp->mSplitMgr->mCameras.front();
	#else
	Camera *cam = mApp->mCamera;
	#endif

	// calculate the far-top-right corner in view-space
	Vector3 farCorner = cam->getViewMatrix(true) * cam->getWorldSpaceCorners()[4];

	// get the pass
	Pass *pass = mat->getBestTechnique()->getPass(0);

	// get the vertex shader parameters
	GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();
	// set the camera's far-top-right corner
	if (params->_findNamedConstantDefinition("farCorner"))
		params->setNamedConstant("farCorner", farCorner);

	// get the fragment shader parameters
	params = pass->getFragmentProgramParameters();
	// set the projection matrix we need
	static const Matrix4 CLIP_SPACE_TO_IMAGE_SPACE(
		0.5,    0,    0,  0.5,
		0,   -0.5,    0,  0.5,
		0,      0,    1,    0,
		0,      0,    0,    1);
	if (params->_findNamedConstantDefinition("ptMat"))
		params->setNamedConstant("ptMat", CLIP_SPACE_TO_IMAGE_SPACE * cam->getProjectionMatrixWithRSDepth());
	if (params->_findNamedConstantDefinition("far"))
		params->setNamedConstant("far", cam->getFarClipDistance());
}



class GodRaysListener: public CompositorInstance::Listener
{
public:
	GodRaysListener(BaseApp * app);
	virtual ~GodRaysListener();
	BaseApp * mApp;
	virtual void notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat);
	virtual void notifyMaterialRender(uint32 pass_id, MaterialPtr &mat);

	Vector4 SunScreenSpacePosition;
};

CompositorInstance::Listener* GodRaysLogic::createListener(CompositorInstance* instance)
{
	GodRaysListener* listener = new GodRaysListener(mApp);
	Viewport* vp = instance->getChain()->getViewport();
	return listener;
}

void GodRaysLogic::setApp(BaseApp* app)
{
	mApp = app;
}


GodRaysListener::GodRaysListener(BaseApp* app) : mApp(app)
{
	SunScreenSpacePosition = Vector4(0,0,0,1);
}
GodRaysListener::~GodRaysListener()
{
}

void GodRaysListener::notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat)
{
	/*	if (pass_id == 1)
	params1 = mat->getTechnique(0)->getPass(0)->getVertexProgramParameters();
	else if (pass_id == 2)
	params2 = mat->getTechnique(0)->getPass(0)->getVertexProgramParameters();
	if (pass_id == 3)
	params3 = mat->getTechnique(0)->getPass(0)->getVertexProgramParameters();
	*/
}
void clamp(Vector2 &v)  {
	v.x = v.x < -1 ? -1 : (v.x > 1 ? 1 : v.x);
	v.y = v.y < -1 ? -1 : (v.y > 1 ? 1 : v.y);
}
void clamp(Vector3 &v)  {
	v.x = v.x < -1 ? -1 : (v.x > 1 ? 1 : v.x);
	v.y = v.y < -1 ? -1 : (v.y > 1 ? 1 : v.y);
}

void GodRaysListener::notifyMaterialRender(uint32 pass_id, MaterialPtr &mat)
{
	if (pass_id !=1 && pass_id !=2 && pass_id !=3)
	{
		return;
	}
	// this is the camera you're using
	#ifndef SR_EDITOR
	Camera *cam = mApp->mSplitMgr->mCameras.front();
	#else
	Camera *cam = mApp->mCamera;
	#endif

	//update the sun position
	Light* sun = ((App*)mApp)->scn->sun;  //todo:!?
	GpuProgramParametersSharedPtr params= mat->getTechnique(0)->getPass(0)->getVertexProgramParameters();
	GpuProgramParametersSharedPtr fparams= mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
	//disable god rays when the sun is not facing us
	float enable=0.0f;
	if (sun != NULL)
	{
		Vector3 sunPosition = sun->getDirection() *100;
		Vector3 worldViewPosition = cam->getViewMatrix() * sunPosition;
		Vector3 hcsPosition = cam->getProjectionMatrix() * worldViewPosition;
		float unclampedLuminance = fabs(hcsPosition.x) + fabs(hcsPosition.y);
		clamp(hcsPosition);
		Vector2 sunScreenSpacePosition = Vector2(0.5f + (0.5f * hcsPosition.x), 0.5f + (0.5f * -hcsPosition.y));
		SunScreenSpacePosition = Vector4(sunScreenSpacePosition.x, sunScreenSpacePosition.y, 0, 1);
		enable = (1.0f / ((unclampedLuminance > 1.0f) ? unclampedLuminance : 1.0f)) * (hcsPosition.z < 1 ? 0.0f : 1.0f);
	}
	params->setNamedConstant("lightPosition", SunScreenSpacePosition);
	fparams->setNamedConstant("enableEffect", enable);
}


//  GBuffer
//----------------------------------------------------------------------------------------------------------------------------

class GBufferListener: public CompositorInstance::Listener
{
public:
	GBufferListener(BaseApp * app);
	virtual ~GBufferListener();
	virtual void notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat);
	virtual void notifyMaterialRender(uint32 pass_id, MaterialPtr &mat);
};

CompositorInstance::Listener* GBufferLogic::createListener(CompositorInstance* instance)
{
	GBufferListener* listener = new GBufferListener(mApp);
	Viewport* vp = instance->getChain()->getViewport();
	return listener;
}

GBufferListener::GBufferListener(BaseApp* app)
{
}
GBufferListener::~GBufferListener()
{
}

void GBufferLogic::setApp(BaseApp* app)
{
	mApp = app;
}


void GBufferListener::notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat)
{

}

void GBufferListener::notifyMaterialRender(uint32 pass_id, MaterialPtr &mat)
{

}


//  Soft Particles
//----------------------------------------------------------------------------------------------------------------------------

class SoftParticlesListener: public CompositorInstance::Listener
{
public:
	SoftParticlesListener(BaseApp * app);
	virtual ~SoftParticlesListener();
	virtual void notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat);
	virtual void notifyMaterialRender(uint32 pass_id, MaterialPtr &mat);
	BaseApp * mApp;
};

CompositorInstance::Listener* SoftParticlesLogic::createListener(CompositorInstance* instance)
{
	SoftParticlesListener* listener = new SoftParticlesListener(mApp);
	Viewport* vp = instance->getChain()->getViewport();

	return listener;
}

SoftParticlesListener::SoftParticlesListener(BaseApp* app) : mApp(app)
{
}
SoftParticlesListener::~SoftParticlesListener()
{
}

void SoftParticlesLogic::setApp(BaseApp* app)
{
	mApp = app;
}


void SoftParticlesListener::notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat)
{

}

void SoftParticlesListener::notifyMaterialRender(uint32 pass_id, MaterialPtr &mat)
{

}


//  Depth Of Field
//----------------------------------------------------------------------------------------------------------------------------

class DepthOfFieldListener: public CompositorInstance::Listener
{
public:
	DepthOfFieldListener(BaseApp * app);
	virtual ~DepthOfFieldListener();
	virtual void notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat);
	virtual void notifyMaterialRender(uint32 pass_id, MaterialPtr &mat);
	BaseApp * mApp;
	int mViewportWidth,mViewportHeight;
};

CompositorInstance::Listener* DepthOfFieldLogic::createListener(CompositorInstance* instance)
{
	DepthOfFieldListener* listener = new DepthOfFieldListener(mApp);
	Viewport* vp = instance->getChain()->getViewport();
	listener->mViewportWidth = vp->getActualWidth();
	listener->mViewportHeight = vp->getActualHeight();
	return listener;
}

DepthOfFieldListener::DepthOfFieldListener(BaseApp* app) : mApp(app)
	,mViewportWidth(1024), mViewportHeight(768)
{
}
DepthOfFieldListener::~DepthOfFieldListener()
{
}

void DepthOfFieldLogic::setApp(BaseApp* app)
{
	mApp = app;
}


void DepthOfFieldListener::notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat)
{
	if (pass_id == 1)
	{
		float blurScale =.5f;

		Vector4 pixelSize(1.0f / (mViewportWidth * blurScale), 1.0f / (mViewportHeight * blurScale), 0.0f, 0.0f);

		mat->load();
		Pass *pass = mat->getBestTechnique()->getPass(0);
		GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();

		if (params->_findNamedConstantDefinition("pixelSize"))
			params->setNamedConstant("pixelSize", pixelSize);

	}
	else if (pass_id == 2)
	{
		float blurScale =.5f;
		Vector4  pixelSize(1.0f / mViewportWidth, 1.0f / mViewportHeight,1.0f / (mViewportWidth * blurScale), 1.0f / (mViewportHeight * blurScale) );

		Pass *pass = mat->getBestTechnique()->getPass(0);
		GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();

		if (params->_findNamedConstantDefinition("pixelSize"))
			params->setNamedConstant("pixelSize", pixelSize);

		// this is the camera you're using
		#ifndef SR_EDITOR
		Camera *cam = mApp->mSplitMgr->mCameras.front();
		#else
		Camera *cam = mApp->mCamera;
		#endif

		if (params->_findNamedConstantDefinition("far"))
			params->setNamedConstant("far", cam->getFarClipDistance());

		if (params->_findNamedConstantDefinition("dofparams"))
		{
			Vector4 dofParams(0.0f,mApp->pSet->dof_focus,mApp->pSet->dof_far,1.0);
			params->setNamedConstant("dofparams", dofParams);
		}
	}
}


void DepthOfFieldListener::notifyMaterialRender(uint32 pass_id, MaterialPtr &mat)
{
	if(pass_id == 2)
	{
		float blurScale =.5f;
		Vector4  pixelSize(1.0f / mViewportWidth, 1.0f / mViewportHeight,1.0f / (mViewportWidth * blurScale), 1.0f / (mViewportHeight * blurScale) );

		Pass *pass = mat->getBestTechnique()->getPass(0);
		GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();

		if (params->_findNamedConstantDefinition("pixelSize"))
			params->setNamedConstant("pixelSize", pixelSize);

		// this is the camera you're using
		#ifndef SR_EDITOR
		Camera *cam = mApp->mSplitMgr->mCameras.front();
		#else
		Camera *cam = mApp->mCamera;
		#endif

		if (params->_findNamedConstantDefinition("dofparams"))
		{
			Vector4 dofParams(0.0f,mApp->pSet->dof_focus,mApp->pSet->dof_far,1.0);
			params->setNamedConstant("dofparams", dofParams);
		}
	}
}


//  Film Grain
//----------------------------------------------------------------------------------------------------------------------------

class FilmGrainListener: public CompositorInstance::Listener
{
public:
	FilmGrainListener(BaseApp * app);
	virtual ~FilmGrainListener();
	BaseApp * mApp;
	virtual void notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat);
	virtual void notifyMaterialRender(uint32 pass_id, MaterialPtr &mat);
	int mViewportWidth,mViewportHeight;
};

CompositorInstance::Listener* FilmGrainLogic::createListener(CompositorInstance* instance)
{
	FilmGrainListener* listener = new FilmGrainListener(mApp);
	Viewport* vp = instance->getChain()->getViewport();
	listener->mViewportWidth = vp->getActualWidth();
	listener->mViewportHeight = vp->getActualHeight();
	return listener;
}

void FilmGrainLogic::setApp(BaseApp* app)
{
	mApp = app;
}


FilmGrainListener::FilmGrainListener(BaseApp* app) : mApp(app)
	,mViewportWidth(1024), mViewportHeight(768)
{
}
FilmGrainListener::~FilmGrainListener()
{
}

void FilmGrainListener::notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat)
{

}


void FilmGrainListener::notifyMaterialRender(uint32 pass_id, MaterialPtr &mat)
{
	if(pass_id == 1)
	{
		float noiseIntensity = 0.1f;
		float exposure = 1-mApp->pSet->hdrParam3;
		Vector4  grainparams(1.0f / mViewportWidth, 1.0f / mViewportHeight, noiseIntensity, exposure);

		Pass *pass = mat->getBestTechnique()->getPass(0);
		GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();

		if (params->_findNamedConstantDefinition("grainparams"))
			params->setNamedConstant("grainparams", grainparams);
	}
}


//  Camera Blur (pixel, not used)
//----------------------------------------------------------------------------------------------------------------------------

class CameraBlurListener : public CompositorInstance::Listener
{
public:
	CameraBlurListener(BaseApp* app);
	virtual ~CameraBlurListener();

	App * mApp;
	Quaternion m_pPreviousOrientation;
	Vector3 m_pPreviousPosition;

	Matrix4 prevviewproj;
	bool mRequiresTextureFlipping;
	CompositorInstance*  compositorinstance;
	virtual void notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat);
	virtual void notifyMaterialRender(uint32 pass_id, MaterialPtr &mat);
};

CameraBlurLogic::CameraBlurLogic(BaseApp* app)
{
	pApp = app;
}

CompositorInstance::Listener* CameraBlurLogic::createListener(CompositorInstance*  instance)
{
	CameraBlurListener* listener = new CameraBlurListener(pApp);
	Viewport* vp = instance->getChain()->getViewport();
	listener->compositorinstance=instance;
	//	listener->mRequiresTextureFlipping  = instance->getTechnique()->getOutputTargetPass()->get("scene",0)->requiresTextureFlipping();
	return listener;
}

CameraBlurListener::CameraBlurListener(BaseApp* app) : mApp(0)
	,mRequiresTextureFlipping(0), compositorinstance(0)
{
	mApp = (App*)app;
}

CameraBlurListener::~CameraBlurListener()
{
}

void CameraBlurListener::notifyMaterialSetup(uint32 pass_id, MaterialPtr &mat)
{
}

void CameraBlurListener::notifyMaterialRender(uint32 pass_id, MaterialPtr &mat)
{
	if (pass_id == 999) 
	{
		if (mApp->pGame->pause == false)
		{
			//acquire the texture flipping attribute in the first frame
			if(compositorinstance)
			{
				mRequiresTextureFlipping  = compositorinstance->getRenderTarget("previousscene")->requiresTextureFlipping();
				compositorinstance=NULL;
			}
			// this is the camera you're using
			#ifndef SR_EDITOR
			Camera *cam = mApp->mSplitMgr->mCameras.front();
			#else
			Camera *cam = mApp->mCamera;
			#endif

			// get the pass
			Pass *pass = mat->getBestTechnique()->getPass(0);
			GpuProgramParametersSharedPtr  params = pass->getFragmentProgramParameters();

			const RenderTarget::FrameStats& stats =  mApp->getWindow()->getStatistics();
			float m_lastFPS =stats.lastFPS;

			Matrix4 projectionMatrix   = cam->getProjectionMatrix();
			if (mRequiresTextureFlipping)
			{
				// Because we're not using setProjectionMatrix, this needs to be done here
				// Invert transformed y
				projectionMatrix[1][0] = -projectionMatrix[1][0];
				projectionMatrix[1][1] = -projectionMatrix[1][1];
				projectionMatrix[1][2] = -projectionMatrix[1][2];
				projectionMatrix[1][3] = -projectionMatrix[1][3];
			}
			Matrix4 iVP = (projectionMatrix * cam->getViewMatrix()).inverse();

			if (params->_findNamedConstantDefinition("EPF_ViewProjectionInverseMatrix"))
				params->setNamedConstant("EPF_ViewProjectionInverseMatrix", iVP);
			if (params->_findNamedConstantDefinition("EPF_PreviousViewProjectionMatrix"))
				params->setNamedConstant("EPF_PreviousViewProjectionMatrix", prevviewproj);
			if (params->_findNamedConstantDefinition("intensity"))
				params->setNamedConstant("intensity", mApp->pSet->blur_int);

			float interpolationFactor = m_lastFPS * 0.03f ; //* m_timeScale m_timeScale is a multiplier to control motion blur interactively
			Quaternion current_orientation = cam->getDerivedOrientation();
			Vector3 current_position = cam->getDerivedPosition();
			Quaternion estimatedOrientation = Quaternion::Slerp(interpolationFactor, current_orientation, (m_pPreviousOrientation));
			Vector3 estimatedPosition    = (1-interpolationFactor) * current_position + interpolationFactor * (m_pPreviousPosition);
			Matrix4 prev_viewMatrix = Math::makeViewMatrix(estimatedPosition, estimatedOrientation);//.inverse().transpose();
			// compute final matrix
			prevviewproj = projectionMatrix * prev_viewMatrix;

			// update position and orientation for next update time
			m_pPreviousOrientation = current_orientation;
			m_pPreviousPosition = current_position;			
		}
	}
}
