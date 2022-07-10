#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "CScene.h"
#include "WaterRTT.h"
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
#include <OgreRoot.h>
#include <OgreMaterialManager.h>
#include <OgreSceneManager.h>
#include <OgreTerrain.h>
#include <OgreRenderWindow.h>
#include <MyGUI.h>
#include "Slider.h"
using namespace MyGUI;
using namespace Ogre;


///  change all Graphics settings
///..............................................................................................................................
void CGuiCom::comboGraphicsAll(ComboBoxPtr cmb, size_t val)
{
	pSet->preset = val;  // for info
	//game_fq = 100.f;  blt_fq = 60.f;  blt_iter = 7;  dyn_iter = 10;  mult_thr = 0;  //sim low
	//veget_collis = true;  car_collis = false;

	SETTINGS& s = *pSet;
	switch (val)        ///  common
	{
	case 0:  // Lowest  -------------
		s.anisotropy = 0;  s.view_distance = 1000;  s.terdetail = 2.0f;  s.terdist = 0.f;  s.road_dist = 1.0;
		s.tex_size = 0;  s.ter_mtr = 0;  s.ter_tripl = 0;  s.use_impostors = 0;  s.impostors_only = 1;
		s.water_reflect = 0;  s.water_refract = 0;  s.water_rttsize = 0;
		s.shadow_type = Sh_None;  s.shadow_size = 0;  s.shadow_count = 1;  s.shadow_dist = 100;
		s.gui.trees = 0.f;  s.grass = 0.f;  s.trees_dist = 1.f;  s.grass_dist = 1.f;	break;

	case 1:  // Low  -------------
		s.anisotropy = 0;  s.view_distance = 1500;  s.terdetail = 1.7f;  s.terdist = 40.f;  s.road_dist = 1.2;
		s.tex_size = 0;  s.ter_mtr = 1;  s.ter_tripl = 0;  s.use_impostors = 0;  s.impostors_only = 1;
		s.water_reflect = 0;  s.water_refract = 0;  s.water_rttsize = 0;
		s.shadow_type = Sh_None;  s.shadow_size = 0;  s.shadow_count = 1;  s.shadow_dist = 100;
		s.gui.trees = 0.f;  s.grass = 0.f;  s.trees_dist = 1.f;  s.grass_dist = 1.f;	break;

	case 2:  // Medium  -------------
		s.anisotropy = 2;  s.view_distance = 2500;  s.terdetail = 1.5f;  s.terdist = 80.f;  s.road_dist = 1.4;
		s.tex_size = 1;  s.ter_mtr = 1;  s.ter_tripl = 0;  s.use_impostors = 1;  s.impostors_only = 1;
		s.water_reflect = 0;  s.water_refract = 0;  s.water_rttsize = 0;
		s.shadow_type = Sh_Depth;  s.shadow_size = 1;  s.shadow_count = 1;  s.shadow_dist = 200;
		s.gui.trees = 0.5f;  s.grass = 1.f;  s.trees_dist = 1.f;  s.grass_dist = 1.f;	break;

	case 3:  // High  -------------
		s.anisotropy = 4;  s.view_distance = 6000;  s.terdetail = 1.2f;  s.terdist = 200.f;  s.road_dist = 1.6;
		s.tex_size = 1;  s.ter_mtr = 2;  s.ter_tripl = 0;  s.use_impostors = 1;  s.impostors_only = 0;
		s.water_reflect = 1;  s.water_refract = 0;  s.water_rttsize = 0;
		s.shadow_type = Sh_Depth;  s.shadow_size = 2;  s.shadow_count = 1;  s.shadow_dist = 700;
		s.gui.trees = 1.f;  s.grass = 1.f;  s.trees_dist = 1.f;  s.grass_dist = 1.5f;	break;

	case 4:  // Higher (default)  -------------
		s.anisotropy = 4;  s.view_distance = 8000;  s.terdetail = 1.0f;  s.terdist = 500.f;  s.road_dist = 1.6;
		s.tex_size = 1;  s.ter_mtr = 2;  s.ter_tripl = 1;  s.use_impostors = 1;  s.impostors_only = 0;
		s.water_reflect = 1;  s.water_refract = 1;  s.water_rttsize = 0;
		s.shadow_type = Sh_Depth;  s.shadow_size = 3;  s.shadow_count = 2;  s.shadow_dist = 1300;
		s.gui.trees = 1.5f;  s.grass = 1.f;  s.trees_dist = 1.f;  s.grass_dist = 2.f;	break;

	case 5:  // Very High  -------------
		s.anisotropy = 4;  s.view_distance = 12000;  s.terdetail = 1.0f;  s.terdist = 700.f;  s.road_dist = 2.0;
		s.tex_size = 1;  s.ter_mtr = 2;  s.ter_tripl = 1;  s.use_impostors = 1;  s.impostors_only = 0;
		s.water_reflect = 1;  s.water_refract = 1;  s.water_rttsize = 1;
		s.shadow_type = Sh_Depth;  s.shadow_size = 3;  s.shadow_count = 2;  s.shadow_dist = 1600;
		s.gui.trees = 1.5f;  s.grass = 1.f;  s.trees_dist = 1.f;  s.grass_dist = 2.f;	break;

	case 6:  // Ultra  -------------
		s.anisotropy = 8;  s.view_distance = 16000;  s.terdetail = 0.9f;  s.terdist = 800.f;  s.road_dist = 2.4;
		s.tex_size = 1;  s.ter_mtr = 2;  s.ter_tripl = 2;  s.use_impostors = 1;  s.impostors_only = 0;
		s.water_reflect = 1;  s.water_refract = 1;  s.water_rttsize = 1;
		s.shadow_type = Sh_Depth;  s.shadow_size = 3;  s.shadow_count = 3;  s.shadow_dist = 2400;
		s.gui.trees = 2.f;  s.grass = 2.f;  s.trees_dist = 2.f;  s.grass_dist = 3.f;	break;

	case 7:  // Impossible  -------------
		s.anisotropy = 8;  s.view_distance = 20000;  s.terdetail = 0.8f;  s.terdist = 1000.f;  s.road_dist = 3.0;
		s.tex_size = 1;  s.ter_mtr = 2;  s.ter_tripl = 2;  s.use_impostors = 1;  s.impostors_only = 0;
		s.water_reflect = 1;  s.water_refract = 1;  s.water_rttsize = 2;
		s.shadow_type = Sh_Depth;  s.shadow_size = 3;  s.shadow_count = 3;  s.shadow_dist = 3000;
		s.gui.trees = 2.f;  s.grass = 2.f;  s.trees_dist = 2.f;  s.grass_dist = 3.f;	break;
	}
#ifndef SR_EDITOR  /// game only
	s.particles = val >= 1;  s.trails = val >= 1;
	
	s.rpl_rec   = val >= 1;
	s.rpl_ghost = val >= 1;
	
	s.all_effects = false;  // todo: broken  val >= 4;  // only bloom on Higher
	s.bloom = val >= 3;
	s.blur = val >= 5;
	s.softparticles = val >= 5;
	s.ssao = val >= 6;
	s.dof = val >= 7;
	s.godrays = val >= 7;
	s.hdr = false;  s.filmgrain = false;  // hdr always off  // TODO: saturated hdr .. ?

	switch (val)
	{
	case 0:  // Lowest  -------------
		s.particles_len = 1.f;  s.trails_len = 1.f;
		s.refl_mode = 0;  s.refl_skip = 150;  s.refl_faces = 1;  s.refl_size = 0;  s.refl_dist = 20.f;  break;

	case 1:  // Low  -------------
		s.particles_len = 1.f;  s.trails_len = 1.f;
		s.refl_mode = 0;  s.refl_skip = 70;  s.refl_faces = 1;  s.refl_size = 0;  s.refl_dist = 50.f;  break;

	case 2:  // Medium  -------------
		s.particles_len = 1.f;  s.trails_len = 1.5f;
		s.refl_mode = 1;  s.refl_skip = 40;  s.refl_faces = 1;  s.refl_size = 0;  s.refl_dist = 100.f;  break;

	case 3:  // High  -------------
		s.particles_len = 1.2f;  s.trails_len = 2.f;
		s.refl_mode = 1;  s.refl_skip = 10;  s.refl_faces = 1;  s.refl_size = 0;  s.refl_dist = 150.f;  break;

	case 4:  // Higher  -------------
		s.particles_len = 1.5f;  s.trails_len = 3.f;
		s.refl_mode = 1;  s.refl_skip = 0;  s.refl_faces = 1;  s.refl_size = 0;  s.refl_dist = 200.f;  break;

	case 5:  // Very High  -------------
		s.particles_len = 1.5f;  s.trails_len = 3.f;
		s.refl_mode = 1;  s.refl_skip = 0;  s.refl_faces = 1;  s.refl_size = 1;  s.refl_dist = 300.f;  break;

	case 6:  // Ultra  -------------
		s.particles_len = 1.5f;  s.trails_len = 4.f;
		s.refl_mode = 1;  s.refl_skip = 0;  s.refl_faces = 1;  s.refl_size = 1;  s.refl_dist = 400.f;  break;

	case 7:  // Impossible  -------------
		s.particles_len = 1.5f;  s.trails_len = 4.f;
		s.refl_mode = 1;  s.refl_skip = 0;  s.refl_faces = 1;  s.refl_size = 2;  s.refl_dist = 500.f;  break;
	}
#endif

#ifdef SR_EDITOR  /// editor only
	switch (val)
	{
	case 0:  // Lowest  -------------
		s.trackmap = 0;  s.brush_prv = 0;	s.ter_skip = 20;  s.mini_skip = 20;  break;

	case 1:  // Low  -------------
		s.trackmap = 1;  s.brush_prv = 0;	s.ter_skip = 10;  s.mini_skip = 20;  break;

	case 2:  // Medium  -------------
		s.trackmap = 1;  s.brush_prv = 1;	s.ter_skip = 3;  s.mini_skip = 6;  break;

	case 3:  // High  -------------
		s.trackmap = 1;  s.brush_prv = 1;	s.ter_skip = 2;  s.mini_skip = 4;  break;

	case 4:  // Higher  -------------
		s.trackmap = 1;  s.brush_prv = 1;	s.ter_skip = 2;  s.mini_skip = 2;  break;

	case 5:  // Very High  -------------
		s.trackmap = 1;  s.brush_prv = 1;	s.ter_skip = 1;  s.mini_skip = 2;  break;

	case 6:  // Ultra  -------------
		s.trackmap = 1;  s.brush_prv = 1;	s.ter_skip = 1;  s.mini_skip = 1;  break;

	case 7:  // Impossible  -------------
		s.trackmap = 1;  s.brush_prv = 1;	s.ter_skip = 1;  s.mini_skip = 1;  break;
	}
#endif


	//  update gui  sld,val,chk
	//-----------------------------------------------------------------
	///  common only
	svTerDetail.Upd();  svTerDist.Upd();  svRoadDist.Upd();
	svViewDist.Upd();  svAnisotropy.Upd();
	svTexSize.Upd();  svTerMtr.Upd();  svTerTripl.Upd();

	svTrees.Upd();  svTreesDist.Upd();
	svGrass.Upd();	svGrassDist.Upd();
	ckUseImposters.Upd();  ckImpostorsOnly.Upd();

	svShadowType.Upd();  svShadowCount.Upd();
	svShadowSize.Upd();  svShadowDist.Upd();
		
	ckWaterReflect.Upd();  ckWaterRefract.Upd();
	svWaterSize.Upd();


	app->gui->UpdGuiAfterPreset();

	app->scn->mWaterRTT->setRTTSize(ciShadowSizesA[pSet->water_rttsize]);
	chkWater(0);
	//^app->changeShadows();  // apply shadow, material factory generate
}


void CGui::UpdGuiAfterPreset()
{
#ifndef SR_EDITOR
	///  game only
	ckParticles.Upd();  ckTrails.Upd();
	svParticles.Upd();  svTrails.Upd();

	svReflSkip.Upd();  svReflFaces.Upd();  svReflSize.Upd();
	svReflDist.Upd();  svReflMode.Upd();

	ckAllEffects.Upd();
	ckBloom.Upd();  ckBlur.Upd();  //ckHDR.Upd();
	ckSoftPar.Upd();  ckSSAO.Upd();
	ckGodRays.Upd();  ckDoF.Upd();
	
	ckRplAutoRec.Upd();
	ckRplGhost.Upd();
#else
	///  editor only
	ckMinimap.Upd();
	svMiniUpd.Upd();  svTerUpd.Upd();
#endif
}
