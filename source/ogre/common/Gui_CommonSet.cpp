#include "pch.h"
#include "../common/Defines.h"
#include "../../road/Road.h"
#include "../../vdrift/pathmanager.h"
#ifndef ROAD_EDITOR
	#include "../../vdrift/game.h"
	#include "../OgreGame.h"
	#include "../SplitScreen.h"
#else
	#include "../../editor/OgreApp.h"
#endif
#include <OgreRoot.h>
#include <OgreMaterialManager.h>
#include <OgreSceneManager.h>
#include <OgreTerrain.h>
#include <OgreRenderWindow.h>
#include "Gui_Def.h"
#include "Slider.h"
using namespace MyGUI;
using namespace Ogre;


///  change all Graphics settings
///..............................................................................................................................
void App::comboGraphicsAll(ComboBoxPtr cmb, size_t val)
{
	//"TexFiltering", comboTexFilter ?
	//fsaa = 0;  vsync = false;  //?  rpl?
	//  sim  - other combobox, not recommended_
	//game_fq = 100.f;  blt_fq = 60.f;  blt_iter = 7;  dyn_iter = 10;  mult_thr = 0;  //<low
	//veget_collis = true;  car_collis = false;

	SETTINGS& s = *pSet;
	switch (val)        ///  common
	{
	case 0:  // Lowest  -------------
		s.anisotropy = 0;  s.view_distance = 1000;  s.terdetail = 2.0f;  s.terdist = 0.f;  s.road_dist = 1.0;
		s.tex_size = 0;  s.ter_mtr = 0;  s.shaders = 0;  s.use_imposters = 0;  s.imposters_only = 1;
		s.water_reflect = 0;  s.water_refract = 0;  s.water_rttsize = 0;
		s.shadow_type = Sh_None;  s.shadow_size = 0;  s.shadow_count = 1;  s.shadow_dist = 100;  s.shadow_filter = 1;
		s.gui.trees = 0.f;  s.grass = 0.f;  s.trees_dist = 1.f;  s.grass_dist = 1.f;	break;

	case 1:  // Low  -------------
		s.anisotropy = 0;  s.view_distance = 1500;  s.terdetail = 1.7f;  s.terdist = 40.f;  s.road_dist = 1.2;
		s.tex_size = 0;  s.ter_mtr = 1;  s.shaders = 0.25;  s.use_imposters = 0;  s.imposters_only = 1;
		s.water_reflect = 0;  s.water_refract = 0;  s.water_rttsize = 0;
		s.shadow_type = Sh_None;  s.shadow_size = 0;  s.shadow_count = 1;  s.shadow_dist = 100;  s.shadow_filter = 1;
		s.gui.trees = 0.f;  s.grass = 0.f;  s.trees_dist = 1.f;  s.grass_dist = 1.f;	break;

	case 2:  // Medium  -------------
		s.anisotropy = 2;  s.view_distance = 2500;  s.terdetail = 1.5f;  s.terdist = 80.f;  s.road_dist = 1.4;
		s.tex_size = 1;  s.ter_mtr = 1;  s.shaders = 0.5;  s.use_imposters = 1;  s.imposters_only = 1;
		s.water_reflect = 0;  s.water_refract = 0;  s.water_rttsize = 0;
		s.shadow_type = Sh_Simple;  s.shadow_size = 1;  s.shadow_count = 1;  s.shadow_dist = 200;  s.shadow_filter = 2;
		s.gui.trees = 0.5f;  s.grass = 1.f;  s.trees_dist = 1.f;  s.grass_dist = 1.f;	break;

	case 3:  // High  -------------
		s.anisotropy = 4;  s.view_distance = 6000;  s.terdetail = 1.3f;  s.terdist = 200.f;  s.road_dist = 1.6;
		s.tex_size = 1;  s.ter_mtr = 2;  s.shaders = 0.75;  s.use_imposters = 1;  s.imposters_only = 0;
		s.water_reflect = 1;  s.water_refract = 0;  s.water_rttsize = 1;
		s.shadow_type = Sh_Depth;  s.shadow_size = 2;  s.shadow_count = 1;  s.shadow_dist = 1000;  s.shadow_filter = 2;
		s.gui.trees = 1.f;  s.grass = 1.f;  s.trees_dist = 1.f;  s.grass_dist = 2.f;	break;

	case 4:  // Very High  -------------
		s.anisotropy = 8;  s.view_distance = 8000;  s.terdetail = 1.2f;  s.terdist = 400.f;  s.road_dist = 2.0;
		s.tex_size = 1;  s.ter_mtr = 3;  s.shaders = 1;  s.use_imposters = 1;  s.imposters_only = 0;
		s.water_reflect = 1;  s.water_refract = 1;  s.water_rttsize = 1;
		s.shadow_type = Sh_Depth;  s.shadow_size = 3;  s.shadow_count = 2;  s.shadow_dist = 2000;  s.shadow_filter = 2;
		s.gui.trees = 1.5f;  s.grass = 1.f;  s.trees_dist = 1.f;  s.grass_dist = 3.f;	break;

	case 5:  // Ultra  -------------
		s.anisotropy = 16;  s.view_distance = 20000;  s.terdetail = 1.0f;  s.terdist = 1000.f;  s.road_dist = 3.0;
		s.tex_size = 1;  s.ter_mtr = 4;  s.shaders = 1;  s.use_imposters = 1;  s.imposters_only = 0;
		s.water_reflect = 1;  s.water_refract = 1;  s.water_rttsize = 2;
		s.shadow_type = Sh_Depth;  s.shadow_size = 3;  s.shadow_count = 3;  s.shadow_dist = 3000;  s.shadow_filter = 4;
		s.gui.trees = 2.f;  s.grass = 2.f;  s.trees_dist = 2.f;  s.grass_dist = 4.f;	break;
	}
#ifndef ROAD_EDITOR  /// game only
	s.particles = val >= 1;  s.trails = val >= 1;
	
	s.rpl_rec   = val >= 1;
	s.rpl_ghost = val >= 1;  s.rpl_alpha = val <= 1;
	
	s.all_effects = val >= 3;  // only bloom on High
	s.bloom = val >= 2;
	s.motionblur = false;  s.camblur = false;  // blur always off  // TODO: new blur is b0rked in gl ..restore old implementation ?
	s.softparticles = val >= 4;
	s.ssao = val >= 5;
	s.dof = val >= 5;
	s.godrays = val >= 5;
	s.hdr = false;  s.filmgrain = false;  // hdr always off  // TODO: saturated hdr ?

	switch (val)
	{
	case 0:  // Lowest  -------------
		s.particles_len = 1.f;  s.trails_len = 1.f;
		s.refl_mode = 0;  s.refl_skip = 150;  s.refl_faces = 1;  s.refl_size = 0;  s.refl_dist = 20.f;  break;

	case 1:  // Low  -------------
		s.particles_len = 1.f;  s.trails_len = 1.f;
		s.refl_mode = 0;  s.refl_skip = 100;  s.refl_faces = 1;  s.refl_size = 0;  s.refl_dist = 50.f;  break;

	case 2:  // Medium  -------------
		s.particles_len = 1.f;  s.trails_len = 1.5f;
		s.refl_mode = 1;  s.refl_skip = 70;  s.refl_faces = 1;  s.refl_size = 0;  s.refl_dist = 100.f;  break;

	case 3:  // High  -------------
		s.particles_len = 1.2f;  s.trails_len = 2.f;
		s.refl_mode = 1;  s.refl_skip = 40;  s.refl_faces = 1;  s.refl_size = 0;  s.refl_dist = 150.f;  break;

	case 4:  // Very High  -------------
		s.particles_len = 1.5f;  s.trails_len = 3.f;
		s.refl_mode = 1;  s.refl_skip = 10;  s.refl_faces = 1;  s.refl_size = 0;  s.refl_dist = 200.f;  break;

	case 5:  // Ultra  -------------
		s.particles_len = 1.5f;  s.trails_len = 4.f;
		s.refl_mode = 1;  s.refl_skip = 1;  s.refl_faces = 1;  s.refl_size = 0;  s.refl_dist = 300.f;  break;
	}
#endif

#ifdef ROAD_EDITOR  /// editor only
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

	case 4:  // Very High  -------------
		s.trackmap = 1;  s.brush_prv = 1;	s.ter_skip = 2;  s.mini_skip = 2;  break;

	case 5:  // Ultra  -------------
		s.trackmap = 1;  s.brush_prv = 1;	s.ter_skip = 1;  s.mini_skip = 1;  break;
	}
