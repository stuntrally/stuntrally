#ifndef _GraphView_H_
#define _GraphView_H_

#include <vector>

namespace Ogre {  class SceneManager;  class ManualObject;  class SceneNode;  }


class GraphView
{
public:
	GraphView(Ogre::SceneManager* pSceneMgr);
	void Create(int size), Destroy();
	
	void AddVal(float val), Update();

	std::vector<float> vals;
	int iCurX;  // cur id to insert new val

protected:
	Ogre::SceneManager* mSceneMgr;
	Ogre::ManualObject* mo;
	Ogre::SceneNode* nd;
};

#endif
