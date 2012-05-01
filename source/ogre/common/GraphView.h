#ifndef _GraphView_H_
#define _GraphView_H_

#include <vector>
#include <OgreString.h>

namespace Ogre {  class SceneManager;  class ManualObject;  class SceneNode;  }


class GraphView
{
public:
	GraphView(Ogre::SceneManager* pSceneMgr);

	//  Create
	void Create(int length,			// values buffer length
				Ogre::String sMtr,	// material for line
				float backAlpha);	// background transparency 0..1
	void Destroy();

	//  Set position and size on screen
	void SetSize(float posX,float posY,float sizeX,float sizeY);  // [0..1]  0,0 is left bottom
	void SetVisible(bool visible);  // show/hide
	
	//  Update, fill
	void AddVal(float val);  // adds value at end of graph, moves left
	void Update();  // ogre update

protected:
	std::vector<float> vals;  // values buffer
	int iCurX;  // cur id to insert new val

	Ogre::SceneManager* mSceneMgr;
	Ogre::ManualObject* moLine, *moBack;  //graph line, background
	Ogre::SceneNode* node;
};

#endif
