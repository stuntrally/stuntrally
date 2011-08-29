// taken from OGRE compositor sample
// only needed for HDRLogic

#ifndef _HelperLogics_H__
#define _HelperLogics_H__

#include <OgrePrerequisites.h>
#include <OgreCompositorLogic.h>
#include <OgreCompositorInstance.h>

//The simple types of compositor logics will all do the same thing -
//Attach a listener to the created compositor
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
	//This is the method that implementations will need to override
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance) = 0;
private:
	typedef std::map<Ogre::CompositorInstance*, Ogre::CompositorInstance::Listener*> ListenerMap;
	ListenerMap mListeners;

};

//The compositor logic for the hdr compositor
class HDRLogic : public ListenerFactoryLogic
{
protected:
	virtual Ogre::CompositorInstance::Listener* createListener(Ogre::CompositorInstance* instance);
};

#endif
