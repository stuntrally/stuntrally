#include "stdafx.h"
#include "settings.h"
#include "../vdrift/tracksurface.h"


void SETTINGS::Load(std::string sfile) {  CONFIGFILE c;  c.Load(sfile);  Serialize(false, c);  }
void SETTINGS::Save(std::string sfile) {  CONFIGFILE c;  c.Load(sfile);  Serialize(true, c);  c.Write();  }

void SETTINGS::Serialize(bool w, CONFIGFILE & c)
{
	c.bFltFull = false;
	Param(c,w, "game.track", track);
	Param(c,w, "game.track_user", track_user);

	Param(c,w, "display_show.fps", show_fps);
	Param(c,w, "display_show.trackmap", trackmap);		Param(c,w, "display_size.minimap", size_minimap);
	Param(c,w, "display_show.mini_num", num_mini);

	Param(c,w, "display_par.anisotropy", anisotropy);	Param(c,w, "display_par.view_dist", view_distance);
	Param(c,w, "display_par.ter_detail", terdetail);	Param(c,w, "display_par.ter_dist", terdist);
	Param(c,w, "display_par.road_dist", road_dist);

	Param(c,w, "display_adv.trees", trees);				Param(c,w, "display_adv.grass", grass);
	Param(c,w, "display_adv.trees_dist", trees_dist);	Param(c,w, "display_adv.grass_dist", grass_dist);

	Param(c,w, "shadow.dist", shadow_dist);		Param(c,w, "shadow.size", shadow_size);
	Param(c,w, "shadow.count",shadow_count);	Param(c,w, "shadow.type", shadow_type);
	Param(c,w, "shadow.shaders", shaders);

	Param(c,w, "misc.autostart", autostart);
	Param(c,w, "misc.ogredialog", ogre_dialog);	Param(c,w, "misc.escquit", escquit);
	
	Param(c,w, "set_cam.px",cam_x);  Param(c,w, "set_cam.py",cam_y);  Param(c,w, "set_cam.pz",cam_z);
	Param(c,w, "set_cam.dx",cam_dx); Param(c,w, "set_cam.dy",cam_dy); Param(c,w, "set_cam.dz",cam_dz);

	Param(c,w, "set.fog", bFog);	Param(c,w, "set.trees", bTrees);
	Param(c,w, "set.cam_speed", cam_speed);		Param(c,w, "set.cam_inert", cam_inert);
	Param(c,w, "set.ter_skip", ter_skip);		Param(c,w, "set.road_sphr", road_sphr);
	Param(c,w, "set.mini_skip", mini_skip);
	
	Param(c,w, "video.windowx", windowx);      Param(c,w, "video.windowy", windowy);
	Param(c,w, "video.fullscreen", fullscreen);
	Param(c,w, "video.fsaa", fsaa); 		   Param(c,w, "video.vsync", vsync);
	Param(c,w, "video.buffer", buffer);
	Param(c,w, "video.rendersystem", rendersystem);
}

SETTINGS::SETTINGS() :  ///  Defaults
	//  track
	track("J1-T"), track_user(false),
	//  show
	show_fps(1), trackmap(1), size_minimap(0.5), num_mini(0),
	//  graphics
	anisotropy(8),	view_distance(3600),
	terdetail(1.57), terdist(300), road_dist(1.0),
	shadow_dist(2000), shadow_size(2), shadow_count(3), shadow_type(2),  /*<+*/
	shaders(1),  trees(1.f), grass(1.f), trees_dist(1.f), grass_dist(1.f),
	//  startup
	autostart(0), ogre_dialog(1), escquit(0),
	//  settings
	cam_x(0), cam_y(50),cam_z(-120),  cam_dx(0), cam_dy(0), cam_dz(1),
	bFog(0), bTrees(0),
	cam_speed(1.f), cam_inert(1.f),
	ter_skip(4), road_sphr(1.f), mini_skip(4),
	windowx(800), windowy(600), fullscreen(false), fsaa(0), vsync(false),
	buffer("FBO"), rendersystem("OpenGL Rendering Subsystem")
{	}
