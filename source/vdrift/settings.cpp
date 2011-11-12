#include "pch.h"
#include "settings.h"


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
	for (int i=0; i < 4; ++i)
	{	std::string s = "game.car";  s += char('1'+i);
		Param(c,w, s, car[i]);						Param(c,w, s+"_hue", car_hue[i]);
		Param(c,w, s+"_sat", car_sat[i]);			Param(c,w, s+"_val", car_val[i]);
		Param(c,w, s+"_cam", cam_view[i]);
	}
	Param(c,w, "game.track", track);						Param(c,w, "game.track_user", track_user);
	Param(c,w, "game.local_players", local_players);		Param(c,w, "game.split_vertically", split_vertically);
	Param(c,w, "game.trk_reverse", trackreverse);			Param(c,w, "game.num_laps", num_laps);
	Param(c,w, "game.language", language);

	Param(c,w, "display_show.fps", show_fps);				Param(c,w, "display_show.mph", show_mph);
	Param(c,w, "display_show.gauges", show_gauges);			Param(c,w, "display_show.show_digits", show_digits);
	Param(c,w, "display_show.trackmap", trackmap);
	Param(c,w, "display_show.caminfo", show_cam);			Param(c,w, "display_show.times", show_times);
	Param(c,w, "display_show.car_dbgtxt", car_dbgtxt);		Param(c,w, "display_show.show_cardbg", car_dbgbars);
	Param(c,w, "display_show.tracks_view", tracks_view);
	Param(c,w, "display_show.tracks_sort", tracks_sort);	Param(c,w, "display_show.tracks_sortup", tracks_sortup);
	Param(c,w, "display_show.check_arrow", check_arrow);

	Param(c,w, "display_size.gauges", size_gauges);			Param(c,w, "display_size.minimap", size_minimap);
	Param(c,w, "display_size.mini_zoom", zoom_minimap);		Param(c,w, "display_size.mini_zoomed", mini_zoomed);
	Param(c,w, "display_size.mini_rotated", mini_rotated);	Param(c,w, "display_size.mini_terrain", mini_terrain);
	Param(c,w, "display_size.arrow", size_arrow);

	Param(c,w, "display_par.anisotropy", anisotropy);		Param(c,w, "display_par.view_dist", view_distance);
	Param(c,w, "display_par.ter_detail", terdetail);		Param(c,w, "display_par.ter_dist", terdist);
	Param(c,w, "display_par.road_dist", road_dist);			Param(c,w, "display_par.tex_size", tex_size);
	Param(c,w, "display_par.ter_mtr", ter_mtr);

	Param(c,w, "display_eff.particles", particles);			Param(c,w, "display_eff.trails", trails);
	Param(c,w, "display_eff.particles_len", particles_len);	Param(c,w, "display_eff.trail_len", trails_len);
	Param(c,w, "display_adv.trees", trees);					Param(c,w, "display_adv.grass", grass);
	Param(c,w, "display_adv.trees_dist", trees_dist);		Param(c,w, "display_adv.grass_dist", grass_dist);
	Param(c,w, "display_adv.use_imposters", use_imposters);

	Param(c,w, "shadow.dist", shadow_dist);			Param(c,w, "shadow.size", shadow_size);
	Param(c,w, "shadow.count",shadow_count);		Param(c,w, "shadow.type", shadow_type);
	Param(c,w, "shadow.shaders", shaders);			Param(c,w, "shadow.lightmap_size", lightmap_size);
		
	Param(c,w, "reflect.skip_frames", refl_skip);	Param(c,w, "reflect.faces_once", refl_faces);
	Param(c,w, "reflect.map_size", refl_size);		Param(c,w, "reflect.dist", refl_dist);
	Param(c,w, "reflect.mode", refl_mode);

	Param(c,w, "control.autotrans", autoshift);
	Param(c,w, "control.autorear", autorear);		Param(c,w, "control.rear_inv", rear_inv);
	Param(c,w, "control.abs", abs);					Param(c,w, "control.tcs", tcs);
	Param(c,w, "control.veget_collis", veget_collis);
	Param(c,w, "control.car_collis", car_collis);
	
	Param(c,w, "misc.volume", vol_master);			Param(c,w, "misc.vol_engine", vol_engine);
	Param(c,w, "misc.vol_tires", vol_tires);		Param(c,w, "misc.vol_env", vol_env);

	Param(c,w, "misc.version", version);			Param(c,w, "misc.autostartgame", autostart);
	Param(c,w, "misc.ogredialog", ogre_dialog);		Param(c,w, "misc.escquit", escquit);
	Param(c,w, "misc.bulletDebug", bltDebug);		Param(c,w, "misc.bulletLines", bltLines);
	Param(c,w, "misc.bulletProfilerTxt", bltProfilerTxt);
	Param(c,w, "misc.loadingback", loadingbackground);
	Param(c,w, "misc.sceneryIdOld", sceneryIdOld);	Param(c,w, "input.x11_capture_mouse", x11_capture_mouse);
	
	Param(c,w, "sim.game_fq", game_fq);				Param(c,w, "sim.mult_thr", mult_thr);
	Param(c,w, "sim.blt_fq", blt_fq);				Param(c,w, "sim.blt_iter", blt_iter);
	
	Param(c,w, "video.bloom", bloom);				Param(c,w, "video.bloomintensity", bloomintensity);
	Param(c,w, "video.bloomorig", bloomorig);		Param(c,w, "video.hdr", hdr);
	Param(c,w, "video.motionblur", motionblur);		Param(c,w, "video.motionblurintensity", motionblurintensity);
	Param(c,w, "video.all_effects", all_effects);
	Param(c,w, "video.windowx", windowx);			Param(c,w, "video.windowy", windowy);
	Param(c,w, "video.fullscreen", fullscreen);
	Param(c,w, "video.fsaa", fsaa);					Param(c,w, "video.vsync", vsync);
	Param(c,w, "video.buffer", buffer);				Param(c,w, "video.rendersystem", rendersystem);
	Param(c,w, "video.ssaa", ssaa);
	Param(c,w, "video.ssao", ssao);

	Param(c,w, "replay.rec", rpl_rec);				Param(c,w, "replay.ghost", rpl_ghost);
	Param(c,w, "replay.bestonly", rpl_bestonly);	Param(c,w, "replay.listview", rpl_listview);
	Param(c,w, "replay.alpha", rpl_alpha);			Param(c,w, "replay.ghostpar", rpl_ghostpar);
}

