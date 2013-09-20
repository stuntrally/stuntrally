#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "../../road/Road.h"
#include "../../vdrift/pathmanager.h"
#ifndef SR_EDITOR
	#include "../../vdrift/game.h"
	#include "../CGame.h"
	#include "../CGui.h"
	#include "../SplitScreen.h"
#else
	#include "../../editor/CApp.h"
	#include "../../editor/CGui.h"
	#include "../../editor/settings.h"
#endif
#include "Slider.h"
#include "WaterRTT.h"
#include "../shiny/Main/Factory.hpp"
#include "../sdl4ogre/sdlinputwrapper.hpp"
#include <OgreTerrain.h>
using namespace MyGUI;
using namespace Ogre;
using namespace std;



///  Gui Init  [Graphics]
//----------------------------------------------------------------------------------------------------------------

void CGui::GuiInitGraphics()  // also called on preset change with bGI true
{
	SliderValue::pGUI = app->mGui;
	SliderValue::bGI = &bGI;

	ButtonPtr btn, bchk;  ComboBoxPtr cmb;
	SliderValue* sv;
	//Check* ck;
	
	//  detail
	sv= &svTerDetail;	sv->Init("TerDetail",	&pSet->terdetail,	0.f,2.f);  Sev(TerDetail);
	sv= &svTerDist;		sv->Init("TerDist",		&pSet->terdist, 0.f,2000.f, 2.f, 0,3, 1.f," m");  Sev(TerDist);
	sv= &svRoadDist;	sv->Init("RoadDist",	&pSet->road_dist,	0.f,4.f, 2.f, 2,5);

	//  textures
	Cmb(cmb, "TexFiltering", comboTexFilter);

	sv= &svViewDist;	sv->Init("ViewDist",	&pSet->view_distance, 50.f,20000.f, 2.f, 1,4, 0.001f," km");  Sev(ViewDist);
	sv= &svAnisotropy;	sv->Init("Anisotropy",	&pSet->anisotropy,	0.f,16.f);  Sev(Anisotropy);
	sv= &svTexSize;
		sv->strMap[0] = "Small";  sv->strMap[1] = "Big";
						sv->Init("TexSize",		&pSet->tex_size,	0.f,1.f);
	sv= &svTerMtr;
		sv->strMap[0] = TR("#{GraphicsAll_Lowest}");	sv->strMap[1] = TR("#{GraphicsAll_Medium}");
		sv->strMap[2] = TR("#{GraphicsAll_High}");		sv->strMap[3] = "Parallax";
						sv->Init("TerMtr",		&pSet->ter_mtr,		0.f,3.f);
	sv= &svTerTripl;
		sv->strMap[0] = "Off";  sv->strMap[1] = "One";  sv->strMap[2] = "Full";
						sv->Init("TerTripl",	&pSet->ter_tripl,	0.f,2.f);

	//  trees/grass
	sv= &svTrees;		sv->Init("Trees",		&pSet->gui.trees,	0.f,4.f, 2.f);
	sv= &svGrass;		sv->Init("Grass",		&pSet->grass,		0.f,4.f, 2.f);
	sv= &svTreesDist;	sv->Init("TreesDist",   &pSet->trees_dist,	0.5f,7.f, 2.f);
	sv= &svGrassDist;	sv->Init("GrassDist",   &pSet->grass_dist,	0.5f,7.f, 2.f);
	Btn("TrGrReset",  btnTrGrReset);

	//ck = ckUseImposters; ck->Init("UseImposters", pSet->use_imposters);  Cev(chkUseImposters);
	Chk("UseImposters",  chkUseImposters,  pSet->use_imposters);
	Chk("ImpostorsOnly", chkImpostorsOnly, pSet->imposters_only);

	//  shadows
	sv= &svShadowType;
		sv->strMap[0] = "None";		sv->strMap[1] = "Depth";	sv->strMap[2] = "Soft-";
						sv->Init("ShadowType",	&pSet->shadow_type, 0.f,1.f); //2.f);
	sv= &svShadowCount;	sv->Init("ShadowCount",	&pSet->shadow_count, 1.f,3.f);
	sv= &svShadowSize;
		for (int i=0; i < ciShadowSizesNum; ++i)  sv->strMap[i] = toStr(ciShadowSizesA[i]);
						sv->Init("ShadowSize",	&pSet->shadow_size, 0,ciShadowSizesNum-1);
	sv= &svShadowDist;	sv->Init("ShadowDist",	&pSet->shadow_dist, 20.f,5000.f, 3.f, 0,3, 1.f," m");
	Btn("Apply", btnShadows);
	
	Btn("ApplyShaders", btnShaders);
	Btn("ApplyShadersWater", btnShaders);
	
	//  water
	Chk("WaterReflection", chkWaterReflect, pSet->water_reflect);
	Chk("WaterRefraction", chkWaterRefract, pSet->water_refract);
	sv= &svWaterSize;
		for (int i=0; i <= 2; ++i)  sv->strMap[i] = toStr(ciShadowSizesA[i]);
						sv->Init("WaterSize",	&pSet->water_rttsize, 0.f,2.f);
	
	//  presets
	Cmb(cmb, "CmbGraphicsAll", comboGraphicsAll);
	if (cmb)
	{	cmb->removeAllItems();
		cmb->addItem(TR("#{GraphicsAll_Lowest}"));  cmb->addItem(TR("#{GraphicsAll_Low}"));
		cmb->addItem(TR("#{GraphicsAll_Medium}"));  cmb->addItem(TR("#{GraphicsAll_High}"));
		cmb->addItem(TR("#{GraphicsAll_Higher}"));  cmb->addItem(TR("#{GraphicsAll_VeryHigh}"));
		cmb->addItem(TR("#{GraphicsAll_Ultra}"));   cmb->addItem(TR("#{GraphicsAll_Impossible}"));
		cmb->setIndexSelected(pSet->preset);
	}
	
	//  screen
	Cmb(cmb, "CmbAntiAliasing", cmbAntiAliasing);
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
	Cmb(cmb, "CmbRendSys", comboRenderSystem);
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
		//const RenderSystemList& rsl = Ogre::Root::getSingleton().getAvailableRenderers();
		//for (int i=0; i < rsl.size(); ++i)
		//	combo->addItem(rsl[i]->getName());
	}
}


