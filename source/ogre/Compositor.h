// adapted from OGRE compositor sample
#pragma once
#include "CGame.h"

#include <OgrePrerequisites.h>
#include <OgreCompositorLogic.h>
#include <OgreCompositorInstance.h>

// The simple types of compositor logics will all do the same thing -
// Attach a listener to the created compositor
class ListenerFactoryLogic : public Ogre::CompositorLogic
{
public:
	virtual void compositorInstanceCreated(Ogre::CompositorInstance* newInstance) 
	{
		Ogre::CompositorInstance::Listener* listener = createListener(newInstance);
		newInstance->addListener(listener);
		mListeners[newInstance] = listener;
	}
	
	virtual void compositorInstanceDestroyed(Ogre::CompositorInstance* destroyedInstance)
	{
		delete mListeners[destroyedInstance];
		mListeners.erase(destroyedInstance);
	}

protected:
	// This is the method that implementations will need to override
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance) = 0;
private:
	typedef std::map<Ogre::CompositorInstance*, Ogre::CompositorInstance::Listener*> ListenerMap;
	ListenerMap mListeners;

};
//class HDRCompositor;
// The compositor logic for the hdr compositor
class HDRLogic : public ListenerFactoryLogic
{
public:
	void setApp(BaseApp* app);
//	HDRCompositor* compositor;
protected:
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance);
private:
	BaseApp * mApp;
};

class MotionBlurLogic : public ListenerFactoryLogic
{
public:
	MotionBlurLogic(BaseApp* app);
protected:
	BaseApp* pApp;
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance);
};

class CameraBlurLogic : public ListenerFactoryLogic
{
public:
	CameraBlurLogic(BaseApp* app);
protected:
	BaseApp* pApp;
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance);
};

// The compositor logic for the ssao compositor
class SSAOLogic : public ListenerFactoryLogic
{
protected:
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance);
public:
	void setApp(BaseApp* app);
private:
	BaseApp * mApp;
};

// The compositor logic for the GodRay compositor
class GodRaysLogic : public ListenerFactoryLogic
{
protected:
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance);
public:
	void setApp(BaseApp* app);
private:
	BaseApp * mApp;
};

// The compositor logic for the gbuffer compositor
class GBufferLogic : public ListenerFactoryLogic
{
protected:
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance);
public:
	void setApp(BaseApp* app);
private:
	BaseApp * mApp;
};

// The compositor logic for the softparticle compositor
class SoftParticlesLogic : public ListenerFactoryLogic
{
protected:
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance);
public:
	void setApp(BaseApp* app);
private:
	BaseApp * mApp;
};

// The compositor logic for the DepthOfField compositor
class DepthOfFieldLogic : public ListenerFactoryLogic
{
protected:
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance);
public:
	void setApp(BaseApp* app);
private:
	BaseApp * mApp;
};

// The compositor logic for the FilmGrain compositor
class FilmGrainLogic : public ListenerFactoryLogic
{
protected:
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance);
public:
	void setApp(BaseApp* app);
private:
	BaseApp * mApp;
};
