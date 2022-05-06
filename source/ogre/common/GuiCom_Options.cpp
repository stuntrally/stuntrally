#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "CScene.h"
#include "../../road/Road.h"
#include "../../vdrift/pathmanager.h"
#ifndef SR_EDITOR
	#include "../../vdrift/game.h"
	#include "../CGame.h"
	#include "../SplitScreen.h"
#else
	#include "../../editor/CApp.h"
	#include "../../editor/settings.h"
#endif
#include "Slider.h"
#include "WaterRTT.h"
#include "../shiny/Main/Factory.hpp"
#include "../sdl4ogre/sdlinputwrapper.hpp"
#include <OgreTerrain.h>
#include <OgreCamera.h>
#include <OgreMaterialManager.h>
#include <OgreSceneNode.h>
#include <MyGUI.h>
using namespace MyGUI;
using namespace Ogre;
using namespace std;



///  Gui Init  [Graphics]
//----------------------------------------------------------------------------------------------------------------

void CGuiCom::GuiInitGraphics()  // also called on preset change with bGI true
{
	Btn btn;  Cmb cmb;
	SV* sv;  Ck* ck;

	BtnC("Quit", btnQuit);  bnQuit = btn;
	
	//  detail
	sv= &svTerDetail;	sv->Init("TerDetail",	&pSet->terdetail,	0.f,4.f, 1.5f);  SevC(TerDetail);  sv->DefaultF(1.f);
	sv= &svTerDist;		sv->Init("TerDist",		&pSet->terdist, 0.f,2000.f, 2.f, 0,3, 1.f," m");
																				SevC(TerDist);  sv->DefaultI(500.f); //ed 2000-
	sv= &svRoadDist;	sv->Init("RoadDist",	&pSet->road_dist,	0.f,4.f, 2.f, 2,5);  sv->DefaultF(1.6f);

	//  textures
	CmbC(cmb, "TexFiltering", comboTexFilter);
	cmb->removeAllItems();
	cmb->addItem(TR("#{Bilinear}"));
	cmb->addItem(TR("#{Trilinear}"));
	cmb->addItem(TR("#{Anisotropic}"));
	cmb->setIndexSelected(pSet->tex_filt);  comboTexFilter(cmb, pSet->tex_filt);

	sv= &svViewDist;	sv->Init("ViewDist",	&pSet->view_distance, 50.f,20000.f, 2.f, 1,4, 0.001f, TR(" #{UnitKm}"));
																				SevC(ViewDist);  sv->DefaultF(8000.f);
	sv= &svAnisotropy;	sv->Init("Anisotropy",	&pSet->anisotropy,	0,16);		SevC(Anisotropy);  sv->DefaultI(4);
	sv= &svTexSize;
		sv->strMap[0] = TR("#{Small}");  sv->strMap[1] = TR("#{Big}");
						sv->Init("TexSize",		&pSet->tex_size,	0,1);	sv->DefaultI(1);
	//  terrain
	sv= &svTerMtr;
		sv->strMap[0] = TR("#{GraphicsAll_Lowest}");	sv->strMap[1] = TR("#{GraphicsAll_Medium}");
		sv->strMap[2] = TR("#{GraphicsAll_High}");		sv->strMap[3] = "Parallax-";
						sv->Init("TerMtr",		&pSet->ter_mtr,		0,3);	sv->DefaultI(2);
	sv= &svTerTripl;
		sv->strMap[0] = TR("#{None}");  sv->strMap[1] = TR("#{max} 2");  sv->strMap[2] = TR("#{Any}");
						sv->Init("TerTripl",	&pSet->ter_tripl,	0,2);	sv->DefaultF(1);

	//  trees/grass
	sv= &svTrees;		sv->Init("Trees",		&pSet->gui.trees,	0.f,4.f, 2.f);   sv->DefaultF(1.5f);
	sv= &svGrass;		sv->Init("Grass",		&pSet->grass,		0.f,4.f, 2.f);   sv->DefaultF(1.f);
	sv= &svTreesDist;	sv->Init("TreesDist",   &pSet->trees_dist,	0.5f,7.f, 2.f);  sv->DefaultF(1.f);
	sv= &svGrassDist;	sv->Init("GrassDist",   &pSet->grass_dist,	0.5f,7.f, 2.f);  sv->DefaultF(2.f);
	BtnC("TrGrReset",  btnTrGrReset);

	ck= &ckUseImposters;  ck->Init("UseImposters", &pSet->use_impostors);
	ck= &ckImpostorsOnly; ck->Init("ImpostorsOnly",&pSet->impostors_only);

	//  shadows
	sv= &svShadowType;
		sv->strMap[0] = TR("#{None}");	sv->strMap[1] = "Depth";	sv->strMap[2] = "Soft-";
						sv->Init("ShadowType",	&pSet->shadow_type,  0,1);  sv->DefaultI(1);
	sv= &svShadowCount;	sv->Init("ShadowCount",	&pSet->shadow_count, 1,3);  sv->DefaultI(2);
	sv= &svShadowSize;
		for (int i=0; i < ciShadowSizesNum; ++i)  sv->strMap[i] = toStr(ciShadowSizesA[i]);
						sv->Init("ShadowSize",	&pSet->shadow_size, 0,ciShadowSizesNum-1);   sv->DefaultI(3);
	sv= &svShadowDist;	sv->Init("ShadowDist",	&pSet->shadow_dist, 20.f,5000.f, 3.f, 0,3, 1.f," m");  sv->DefaultF(1300.f);

	BtnC("Apply", btnShadows);
	BtnC("ApplyShaders", btnShaders);
	BtnC("ApplyShadersWater", btnShaders);
	
	//  water
	ck= &ckWaterReflect; ck->Init("WaterReflection", &pSet->water_reflect);  CevC(Water);
	ck= &ckWaterRefract; ck->Init("WaterRefraction", &pSet->water_refract);  CevC(Water);
	sv= &svWaterSize;
		for (int i=0; i <= 2; ++i)  sv->strMap[i] = toStr(ciShadowSizesA[i]);
						sv->Init("WaterSize",	&pSet->water_rttsize, 0.f,2.f);  sv->DefaultI(0);  SevC(WaterSize);
	
	//  presets
	CmbC(cmb, "CmbGraphicsAll", comboGraphicsAll);
	if (cmb)
	{	cmb->removeAllItems();
		cmb->addItem(TR("#{GraphicsAll_Lowest}"));  cmb->addItem(TR("#{GraphicsAll_Low}"));
		cmb->addItem(TR("#{GraphicsAll_Medium}"));  cmb->addItem(TR("#{GraphicsAll_High}"));
		cmb->addItem(TR("#{GraphicsAll_Higher}"));  cmb->addItem(TR("#{GraphicsAll_VeryHigh}"));
		cmb->addItem(TR("#{GraphicsAll_Ultra}"));   cmb->addItem(TR("#{GraphicsAll_Impossible}"));
		cmb->setIndexSelected(pSet->preset);
	}
	
	//  video
	ck= &ckLimitFps;   ck->Init("LimitFpsOn", &pSet->limit_fps);
	sv= &svLimitFps;   sv->Init("LimitFps",   &pSet->limit_fps_val,	20.f,144.f);  sv->DefaultF(60.f);
	sv= &svLimitSleep; sv->Init("LimitSleep", &pSet->limit_sleep,  -2,20, 1.5f);  sv->DefaultI(-1);
	
	//  screen
	CmbC(cmb, "CmbAntiAliasing", cmbAntiAliasing);
	int si=0;
	if (cmb)
	{	cmb->removeAllItems();

		int a[6] = {0,1,2,4,8,16};
		for (int i=0; i < 6; ++i)
		{	int v = a[i];

			cmb->addItem(toStr(v));
			if (pSet->fsaa >= v)
				si = i;
		}
		cmb->setIndexSelected(si);
	}

	//  render systems
	#if 0
	CmbC(cmb, "CmbRendSys", comboRenderSystem);
	if (cmb)
	{	cmb->removeAllItems();

		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		const int nRS = 3;  //4
		const String rs[nRS] = {"Default", "OpenGL Rendering Subsystem",
			"Direct3D9 Rendering Subsystem"/*, "Direct3D11 Rendering Subsystem"*/};
		#else
		const int nRS = 2;
		const String rs[nRS] = {"Default", "OpenGL Rendering Subsystem"};
		#endif
			
		for (int i=0; i < nRS; ++i)
		{
			cmb->addItem(rs[i]);
			if (pSet->rendersystem == rs[i])
				cmb->setIndexSelected(cmb->findItemIndexWith(rs[i]));
		}
		//const RenderSystemList& rsl = Root::getSingleton().getAvailableRenderers();
		//for (int i=0; i < rsl.size(); ++i)
		//	combo->addItem(rsl[i]->getName());
	}
	#endif
}


