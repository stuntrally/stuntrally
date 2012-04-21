#include "pch.h"
#include "../ogre/common/Defines.h"
#include "OgreApp.h"
#include "../road/Road.h"
#include "../paged-geom/PagedGeometry.h"
#include "../vdrift/pathmanager.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/MaterialGen/MaterialFactory.h"
using namespace Ogre;


//  Create Scene
//-------------------------------------------------------------------------------------
void App::createScene()
{
	//  camera
	asp = float(mWindow->getWidth())/float(mWindow->getHeight());
	mCamera->setFarClipDistance(pSet->view_distance*1.1f);
	mCamera->setNearClipDistance(0.1f);

	//  cam pos from last set
	mCameraT->setPosition(Vector3(pSet->cam_x,pSet->cam_y,pSet->cam_z));
	mCameraT->setDirection(Vector3(pSet->cam_dx,pSet->cam_dy,pSet->cam_dz).normalisedCopy());
	mViewport->setVisibilityMask(RV_MaskAll);  // hide prv cam rect

	//  tex fil
	MaterialManager::getSingleton().setDefaultTextureFiltering(TFO_ANISOTROPIC);
	MaterialManager::getSingleton().setDefaultAnisotropy(pSet->anisotropy);

	mRoot->addResourceLocation(pathTrkPrv[1], "FileSystem");  //prv user tracks

	QTimer ti;  ti.update();  /// time

	//  tracks.xml
	tracksXml.LoadXml(PATHMANAGER::GetGameConfigDir() + "/tracks.xml");
	//tracksXml.SaveXml(PATHMANAGER::GetGameConfigDir() + "/tracks2.xml");

	//  fluids.xml
	fluidsXml.LoadXml(PATHMANAGER::GetDataPath() + "/materials/fluids.xml");
	sc.pFluidsXml = &fluidsXml;
	LogO(String("**** Loaded fluids.xml: ") + toStr(fluidsXml.fls.size()));

	//  collisions.xml
	objs.LoadXml();
	LogO(String("**** Loaded Vegetation objects: ") + toStr(objs.colsMap.size()));

	ti.update();  /// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time load xmls: ") + toStr(dt) + " ms");

	//  gui
	bGuiFocus = false/*true*/;  bMoveCam = true;  //*--
	InitGui();
	TerCircleInit();
	createBrushPrv();

	//  load
	if (pSet->autostart)
		LoadTrack();
	else
	{	bGuiFocus = true;  UpdVisGui();	}
}


//---------------------------------------------------------------------------------------------------------------
///  Load Track
//---------------------------------------------------------------------------------------------------------------
void App::UpdTrees()
{
	if (!pSet->bTrees)
	{
		if (grass) {  delete grass->getPageLoader();  delete grass;  grass=0;   }
		if (trees) {  delete trees->getPageLoader();  delete trees;  trees=0;   }
	}else
		CreateTrees();
}

