#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/RenderConst.h"
#include "PaceNotes.h"
#include <OgreTimer.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreBillboardSet.h>
//#include <OgreTerrain.h>
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
	
	

	//UpdVis(fLodBias);

	LogO(String("::: Time PaceNotes Rebuild: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}
//--------------------------------------------------------------------------------------


//  Create
void PaceNotes::Create(PaceNote& n)
{
	float fSize = 12.f;  ++ii;
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
#ifdef SR_EDITOR
PaceNotes::PaceNotes(App* papp) :pApp(papp)
#else
PaceNotes::PaceNotes(GAME* pgame) :pGame(pgame)
#endif
	,mSceneMgr(0),mCamera(0),mTerrain(0), ii(0)
{	}

//  setup
void PaceNotes::Setup(SceneManager* sceneMgr, Camera* camera, Terrain* terrain)
{
	mSceneMgr = sceneMgr;  mCamera = camera;  mTerrain = terrain;
}