//  events . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

void CGuiCom::comboTexFilter(CMB)
{
	pSet->tex_filt = val;
	TextureFilterOptions tfo;
	switch (val)  {
		case 0:	 tfo = TFO_BILINEAR;	break;
		case 1:	 tfo = TFO_TRILINEAR;	break;
		case 2:	 tfo = TFO_ANISOTROPIC;	break;	}
	MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
}

void CGuiCom::slAnisotropy(SV*)
{
	MaterialManager::getSingleton().setDefaultAnisotropy(pSet->anisotropy);
}

void CGuiCom::slViewDist(SV* sv)
{
	Vector3 sc = pSet->view_distance * Vector3::UNIT_SCALE;
	if (app->ndSky)
		app->ndSky->setScale(sc);

	#ifndef SR_EDITOR
		app->mSplitMgr->UpdateCamDist();  // game, for all cams
	#else
		app->mCamera->setFarClipDistance(pSet->view_distance*1.1f);
	#endif
}

void CGuiCom::slTerDetail(SV*)
{
	app->scn->UpdTerErr();
}

void CGuiCom::slTerDist(SV*)
{
	if (app->scn->mTerrainGlobals)
		app->scn->mTerrainGlobals->setCompositeMapDistance(pSet->terdist);
}

//  trees/grass
void CGuiCom::btnTrGrReset(WP wp)
{
	svTrees.SetValueF(1.5f);
	svGrass.SetValueF(1.f);
	svTreesDist.SetValueF(1.f);
	svGrassDist.SetValueF(2.f);
}


//  shadows
void CGuiCom::btnShadows(WP){	app->scn->changeShadows();	}
void CGuiCom::btnShaders(WP){	app->scn->changeShadows();	}


//  water
void CGuiCom::chkWater(Ck*)
{
	app->scn->mWaterRTT->setReflect(pSet->water_reflect);
	app->scn->mWaterRTT->setRefract(pSet->water_refract);
	app->scn->changeShadows();
	app->scn->mWaterRTT->recreate();
}

void CGuiCom::slWaterSize(SV*)
{
	app->scn->mWaterRTT->setRTTSize(ciShadowSizesA[pSet->water_rttsize]);
	app->scn->mWaterRTT->recreate();
}