//  events . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

void CGui::comboTexFilter(CMB)
{
	TextureFilterOptions tfo;
	switch (val)  {
		case 0:	 tfo = TFO_BILINEAR;	break;
		case 1:	 tfo = TFO_TRILINEAR;	break;
		case 2:	 tfo = TFO_ANISOTROPIC;	break;	}
	MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
}

void CGui::slAnisotropy(SV*)
{
	MaterialManager::getSingleton().setDefaultAnisotropy(pSet->anisotropy);
}

void CGui::slViewDist(SV* sv)
{
	Vector3 sc = (*sv->pFloat) * Vector3::UNIT_SCALE;

	if (app->ndSky)  app->ndSky->setScale(sc);
	#ifndef SR_EDITOR
		app->mSplitMgr->UpdateCamDist();  // game, for all cams
	#else
		app->mCamera->setFarClipDistance(pSet->view_distance*1.1f);
	#endif
}

void CGui::slTerDetail(SV*)
{
	app->UpdTerErr();
}

void CGui::slTerDist(SV*)
{
	if (app->mTerrainGlobals)
		app->mTerrainGlobals->setCompositeMapDistance(pSet->terdist);
}

//  trees/grass
void CGui::btnTrGrReset(WP wp)
{
	svTrees.SetValueF(1.5f);
	svGrass.SetValueF(1.f);
	svTreesDist.SetValueF(1.f);
	svGrassDist.SetValueF(2.f);
}

void CGui::chkUseImposters(WP wp)
{
	ChkEv(use_imposters);
}
void CGui::chkImpostorsOnly(WP wp)
{
	ChkEv(imposters_only);
}


//  shadows
void CGui::btnShadows(WP){	app->changeShadows();	}
void CGui::btnShaders(WP){	app->changeShadows();	}


//  water
void CGui::chkWaterReflect(WP wp)
{
	ChkEv(water_reflect);
	app->mWaterRTT->setReflect(pSet->water_reflect);
	app->changeShadows();
	app->mWaterRTT->recreate();
}

void CGui::chkWaterRefract(WP wp)
{
	ChkEv(water_refract);
	app->mWaterRTT->setRefract(pSet->water_refract);
	app->changeShadows();
	app->mWaterRTT->recreate();
}
