#include "pch.h"
#include "../ogre/common/Defines.h"
#include "settings.h"
#include "../vdrift/tracksurface.h"


void SETTINGS::Load(std::string sfile)
{
	CONFIGFILE c;  c.Load(sfile);
	Serialize(false, c);
}
void SETTINGS::Save(std::string sfile)
{
	CONFIGFILE c;  c.Load(sfile);  version = SET_VER;
	Serialize(true, c);  c.Write();
}

void SETTINGS::Serialize(bool w, CONFIGFILE & c)
{
	c.bFltFull = false;
	Param(c,w, "game.in_menu", inMenu);			Param(c,w, "game.in_main", isMain);
	Param(c,w, "game.track", gui.track);		Param(c,w, "game.track_user", gui.track_user);

	Param(c,w, "hud_show.fps", show_fps);
	Param(c,w, "hud_show.trackmap", trackmap);				Param(c,w, "hud_size.minimap", size_minimap);
	Param(c,w, "hud_show.mini_num", num_mini);				Param(c,w, "hud_show.brushpreview", brush_prv);
	Param(c,w, "hud_show.tracks_view", tracks_view);
	Param(c,w, "hud_show.tracks_sort", tracks_sort);		Param(c,w, "hud_show.tracks_sortup", tracks_sortup);

	Param(c,w, "graph_detail.anisotropy", anisotropy);		Param(c,w, "graph_detail.view_dist", view_distance);
	Param(c,w, "graph_detail.ter_detail", terdetail);		Param(c,w, "graph_detail.ter_dist", terdist);
	Param(c,w, "graph_detail.road_dist", road_dist);		Param(c,w, "graph_detail.tex_size", tex_size);
	Param(c,w, "graph_detail.ter_mtr", ter_mtr);

	Param(c,w, "graph_shadow.dist", shadow_dist);			Param(c,w, "graph_shadow.size", shadow_size);
	Param(c,w, "graph_shadow.count",shadow_count);			Param(c,w, "graph_shadow.type", shadow_type);
	Param(c,w, "graph_shadow.shaders", shaders);			Param(c,w, "graph_shadow.lightmap_size", lightmap_size);
	Param(c,w, "graph_shadow.filter", shadow_filter);
	
	Param(c,w, "graph_reflect.water_reflect", water_reflect); Param(c,w, "graph_reflect.water_refract", water_refract);
	Param(c,w, "graph_reflect.water_rttsize", water_rttsize);

	Param(c,w, "graph_veget.trees", gui.trees);				Param(c,w, "graph_veget.grass", grass);
	Param(c,w, "graph_veget.trees_dist", trees_dist);		Param(c,w, "graph_veget.grass_dist", grass_dist);
	Param(c,w, "graph_veget.use_imposters", use_imposters);
	
	Param(c,w, "misc.allow_save", allow_save);
	Param(c,w, "misc.version", version);			Param(c,w, "misc.autostart", autostart);
	Param(c,w, "misc.ogredialog", ogre_dialog);		Param(c,w, "misc.escquit", escquit);
	Param(c,w, "misc.language", language);			Param(c,w, "misc.x11_capture_mouse", x11_capture_mouse);
	
	Param(c,w, "set_cam.px",cam_x);  Param(c,w, "set_cam.py",cam_y);  Param(c,w, "set_cam.pz",cam_z);
	Param(c,w, "set_cam.dx",cam_dx); Param(c,w, "set_cam.dy",cam_dy); Param(c,w, "set_cam.dz",cam_dz);

	Param(c,w, "set.fog", bFog);					Param(c,w, "set.trees", bTrees);
	Param(c,w, "set.cam_speed", cam_speed);			Param(c,w, "set.cam_inert", cam_inert);
	Param(c,w, "set.ter_skip", ter_skip);			Param(c,w, "set.road_sphr", road_sphr);
	Param(c,w, "set.mini_skip", mini_skip);
	
	Param(c,w, "video.windowx", windowx);			Param(c,w, "video.windowy", windowy);
	Param(c,w, "video.fullscreen", fullscreen);
	Param(c,w, "video.fsaa", fsaa);					Param(c,w, "video.vsync", vsync);
	Param(c,w, "video.buffer", buffer);				Param(c,w, "video.rendersystem", rendersystem);
	
	Param(c,w, "generate.scale", gen_scale);
	Param(c,w, "generate.freq", gen_freq);			Param(c,w, "generate.persist", gen_persist);
	Param(c,w, "generate.pow", gen_pow);			Param(c,w, "generate.oct", gen_oct);
	Param(c,w, "generate.ofsx", gen_ofsx);			Param(c,w, "generate.ofsy", gen_ofsy);
}

SETTINGS::SETTINGS() :  ///  Defaults
	version(100),  // old
	//  show
	show_fps(1), trackmap(1), size_minimap(0.5), num_mini(0), brush_prv(1),
	tracks_view(0), tracks_sort(0), tracks_sortup(0),
	//  graphics
	anisotropy(8),	view_distance(3600),
	terdetail(1.57), terdist(300), road_dist(1.0), tex_size(1), ter_mtr(2),
	shadow_dist(2000), shadow_size(2), shadow_count(3), shadow_type(2), shadow_filter(1),  /*<+*/
	shaders(0.5), lightmap_size(0),
	grass(1.f), trees_dist(1.f), grass_dist(1.f),use_imposters(false),
	water_reflect(0), water_refract(0), water_rttsize(0),
	//  startup
	autostart(0), ogre_dialog(1), escquit(0), language(""), allow_save(0),
	isMain(1), inMenu(0),
	//  settings
	cam_x(0), cam_y(50),cam_z(-120),  cam_dx(0), cam_dy(0), cam_dz(1),
	bFog(0), bTrees(0),
	cam_speed(1.f), cam_inert(1.f),
	ter_skip(4), road_sphr(1.f), mini_skip(4),
	windowx(800), windowy(600), fullscreen(false), fsaa(0), vsync(false),
	buffer("FBO"), rendersystem("OpenGL Rendering Subsystem"), x11_capture_mouse(false),
	//  ter gen
	gen_scale(20.f), gen_freq(0.2f), gen_oct(3), gen_persist(0.4f),
	gen_pow(1.5f), gen_ofsx(0.f), gen_ofsy(0.f)
{
	gui.track = "J1-T";
	gui.track_user = false;
	gui.trees = 1.f;
}
