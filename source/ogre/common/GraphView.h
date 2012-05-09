#ifndef _GraphView_H_
#define _GraphView_H_

#include <vector>
#include <OgreString.h>
#include <MyGUI_Colour.h>

namespace Ogre  {  class SceneManager;  class ManualObject;  class SceneNode;  }
namespace MyGUI {  class Gui;  class TextBox;  }


class GraphView
{
public:
	//  ctor
	GraphView(Ogre::SceneManager* pSceneMgr,
		Ogre::RenderWindow* pWindow=0,
		MyGUI::Gui* pGui=0);

	//  Create
	void Create(int length,			// values buffer length
				Ogre::String sMtr,	// material for line
				float backAlpha);	// background transparency 0..1

	void CreateGrid(int numH,		// == horizontal lines number   1 only frame
					int numV,		// || vertical lines number		2 frame with center, 4 frame and 3 lines, etc.
					float clr,		// color (rgb gray)
					float alpha);	// transparency

	void CreateTitle(Ogre::String title, // title text
				char clr,			// text color id
				float posX,			// pos in graph rect 0..1
				char alignY,		// top:    -1 above rect, -2 in rect, -3 in rect 2nd line (below -2)
									// bottom:  1 below rect,  2 in rect,  3 in rect 2nd line (above 2)
				int fontHeight,		// in pixels eg.24
				int numLines=2,		// max number of text lines
				bool shadow=false);	// shadow under text
	void Destroy();

	//  Set position and size on screen
	void SetSize(float posX,float posY,
				float sizeX,float sizeY);  // [0..1]  0,0 is left bottom

	void SetVisible(bool visible);  // show/hide
	
	//  Update, fill
	void AddVal(float val);  // adds value at end of graph, moves left (in buffer)
	void Update();  // ogre update (on screen)

	void UpdTitle(Ogre::String title);

protected:
	std::vector<float> vals;  // values buffer
	int iCurX;  // cur id to insert new val

	Ogre::SceneManager* mSceneMgr;  // for creating
	Ogre::RenderWindow* mWindow;    // gui resolution-
	MyGUI::Gui* mGui;			    // for text only
	const static MyGUI::Colour graphClr[5+8+8];  // text colors
	
	Ogre::ManualObject* moLine, *moBack, *moGrid;  //graph line, background, grid	
	Ogre::SceneNode* node;
	void moSetup(Ogre::ManualObject* mo, bool dynamic, Ogre::uint8 RQG);  // helper

	MyGUI::TextBox* txt;
	float txPosX;  int txH, txAlignY;  // title text pos, height
};

#endif