SETTINGS::SETTINGS() :  ///  Defaults
	version(100),  // old
	//  car, track
	track("J1-T"), track_user(false),
	//  show
	show_fps(1), show_gauges(1), trackmap(1),
	show_cam(1), show_times(0), show_digits(1), car_dbgtxt(0), car_dbgbars(0),
	size_gauges(0.18), size_minimap(0.2), zoom_minimap(1.0),
	mini_zoomed(0), mini_rotated(1), mini_terrain(0),
	tracks_view(0), tracks_sort(0), tracks_sortup(0),
	check_arrow(0), size_arrow(0.2),
	//  graphics
	anisotropy(4),	view_distance(1500), bFog(0),
	terdetail(2), terdist(100), road_dist(1.0), tex_size(1), ter_mtr(2),
	particles(true), trails(true),
	shadow_dist(3000), shadow_size(2), shadow_count(3), shadow_type(1), lightmap_size(0),
	refl_skip(10), refl_faces(1), refl_size(0), refl_dist(500.f), refl_mode("single"),
	shaders(0),  trees(1.f), grass(1.f), trees_dist(1.f), grass_dist(1.f),
	particles_len(1.f), trails_len(1.f),use_imposters(true),
	//  car
	abs(1), tcs(1), autoshift(1), autorear(1), rear_inv(1), show_mph(0),
	//  game
	trackreverse(false), local_players(1), num_laps(2),
	split_vertically(true), language(""), // "" = autodetect lang
	//  other
	vol_master(1.f), vol_engine(1.f), vol_tires(1.f), vol_env(1.f),
	autostart(0), ogre_dialog(1), escquit(0), bltDebug(0), bltLines(1),  bltProfilerTxt(0),
	loadingbackground(true),
	//  sim
	game_fq(100.f), blt_fq(60.f), blt_iter(7), mult_thr(0),
	veget_collis(true), car_collis(false),
	//  video
	bloom(false), bloomintensity(0.2), bloomorig(1.0), hdr(false),
	motionblur(false), motionblurintensity(0.3),
	all_effects(false), ssaa(true), ssao(false),
	windowx(800), windowy(600), fullscreen(false), fsaa(0), vsync(false),
	buffer("FBO"), rendersystem("OpenGL Rendering Subsystem"),
	//  input
	x11_capture_mouse(false),
	//  replay
	rpl_rec(1), rpl_ghost(1), rpl_bestonly(1),
	rpl_alpha(0), rpl_ghostpar(0), rpl_listview(0),
	sceneryIdOld(0)
{
	for (int i=0; i < 4; ++i)
	{	car[i] = "ES";  car_hue[i] = 0.2f*i;  car_sat[i] = 0.f;  car_val[i] = 0.f;  cam_view[0] = 0;  }
}
