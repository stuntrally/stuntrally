#pragma once

#include <vector>
#include <OgreString.h>
#include <OgreVector3.h>

#include <OgreMesh.h>
#include <OgreAxisAlignedBox.h>

namespace Ogre {  class SceneManager;  class SceneNode;  class Entity;  class Terrain;  class Camera;  }
class SplineRoad;


enum PaceTypes                 // 90     // 180    // 270
{	P_1=0, P_2, P_3, P_4, P_5, P_6sq, P_7hrp, P_8u, P_9o,  // turns
	P_Jump, P_JumpTer,  // jumps
	P_Loop, P_Loop2, P_SideLoop, P_LoopBarrel,  // loops
	P_OnPipe, P_Slow, P_Stop, P_Danger,   // brake, warn
	P_Bumps, P_Narrow, P_Obstacle, P_Split,
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

	//PaceTypes type;
	//int dir;  // -1 left, 1 right
	//float vel;  // for jump
	PaceNote();
	PaceNote(Ogre::Vector3 p, float sx,float sy,
		float r,float g,float b,float a, float ox,float oy, float u,float v);
};


class PaceNotes
{
public:
	class App* pApp;  ///*
	PaceNotes(App* papp);

	//virtual ~PaceNotes();
	//void Defaults();

	//  Setup, call this on Init
	void Setup(Ogre::SceneManager* sceneMgr, Ogre::Camera* camera, Ogre::Terrain* terrain);

	//  Rebuild
	void Rebuild(SplineRoad* road), Create(PaceNote& n);
	void Destroy(), Destroy(PaceNote& n);


	//  Update
	//void UpdVis(/*Camera* pCam,*/ float fBias=1.f, bool bFull=false);

	//void Pick(Ogre::Camera* mCamera, Ogre::Real mx, Ogre::Real my,
	//		bool bRay=true, bool bAddH=false, bool bHide=false);
	//void SelectMarker(bool bHide=false);


	//  Insert  -------
	//void Insert(eIns ins);
	//void Delete(), DelSel();

	//bool CopySel();
	//void Paste(bool reverse=false);
	

private:
	//  ogre vars
	Ogre::SceneManager* mSceneMgr;
	Ogre::Camera* mCamera;
	Ogre::Terrain* mTerrain;
	
	//  all notes
	std::vector<PaceNote> vPN;
	int ii;  // id for names
};