void App::NewCommon(bool onlyTerVeget)
{
	//  destroy all
	if (ndSky)
		mSceneMgr->destroySceneNode(ndSky);

	if (grass) {  delete grass->getPageLoader();  delete grass;  grass=0;   }
	if (trees) {  delete trees->getPageLoader();  delete trees;  trees=0;   }

	//mSceneMgr->destroyAllStaticGeometry();
	if (!onlyTerVeget)
	{
		DestroyObjects();
		DestroyFluids();
	}

	//  terrain
	terrain = 0;
	materialFactory->setTerrain(0);
	if (mTerrainGroup)
		mTerrainGroup->removeAllTerrains();

	if (resTrk != "")  mRoot->removeResourceLocation(resTrk);
		resTrk = TrkDir() + "objects";
	mRoot->addResourceLocation(resTrk, "FileSystem");
}
//---------------------------------------------------------------------------------------------------------------
void App::LoadTrack()
{
	eTrkEvent = TE_Load;
	Status("Loading...", 0.3,0.6,1.0);
}
void App::LoadTrackEv()
{
	QTimer ti;  ti.update();  /// time
	NewCommon(false);  // full destroy

	if (road)
	{	road->Destroy();  delete road;  road = 0;  }

	// load scene
	sc.ter = true;
	sc.LoadXml(TrkDir()+"scene.xml");
	
	//  water RTT
	UpdateWaterRTT(mCamera);
	
	/// generate materials
	materialFactory->generate();
	CreateRoadSelMtrs();

	LoadSurf();
	UpdWndTitle();

	CreateFluids();

	bNewHmap = false;/**/
	CreateTerrain();

	//  road ~
	road = new SplineRoad();
	road->iTexSize = pSet->tex_size;
	road->Setup("sphere.mesh", 1.4f*pSet->road_sphr, terrain, mSceneMgr, mCamera);
	road->LoadFile(TrkDir()+"road.xml");
	UpdPSSMMaterials();
	
	CreateObjects();

	if (pSet->bTrees)
		CreateTrees();  // trees after objects so they aren't inside them


	//  updates after load
	//--------------------------
	ReadTrkStats();
	SetGuiFromXmls();  ///
	
	Rnd2TexSetup();
	UpdVisGui();
	LoadStartPos();

	try {
	TexturePtr tex = TextureManager::getSingleton().getByName("waterDepth.png");
	if (!tex.isNull())
		tex->reload();
	} catch(...) {  }


	Status("Loaded", 0.5,0.7,1.0);

	ti.update();	/// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Load Track: ") + toStr(dt) + " ms");
}


///  Update Track
//---------------------------------------------------------------------------------------------------------------
void App::UpdateTrack()
{
	eTrkEvent = TE_Update;
	Status("Updating...",0.2,1.0,0.5);
}
void App::UpdateTrackEv()
{
	NewCommon(true);  // destroy only terrain and veget
	
	//CreateFluids();
	CreateTerrain(bNewHmap,true);/**/

	//  road ~
	road->mTerrain = terrain;
	road->RebuildRoad(true);
	UpdPSSMMaterials();

	//CreateObjects();

	if (pSet->bTrees)
		CreateTrees();

	Rnd2TexSetup();

	Status("Updated",0.5,1.0,0.7);
}


void App::UpdWndTitle()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	HWND hwnd = 0;  // update wnd title
	mWindow->getCustomAttribute("WINDOW", (void*)&hwnd); 
	String s = String("SR Editor  track: ") + pSet->gui.track;
	if (pSet->gui.track_user)  s += "  *user*";
	SetWindowText(hwnd, s.c_str());
#endif
	// TODO: Window title for linux
	// overlay ?  not visible in fullscreen...
}

String App::TrkDir() {
	int u = pSet->gui.track_user ? 1 : 0;		return pathTrk[u] + pSet->gui.track + "/";  }

String App::PathListTrk(int user) {
	int u = user == -1 ? bListTrackU : user;	return pathTrk[u] + sListTrack;  }
	
String App::PathListTrkPrv(int user){
	int u = user == -1 ? bListTrackU : user;	return pathTrkPrv[u] + sListTrack;  }
	
String App::PathCopyTrk(int user){
	int u = user == -1 ? bCopyTrackU : user;	return pathTrk[u] + sCopyTrack;  }


