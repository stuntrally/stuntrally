#include "stdafx.h"
#include "settings.h"


void SETTINGS::Load(std::string sfile) {  CONFIGFILE c;  c.Load(sfile);  Serialize(false, c);  }
void SETTINGS::Save(std::string sfile) {  CONFIGFILE c;  c.Load(sfile);  Serialize(true, c);  c.Write();  }

void SETTINGS::Serialize(bool w, CONFIGFILE & c)
{
	c.bFltFull = false;
	Param(c,w, "game.car", car);			Param(c,w, "game.car_hue", car_hue);
	Param(c,w, "game.car_sat", car_sat);	Param(c,w, "game.car_val", car_val);
	Param(c,w, "game.opp_car", car_ai);
	Param(c,w, "game.track", track);

	Param(c,w, "game2.ai_diff", ai_difficulty);		Param(c,w, "game2.record", recordreplay);
	Param(c,w, "game2.replay_sel", selected_replay);
	Param(c,w, "game2.trk_reverse", trackreverse);	Param(c,w, "game2.laps", number_of_laps);

	Param(c,w, "display_show.fps", show_fps);		Param(c,w, "display_show.gauges", show_gauges);
	Param(c,w, "display_show.trackmap", trackmap);	Param(c,w, "display_show.racingline", racingline);
	Param(c,w, "display_show.caminfo", show_cam);	Param(c,w, "display_show.times", show_times);
	Param(c,w, "display_show.car_dbgtxt", car_dbgtxt);Param(c,w, "display_show.show_cardbg", car_dbgbars);
	Param(c,w, "display_show.mph", show_mph);
	Param(c,w, "display_size.gauges", size_gauges);	Param(c,w, "display_size.minimap", size_minimap);

	Param(c,w, "display_par.anisotropy", anisotropy);	Param(c,w, "display_par.view_dist", view_distance);
	Param(c,w, "display_par.ter_detail", terdetail);	Param(c,w, "display_par.ter_dist", terdist);
	Param(c,w, "display_par.road_dist", road_dist);

	Param(c,w, "display_eff.particles", particles);		Param(c,w, "display_eff.trails", trails);
	Param(c,w, "display_eff.particles_len", particles_len);  Param(c,w, "display_eff.trail_len", trails_len);
	Param(c,w, "display_adv.trees", trees);				Param(c,w, "display_adv.grass", grass);
	Param(c,w, "display_adv.trees_dist", trees_dist);	Param(c,w, "display_adv.grass_dist", grass_dist);

	Param(c,w, "shadow.dist", shadow_dist);		Param(c,w, "shadow.size", shadow_size);
	Param(c,w, "shadow.count",shadow_count);	Param(c,w, "shadow.type", shadow_type);
	Param(c,w, "shadow.shaders", shaders);
		
	Param(c,w, "reflect.skip_frames", refl_skip);	Param(c,w, "reflect.faces_once", refl_faces);
	Param(c,w, "reflect.map_size", refl_size);		Param(c,w, "reflect.dist", refl_dist);

	Param(c,w, "joystick.type", joytype);		Param(c,w, "joystick.two_hundred", joy200);  joy200 = false;
	Param(c,w, "joystick.calibrated", joystick_calibrated);
	Param(c,w, "joystick.ff_device", ff_device);	Param(c,w, "joystick.ff_gain", ff_gain);
	Param(c,w, "joystick.ff_invert", ff_invert);	Param(c,w, "joystick.hgateshifter", hgateshifter);

	Param(c,w, "control.autoclutch", autoclutch);	Param(c,w, "control.autotrans", autoshift);
	Param(c,w, "control.autorear", autorear);
	Param(c,w, "control.speed_sens", speed_sensitivity);	Param(c,w, "control.button_ramp", button_ramp);
	Param(c,w, "control.abs", abs);				Param(c,w, "control.tcs", tcs);
	
	Param(c,w, "misc.skin", skin);				Param(c,w, "misc.volume", vol_master);
	Param(c,w, "misc.vol_engine", vol_engine);	Param(c,w, "misc.vol_tires", vol_tires);
	Param(c,w, "misc.vol_env", vol_env);

	Param(c,w, "misc.autostartgame", autostart);
	Param(c,w, "misc.ogredialog", ogre_dialog);	Param(c,w, "misc.escquit", escquit);
	Param(c,w, "misc.bulletDebug", bltDebug);	Param(c,w, "misc.bulletLines", bltLines);
	Param(c,w, "misc.loadingbackground", loadingbackground);
	
	//c.bFltFull = true;
	Param(c,w, "sim.game_fq", game_fq);	Param(c,w, "sim.mult_thr", mult_thr);
	Param(c,w, "sim.blt_fq", blt_fq);	Param(c,w, "sim.blt_iter", blt_iter);
	
	Param(c,w, "video.bloom", bloom); 		Param(c,w, "video.bloomintensity", bloomintensity);
	Param(c,w, "video.bloomorig", bloomorig);		   Param(c,w, "video.hdr", hdr);
	Param(c,w, "video.motionblur", motionblur); 	Param(c,w, "video.motionblurintensity", motionblurintensity);
	Param(c,w, "video.windowx", windowx);      Param(c,w, "video.windowy", windowy);
	Param(c,w, "video.fullscreen", fullscreen);
	Param(c,w, "video.fsaa", fsaa); 		   Param(c,w, "video.vsync", vsync);
}

SETTINGS::SETTINGS() :  ///  Defaults
	//  car, track
	car("360"), car_ai("360"), track("Jungle-T"),
	car_hue(0.f), car_sat(0.f), car_val(0.f),
	//  show
	show_fps(1), show_gauges(1), trackmap(1), racingline(1),
	show_cam(1), show_times(0), car_dbgtxt(0), car_dbgbars(0),
	size_gauges(0.18), size_minimap(0.2),
	//  graphics
	anisotropy(4),	view_distance(1500), bFog(0),
	terdetail(2), terdist(100), road_dist(1.0),
	particles(true), trails(true),
	shadow_dist(3000), shadow_size(2), shadow_count(3), shadow_type(1),
	refl_skip(10), refl_faces(1), refl_size(0), refl_dist(500.f),
	shaders(0),  trees(1.f), grass(1.f), trees_dist(1.f), grass_dist(1.f),
	particles_len(1.f), trails_len(1.f),
	//  joy input
	joytype("joystick"), joy200(false), joystick_calibrated(false),
	speed_sensitivity(0.5), hgateshifter(false),  button_ramp(7.5),
	ff_device("/dev/input/event0"), ff_gain(2.0), ff_invert(false),
	//  car
	abs(1), tcs(1), autoclutch(1), autoshift(1), autorear(1), show_mph(0),
	//  game
	recordreplay(false), selected_replay(0),
	trackreverse(false), number_of_laps(1), ai_difficulty(1),
	//  other
	skin("simple"), vol_master(1.f), vol_engine(1.f), vol_tires(1.f), vol_env(1.f),
	autostart(0), ogre_dialog(1), escquit(0), bltDebug(0), bltLines(1),
	// loading
	loadingbackground(true), veget_collis(true),
	//  sim
	game_fq(100.f), blt_fq(60.f), blt_iter(7), mult_thr(0),  //low
	// video
	bloom(false), bloomintensity(0.2), bloomorig(1.0), hdr(false), motionblur(false), motionblurintensity(0.3), windowx(800), windowy(600), fullscreen(false), fsaa(0), vsync(false)
{}
