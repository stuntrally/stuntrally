#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/Axes.h"
#include "PaceNotes.h"
#include "../ogre/ReplayTrk.h"
#include "../vdrift/dbl.h"
#ifdef SR_EDITOR
	#include "../editor/CApp.h"
	#include "../editor/settings.h"
#else
	#include "../ogre/CGame.h"
	#include "../vdrift/settings.h"
#endif
#include "../vdrift/pathmanager.h"
#include "../road/SplineBase.h"
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
	
	///  trace road
return;

	///  trace Track's Ghost
	TrackGhost gho;
	
	//  foreach track
	bool rev = false;
	string sRev = rev ? "_r" : "";
	#ifdef SR_EDITOR
	string track = pApp->pSet->gui.track;
	#else
	string track = pApp->pSet->game.track;
	#endif
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

		int num = gho.getNumFrames(), ijmp = 0;
		float* sta = new float[num];
		int i,ii,n;  float oy=0.f;
		for (ii=0; ii < 2; ++ii)
		for (i=0; i < num; ++i)
		{
			//  pos
			const TrackFrame& fr = gho.getFrame0(i);
			Axes::toOgre(pos, fr.pos);  // pos
			rot = Axes::toOgre(fr.rot);
			//float y = rot.getYaw().valueDegrees();
			Vector3 pp = pos - oldPos;
			
			//  yaw
			float yy = TerUtil::GetAngle(pp.x, pp.z) *180.f/PI_d;
			float y = yy-oy;  if (y > 180)  y -= 360;  if (y < -180)  y += 360;
			oy = yy;

			//  vel
			float dist = pp.length();
			float dt = fr.time - oldTime;  // 0.04
			if (i > 0 && i < num-1 && dt > 0.001f)
				vel = 3.6f * dist / dt;

			if (vel < 20)  y *= vel / 20.f;  // y sc

			//  sudden pos jumps-
			bool jmp = false;
			if (i > 10 && i < num-1)
			if (dist > 6.f)  //par
			{	jmp = true;  ++ijmp;  }


			//  avg steer
			float sa = 0.f;  const int nn = 10;  //par
			if (i < num-nn)
			for (n=i; n < i+nn; ++n)
			{
				const TrackFrame& f = gho.getFrame0(n);
				sa += fabs(f.steer/127.f);
			}
			sa /= float(nn);
			sta[i] = sa;
			
			
			//  log  ----
			if (ii==0)
			LogO("i:"+ iToStr(i,4) +" t:"+ fToStr(fr.time,2,6)//+" dt: "+fToStr(dt)
				+"  v:"+ fToStr(vel,0,4)
				+"  b:"+ toStr(fr.brake)
				+"  s:"+ (fr.steer==0 ? " ---" : fToStr(fr.steer/127.f,1,4))
				+"  a:"+ fToStr(sa,1,4)
				+"  y:"+ fToStr(y,1,4)
				//+"  p: "+ fToStr(fr.pos[0])+" "+fToStr(fr.pos[1])+" "+fToStr(fr.pos[2])
				+(jmp ? " !jd: "+fToStr(dist) : "")
			);
	
			
			///  add pace note
			if (ii==1)
			{	
				float sb = 0.f;  const int nn = 10;  //par
				if (i < num-nn)
				for (n=i; n < i+nn; ++n)
				{
					const TrackFrame& f = gho.getFrame0(n);
					if (sa > 0.3f)
					sb += sa;
				}
				sb /= float(nn);
				//LogO(fToStr(sb));
							
				//  create
				if (i%6==0 && sb > 0.4f)
				{
					PaceNote n;
					n.pos = pos;  //fr.brake  fr.steer
					Create(n);
					vv.push_back(n);
				}
			}
			
			oldPos = pos;  oldTime = fr.time;
		}
		if (ijmp > 0)
			LogO("!Jumps: "+toStr(ijmp));
		delete[] sta;
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
	n.bb->setCustomParameter(0, Vector4(1.f,1.f, 0.f,0.f));  // uv ofs

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
	for (size_t i=0; i < vv.size(); ++i)
		Destroy(vv[i]);
	vv.clear();
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