///  Save Terrain
//---------------------------------------------------------------------------------------------------------------
void App::SaveTrack()
{
	if (!pSet->allow_save)  // could force it when in writable location
	if (!pSet->gui.track_user)
	{	MyGUI::Message::createMessageBox(
			"Message", "Save Track", "Can't save original track. Duplicate it first.",
			MyGUI::MessageBoxStyle::IconWarning | MyGUI::MessageBoxStyle::Ok);
		return;
	}
	eTrkEvent = TE_Save;
	Status("Saving...", 1,0.4,0.1);
}
void App::SaveTrackEv()
{	
	//  track dir in user
	CreateDir(TrkDir());
	CreateDir(TrkDir() + "/objects");
	//  check if succeded ...

	if (terrain)
	{	float *fHmap = terrain->getHeightData();
		int size = sc.td.iVertsX * sc.td.iVertsY * sizeof(float);

		String file = TrkDir()+"heightmap.f32";
		std::ofstream of;
		of.open(file.c_str(), std::ios_base::binary);
		of.write((const char*)fHmap, size);
		of.close();
	}
	if (road)
		road->SaveFile(TrkDir()+"road.xml");

	sc.SaveXml(TrkDir()+"scene.xml");
	SaveSurf(TrkDir()+"surfaces.txt");

	SaveGrassDens();
	SaveWaterDepth();
	SaveStartPos(TrkDir()+"track.txt");  //..load/save inside
	Status("Saved", 1,0.6,0.2);
}


///  Ter Circle mesh   o
//-------------------------------------------------------------------------------------
const int divs = 90;
const Real aAdd = 2 * 2*PI_d / divs, dTc = 2.f/(divs+1) *4;
static Real fTcos[divs+4], fTsin[divs+4];


void App::TerCircleInit()
{
	moTerC = mSceneMgr->createManualObject();
	moTerC->setDynamic(true);
	moTerC->setCastShadows(false);

	moTerC->estimateVertexCount(divs+2);
	moTerC->estimateIndexCount(divs+2);
	moTerC->begin("circle_deform", RenderOperation::OT_TRIANGLE_STRIP);

	for (int d = 0; d < divs+2; ++d)
	{
		Real a = d/2 * aAdd;	fTcos[d] = cosf(a);  fTsin[d] = sinf(a);
		Real r = (d % 2 == 0) ? 1.f : 0.9f;
		Real x = r * fTcos[d], z = r * fTsin[d];
		moTerC->position(x,0,z);  //moTerC->normal(0,1,0);
		moTerC->textureCoord(d/2*dTc, d%2);
	}
	moTerC->end();
 
	AxisAlignedBox aabInf;	aabInf.setInfinite();
	moTerC->setBoundingBox(aabInf);  // always visible
	moTerC->setRenderQueueGroup(RQG_Hud2);
	moTerC->setVisibilityFlags(RV_Hud);
	ndTerC = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,0,0));
	ndTerC->attachObject(moTerC);  ndTerC->setVisible(false);
}


void App::TerCircleUpd()
{
	if (!moTerC || !terrain || !road)  return;

	bool edTer = bEdit() && (edMode < ED_Road) && road->bHitTer;
	ndTerC->setVisible(edTer);
	if (!edTer)  return;
	
	Real rbr = mBrSize[curBr] * 0.5f * sc.td.fTriangleSize * 0.8f/*?par*/;

	static ED_MODE edOld = ED_ALL;
	if (edOld != edMode)
	{	edOld = edMode;
		switch(edMode)
		{
		case ED_Deform: moTerC->setMaterialName(0, "circle_deform");  break;
		case ED_Filter: moTerC->setMaterialName(0, "circle_filter");  break;
		case ED_Smooth: moTerC->setMaterialName(0, "circle_smooth");  break;
		case ED_Height: moTerC->setMaterialName(0, "circle_height");  break;
		}
	}
	moTerC->beginUpdate(0);
	for (int d = 0; d < divs+2; ++d)
	{
		Real a = d/2 * aAdd;
		Real r = ((d % 2 == 0) ? 1.0f : 0.95f) * rbr;
		Real x = r * fTcos[d], z = r * fTsin[d];
		Vector3 p(x,0,z);  p += road->posHit;
		p.y = terrain->getHeightAtWorldPosition(p) + 0.3f;
		moTerC->position(p);  //moTerC->normal(0,1,0);
		moTerC->textureCoord(d/2*dTc, d%2);
	}
	moTerC->end();
}
