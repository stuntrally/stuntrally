#pragma once

#include <vector>
#include <OgreString.h>
#include <OgreVector3.h>

#include <OgreMesh.h>
#include <OgreAxisAlignedBox.h>

namespace Ogre {  class SceneManager;  class SceneNode;  class Entity;  class Terrain;  class Camera;  }
class SplineRoad;  class Scene;


enum PaceTypes                 // 90     // 180    // 270
{	P_1=0, P_2, P_3, P_4, P_5, P_6sq, P_7hrp, P_8u, P_9o,  // turns
	P_Loop, P_Loop2, P_SideLoop, P_LoopBarrel,  // loops
	P_Jump, P_JumpTer,  // jumps
	P_OnPipe,
	P_Bumps,
	//  manual..
	P_Slow, P_Stop, P_Danger,   // brake, warn
	P_Narrow, P_Obstacle, P_Split,
	P_Ice, P_Mud, P_Water,  //..
	Pace_ALL
};

struct PaceNote
{
	Ogre::SceneNode* nd;
	Ogre::BillboardSet* bb;

	Ogre::Vector3 pos;
	Ogre::Vector4 clr;
	Ogre::Vector2 size, ofs,uv;
	int use;  // 1 normal  2 dbg start 3 dbg cont  4 bar  5 trk gho
	int id;
	bool start;  // start pos only

	//PaceTypes type;
	//int dir;  // -1 left, 1 right
	float vel;  // for jump

	PaceNote();
	PaceNote(int i,int t, Ogre::Vector3 p, float sx,float sy,
		float r,float g,float b,float a, float ox,float oy, float u,float v);
};


class PaceNotes
{
public:
	class App* pApp;  ///*
	PaceNotes(App* papp);
	//void Defaults();

	//  Setup, call this on Init
	void Setup(Ogre::SceneManager* sceneMgr, Ogre::Camera* camera, Ogre::Terrain* terrain);

	//  Rebuild
	void Rebuild(SplineRoad* road, Scene* sc, bool reversed);
	void Destroy(),  Create(PaceNote& n), Destroy(PaceNote& n);
	
	//  pacenotes.xml
	bool LoadFile(Ogre::String fname), SaveFile(Ogre::String fname);

	//  Update
	void UpdVis(Ogre::Vector3 carPos, bool hide=false);

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
	
	//  all notes
	std::vector<PaceNote> vPN, vPS;  // sorted
	int ii;  // id for names
public:
	int iStart;  // vPN id closest to track start
	int iAll;  // all road markers from road->vPace
	int iDir;  // copy from road
	int iCur;  // cur car pace id, for tracking
};


static bool PaceSort(const PaceNote& a, const PaceNote& b)
{
	return a.id < b.id;
}
