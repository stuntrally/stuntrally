#pragma once
#include <vector>
#include <OgreString.h>

namespace Ogre {  class InstanceManager;  class InstancedEntity;  class Entity;  class SceneManager;  }


///  HW instancing  ---- ----
class InstSub
{
public:
	class Ogre::InstanceManager* instMgr;
	std::vector<Ogre::InstancedEntity*> ents;

	InstSub()
		: instMgr(0)
	{  }
};

class InstMesh
{
public:
	std::vector<InstSub> subs;
	InstMesh()
	{  }
};

class Instanced
{
	std::vector<InstMesh> inst;
public:
	void Create(Ogre::SceneManager* mSceneMgr, Ogre::String sMesh);
};
