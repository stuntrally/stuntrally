#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/Axes.h"
#include "PaceNotes.h"
#include "../ogre/ReplayTrk.h"
#include "../editor/CApp.h"
#include "../editor/settings.h"
#include "../vdrift/pathmanager.h"
//#include "CGui.h"
#include <OgreTimer.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreBillboardSet.h>
//#include <OgreTerrain.h>
using namespace std;
using namespace Ogre;


//  ctor  ---------
PaceNote::PaceNote()
	:nd(0), bb(0), pos(0,0,0)
	,type(P_1), dir(0), vel(0.f)
{	}

//TODO: same mtr/tex, uv ofs for signs, fade alpha when close


//  Pace notes
//--------------------------------------------------------------------------------------
void PaceNotes::Rebuild()
{
	Ogre::Timer ti;	

	Destroy();

	#if 0	//  test
	for (int i=0; i < 10; ++i)
	{
		PaceNote n;
		n.pos = Vector3(i%4*20, 10+i/3, i/4*20);
		Create(n);
		v.push_back(n);
	}
	#endif
	
	///  trace road
	//TODO
return;

	///  trace Track's Ghost
	TrackGhost gho;
	
	//  foreach track
	bool rev = false;
	string sRev = rev ? "_r" : "";
	string track = pApp->pSet->gui.track;
	//if (track.substr(0,4) == "Test" && track.substr(0,5) != "TestC")  continue;
	
	//  load
	string file = PATHMANAGER::TrkGhosts()+"/"+ track + sRev + ".gho";
	if (!PATHMANAGER::FileExists(file))
	{	LogO("NOT found: "+file);/**/  }
	else
	{	LogO("---------  "+track+"  ---------");
		gho.LoadFile(file);
		
		//  test
		Vector3 pos,oldPos;  float oldTime = 0.f;
		Quaternion rot;  float vel = 0.f;

		int num = gho.getNumFrames(), jmp = 0;
		for (int i=0; i < num; ++i)
		{
			const TrackFrame& fr = gho.getFrame0(i);
			Axes::toOgre(pos, fr.pos);  // pos
			rot = Axes::toOgre(fr.rot);

			float dist = (pos - oldPos).length();
			float dt = fr.time - oldTime;  // 0.04
			if (i > 0 && dt > 0.001f)
				vel = 3.6f * dist / dt;

			//  check for sudden pos jumps  (rewind used but not with _Tool_ go back time !)
			bool jmp = false;
			if (i > 10 && i < num-1)  // ignore jumps at start or end
			if (dist > 6.f)  //par
				jmp = true;

			LogO("i:"+ iToStr(i,4) +" t:"+ fToStr(fr.time,2,6)//+" dt: "+fToStr(dt)
				+"  v:"+ fToStr(vel,1,4)
				+"  b:"+ toStr(fr.brake)
				+"  s: "+ (fr.steer==0 ? "----" : fToStr(fr.steer/127.f))
				//+"  p: "+ fToStr(fr.pos[0])+" "+fToStr(fr.pos[1])+" "+fToStr(fr.pos[2])
				+(jmp ? " !jd: "+fToStr(dist) : "")
			);
			
			///  add pace note
			if (i%20==0)
			{
				PaceNote n;
				n.pos = pos;  //fr.brake  fr.steer
				Create(n);
				v.push_back(n);
			}
			
			oldPos = pos;  oldTime = fr.time;
		}
		if (jmp > 0)
			LogO("!Jumps: "+toStr(jmp));
	}
	


	//UpdVis(fLodBias);

	LogO(String("::: Time PaceNotes Rebuild: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}
//--------------------------------------------------------------------------------------


//  Create
void PaceNotes::Create(PaceNote& n)
{
	float fSize = 4.f;  ++ii;
	n.nd = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	n.bb = mSceneMgr->createBillboardSet("P-"+toStr(ii),2);
	n.bb->setDefaultDimensions(fSize, fSize);

	n.bb->setRenderQueueGroup(RQG_CarTrails);
	n.bb->setVisibilityFlags(RV_Car);
	n.bb->setCustomParameter(1, Vector4(1.f,0.5f, 0.f,0.f));  // uv ofs

	n.bb->createBillboard(Vector3(0,0,0), ColourValue::White);
	//n.bb->setVisible(false);
	n.bb->setMaterialName("flare2");
	n.nd->attachObject(n.bb);
	n.nd->setPosition(n.pos);
}

//  Destroy
void PaceNotes::Destroy(PaceNote& n)
{
	mSceneMgr->destroyBillboardSet(n.bb);  n.bb = 0;
	mSceneMgr->destroySceneNode(n.nd);  n.nd = 0;
}

void PaceNotes::Destroy()  // all
{
	for (size_t i=0; i < v.size(); ++i)
		Destroy(v[i]);
	v.clear();
	ii = 0;
}


//  ctor  ---------
PaceNotes::PaceNotes(App* papp) :pApp(papp)
	,mSceneMgr(0),mCamera(0),mTerrain(0), ii(0)
{	}

//  setup
void PaceNotes::Setup(SceneManager* sceneMgr, Camera* camera, Terrain* terrain)
{
	mSceneMgr = sceneMgr;  mCamera = camera;  mTerrain = terrain;
}
