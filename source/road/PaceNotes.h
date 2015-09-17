#pragma once
#include <vector>
#include <OgreString.h>
#include <OgreVector3.h>

namespace Ogre {  class SceneManager;  class SceneNode;  class Entity;  class Terrain;  class Camera;
	class BillboardSet;  class Billboard;  class RenderWindow;  }
namespace MyGUI {  class TextBox;  class Gui;  }
class SplineRoad;  class Scene;  class SETTINGS;

#if 0
enum PaceTypes                 // 90     // 180    // 270
{	P_1=0, P_2, P_3, P_4, P_5, P_6sq, P_7hrp, P_8u, P_9o,  // turns
	P_Loop, P_LoopBig, P_LoopSide, P_LoopBarrel,  // loops
	P_Jump, P_JumpTer,  // jumps
	P_OnPipe,
	P_Bumps,
	//  manual..
	P_Slow, P_Stop, P_Danger,   // brake, warn
	P_Obstacle, P_Narrow, P_Split,
	P_Ice, P_Mud, P_Water,  //..
	Pace_ALL
};
#endif

struct PaceNote
{
	//  ogre
	Ogre::SceneNode* nd;
	Ogre::BillboardSet* bb;
	Ogre::Billboard* bc;
	MyGUI::TextBox* txt;  // text for jmp vel

	//  data
	Ogre::Vector3 pos;
	Ogre::Vector4 clr;
	Ogre::Vector2 size, ofs,uv;
	int use;  // 1 normal  2 dbg start 3 dbg cont  4 bar  5 trk gho
	int id;
	bool start;  // start pos only
	int jump;  // 0 none 1 jump 2 land

	//PaceTypes type;
	//int dir;  // -1 left, 1 right
	float vel;  // for jump
	bool text;

	PaceNote();
	PaceNote(int i,int t, Ogre::Vector3 p, float sx,float sy,
		float r,float g,float b,float a, float ox,float oy, float u,float v);
};


class PaceNotes
{
public:
	SETTINGS* pSet;  ///*
	PaceNotes(SETTINGS* pset);
	//void Defaults();

	//  Setup, call this on Init
	void Setup(Ogre::SceneManager* sceneMgr, Ogre::Camera* camera,
		Ogre::Terrain* terrain, MyGUI::Gui* gui, Ogre::RenderWindow* window);

	//  Rebuild
	void Rebuild(SplineRoad* road, Scene* sc, bool reversed);
	void Destroy(), Destroy(PaceNote& n);
	void Create(PaceNote& n);
	void Update(PaceNote& n), UpdateTxt(PaceNote& n), UpdTxt();
	void Reset();
	
	//  pacenotes.xml
	bool LoadFile(Ogre::String fname), SaveFile(Ogre::String fname);

	//  Update
	void UpdVis(Ogre::Vector3 carPos, bool hide=false);
	void updTxt(PaceNote& n, bool vis);

	//  edit ..
	//void Pick(Ogre::Camera* mCamera, Ogre::Real mx, Ogre::Real my,
	//		bool bRay=true, bool bAddH=false, bool bHide=false);
	//void Select(bool bHide=false);

	//  Insert  -------
	//void Insert(), Delete();

private:
	//  ogre vars
	Ogre::SceneManager* mSceneMgr;
	Ogre::Camera* mCamera;
	Ogre::Terrain* mTerrain;
	MyGUI::Gui* mGui;
	Ogre::RenderWindow* mWindow;
	
	//  all notes
	std::vector<PaceNote> vPN,  // all incl. debug
		vPS;  // game, signs only, sorted by id
	int ii;  // id for names
public:
	int iStart;  // vPS id of track start
	int iAll;  // all road markers from road->vPace
	int iDir;  // copy from road
	int iCur;  // cur car pace id, for tracking
	float carVel;
	bool rewind;
};


static bool PaceSort(const PaceNote& a, const PaceNote& b)
{
	return a.id < b.id;
}