#endif

	//  update gui  sld,val,chk  ...
	GuiInitGraphics();  // += newDelegate..?

	ButtonPtr bchk;  Slider* sl;
#ifndef ROAD_EDITOR  /// game only
	// duplicated code..
	Chk("ParticlesOn", chkParticles, pSet->particles);	Chk("TrailsOn", chkTrails, pSet->trails);
	Slv(Particles,	powf(pSet->particles_len /4.f, 0.5f));
	Slv(Trails,		powf(pSet->trails_len /4.f, 0.5f));

	Slv(ReflSkip,	powf(pSet->refl_skip /1000.f, 0.5f));
	Slv(ReflSize,	pSet->refl_size /float(ciShadowNumSizes));
	Slv(ReflFaces,	pSet->refl_faces /6.f);
	Slv(ReflDist,	powf((pSet->refl_dist -20.f)/1480.f, 0.5f));
	Slv(ReflMode,   pSet->refl_mode /2.f);

	Chk("AllEffects", chkVidEffects, pSet->all_effects);
	Chk("Bloom", chkVidBloom, pSet->bloom);
	//Chk("HDR", chkVidHDR, pSet->hdr);
	Chk("MotionBlur", chkVidBlur, pSet->camblur);
	Chk("softparticles", chkVidSoftParticles, pSet->softparticles);
	Chk("ssao", chkVidSSAO, pSet->ssao);
	Chk("DepthOfField", chkVidDepthOfField, pSet->dof);
	Chk("godrays", chkVidGodRays, pSet->godrays);
	Chk("ssao", chkVidSSAO, pSet->ssao);
	Chk("godrays", chkVidGodRays, pSet->godrays);
	
	Chk("RplChkAutoRec", chkRplAutoRec, pSet->rpl_rec);
	Chk("RplChkGhost", chkRplChkGhost, pSet->rpl_ghost);
	Chk("RplChkAlpha", chkRplChkAlpha, pSet->rpl_alpha);
#endif

#ifdef ROAD_EDITOR  /// editor only
	Chk("Minimap", chkMinimap, pSet->trackmap);
	Slv(TerUpd, pSet->ter_skip /20.f);
	Slv(MiniUpd, pSet->mini_skip /20.f);
#endif

	Chk("WaterReflection", chkWaterReflect, pSet->water_reflect);
	Chk("WaterRefraction", chkWaterRefract, pSet->water_refract);
	Slv(WaterSize, pSet->water_rttsize / 2.f)
	mWaterRTT.setRTTSize(ciShadowSizesA[pSet->water_rttsize]);
	mWaterRTT.recreate();

	changeShadows(); // apply shadow, material factory generate
}
