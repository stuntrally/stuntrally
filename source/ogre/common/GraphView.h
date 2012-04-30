#ifndef _GraphView_H_
#define _GraphView_H_

#include <vector>
#include <OgreString.h>

namespace Ogre {  class SceneManager;  class ManualObject;  class SceneNode;  }


class GraphView
{
public:
	GraphView(Ogre::SceneManager* pSceneMgr);

	void Create(int size/*values length*/, Ogre::String sMtr, bool background);
	void Destroy();

	void SetSize(float posX,float posY,float sizeX,float sizeY);  // [0..1]  0,0 is left bottom
	
	void AddVal(float val);  // adds value at end of graph, moves left
	void Update();  // ogre update

protected:
	std::vector<float> vals;
	int iCurX;  // cur id to insert new val

	Ogre::SceneManager* mSceneMgr;
	Ogre::ManualObject* mo, *mb;  //o-graph, b-background
	Ogre::SceneNode* nd;
};

#endif
