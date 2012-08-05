#include <OgrePlane.h>
#include <OgreRenderTargetListener.h>
#include <OgreRenderTarget.h>

namespace Ogre { class Camera;  class SceneManager; }

/*
 * Class that handles RenderTargets for water reflection and refraction.
 * Could also be used for other types of planar reflections (e.g. a mirror)
 */

class WaterRTT : public Ogre::RenderTargetListener
{
public:
	WaterRTT();
	~WaterRTT();
	
	void create();
	void destroy();
	void recreate();
	
	void setRTTSize(const unsigned int size) { if (mRTTSize != size) { mChangedSettings=true; mRTTSize = size; } };
	void setReflect(const bool reflect) { if (mReflect != reflect) { mChangedSettings=true; mReflect = reflect; } };
	void setRefract(const bool refract) { if (mRefract != refract) { mChangedSettings=true; mRefract = refract; } };
	
	void setActive(const bool active);
	void setPlane(const Ogre::Plane& plane) { mWaterPlane = plane; };
	
	void setViewerCamera(Ogre::Camera* cam);
	
	Ogre::SceneManager* mSceneMgr;
	Ogre::SceneNode* mNdFluidsRoot;
	
private:
	Ogre::RenderTarget* mReflectionTarget;
	Ogre::RenderTarget* mRefractionTarget;
	
	Ogre::Plane mWaterPlane;

	Ogre::Camera* mViewerCamera;
	Ogre::Camera* mCamera;
	
	unsigned int mRTTSize;
	
	bool mReflect;
	bool mRefract;
	
	bool mChangedSettings;
	
protected:
	void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
	void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
};
