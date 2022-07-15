#include "pch.h"
#include "settings.h"
#include "../network/protocol.hpp"
#include <stdio.h>


void SETTINGS::Load(std::string sfile)
{
	CONFIGFILE c;  c.Load(sfile);
	Serialize(false, c);
}
void SETTINGS::Save(std::string sfile)
{
	if (net_local_plr > 0)  // save only for host for many local games
		return;
	CONFIGFILE c;  c.Load(sfile);  version = SET_VER;
	Serialize(true, c);  c.Write();
}

void SETTINGS::Serialize(bool w, CONFIGFILE & c)
{
	c.bFltFull = false;

	SerializeCommon(w,c);

	Param(c,w, "game.menu", iMenu);  Param(c,w, "game.ymain", yMain);  Param(c,w, "game.yrace", yRace);
	Param(c,w, "game.difficulty", difficulty);

	//  game common
	Param(c,w, "game.track", gui.track);				Param(c,w, "game.track_user", gui.track_user);
	Param(c,w, "graph_veget.trees", gui.trees);


	//  cars
	for (int i=0; i < 6; ++i)
	{
		char ss[64];  sprintf(ss, "car%d.", i+1);   std::string s = ss;
		if (i < 4)
		{	Param(c,w, s+"car", gui.car[i]);			Param(c,w, s+"camera", cam_view[i]);
		}
		Param(c,w, s+"clr_hue", gui.car_hue[i]);
		Param(c,w, s+"clr_sat", gui.car_sat[i]);		Param(c,w, s+"clr_val", gui.car_val[i]);
		Param(c,w, s+"clr_gloss", gui.car_gloss[i]);	Param(c,w, s+"clr_refl", gui.car_refl[i]);
	}
	//todo: this for all 4 cars..
	Param(c,w, "car1.autotrans", autoshift);
	Param(c,w, "car1.autorear", autorear);		Param(c,w, "car1.autorear_inv", rear_inv);
	for (int i=0; i <= 1; ++i)
	{	std::string s = i==1 ? "A":"";
		Param(c,w, "car1.abs"+s, abs[i]);		Param(c,w, "car1.tcs"+s, tcs[i]);
		Param(c,w, "car1.sss_effect"+s, sss_effect[i]);
		Param(c,w, "car1.sss_velfactor"+s, sss_velfactor[i]);
		Param(c,w, "car1.steer_range"+s, steer_range[i]);
	}
	Param(c,w, "car1.steer_sim_easy", steer_sim[0]);
	Param(c,w, "car1.steer_sim_normal", steer_sim[1]);

	//  game
	Param(c,w, "game.pre_time", gui.pre_time);			Param(c,w, "game.chall_num", gui.chall_num);  //rem-
	Param(c,w, "game.champ_num", gui.champ_num);		Param(c,w, "game.champ_rev", gui.champ_rev);
	Param(c,w, "game.ch_all", ch_all);
	Param(c,w, "game.boost_type", gui.boost_type);		Param(c,w, "game.flip_type", gui.flip_type);
	Param(c,w, "game.rewind_type", gui.rewind_type);
	Param(c,w, "game.damage_type", gui.damage_type);	Param(c,w, "game.damage_dec", gui.damage_dec);

	Param(c,w, "game.boost_power", gui.boost_power);
	Param(c,w, "game.boost_min", gui.boost_min);		Param(c,w, "game.boost_max", gui.boost_max);
	Param(c,w, "game.boost_per_km", gui.boost_per_km);	Param(c,w, "game.boost_add_sec", gui.boost_add_sec);

	Param(c,w, "game.collis_cars", gui.collis_cars);	Param(c,w, "game.collis_veget", gui.collis_veget);
	Param(c,w, "game.collis_roadw", gui.collis_roadw);	Param(c,w, "game.dyn_objects", gui.dyn_objects);

	Param(c,w, "game.trk_reverse", gui.trackreverse);	Param(c,w, "game.sim_mode", gui.sim_mode);
	Param(c,w, "game.local_players", gui.local_players); Param(c,w, "game.num_laps", gui.num_laps);
	Param(c,w, "game.start_order", gui.start_order);	Param(c,w, "game.split_vertically", split_vertically);

	//  graphs
	Param(c,w, "graphs.tc_r", tc_r);			Param(c,w, "graphs.tc_xr", tc_xr);
	Param(c,w, "graphs.te_yf", te_yf);			Param(c,w, "graphs.te_xf_pow", te_xf_pow);
	Param(c,w, "graphs.te_xfx", te_xfx);		Param(c,w, "graphs.te_xfy", te_xfy);
	Param(c,w, "graphs.te_reference", te_reference);	Param(c,w, "graphs.te_common", te_common);


	//  hud
	Param(c,w, "hud_show.mph", show_mph);
	Param(c,w, "hud_show.gauges", show_gauges);			Param(c,w, "hud_show.show_digits", show_digits);
	Param(c,w, "hud_show.trackmap", trackmap);			Param(c,w, "hud_show.times", show_times);
	Param(c,w, "hud_show.caminfo", show_cam);			Param(c,w, "hud_show.cam_tilt", cam_tilt);
	Param(c,w, "hud_show.car_dbgtxt", car_dbgtxt);		Param(c,w, "hud_show.show_cardbg", car_dbgbars);
	Param(c,w, "hud_show.car_dbgsurf", car_dbgsurf);	Param(c,w, "hud_show.car_tirevis", car_tirevis);
	Param(c,w, "hud_show.car_dbgtxtclr", car_dbgtxtclr); Param(c,w, "hud_show.car_dbgtxtcnt", car_dbgtxtcnt);
	Param(c,w, "hud_show.check_arrow", check_arrow);	Param(c,w, "hud_show.check_beam", check_beam);
	Param(c,w, "hud_show.opponents", show_opponents);	Param(c,w, "hud_show.opplist_sort", opplist_sort);
	Param(c,w, "hud_show.graphs", show_graphs);			Param(c,w, "hud_show.graphs_type", (int&)graphs_type);
	//  gui
	Param(c,w, "gui.cars_view", cars_view);			Param(c,w, "gui.cars_sort", cars_sort);
	Param(c,w, "gui.car_ed_tab", car_ed_tab);		Param(c,w, "gui.tweak_tab", tweak_tab);
	Param(c,w, "gui.champ_tab", champ_type);
	Param(c,w, "gui.chall_tab", chall_type);		Param(c,w, "gui.champ_info", champ_info);
	//  hud size
	Param(c,w, "hud_size.gauges", size_gauges);			Param(c,w, "hud_size.arrow", size_arrow);
	Param(c,w, "hud_size.minimap", size_minimap);		Param(c,w, "hud_size.minipos", size_minipos);
	Param(c,w, "hud_size.mini_zoom", zoom_minimap);		Param(c,w, "hud_size.mini_zoomed", mini_zoomed);
	Param(c,w, "hud_size.mini_rotated", mini_rotated);	Param(c,w, "hud_size.mini_terrain", mini_terrain);
	Param(c,w, "hud_size.mini_border", mini_border);
	Param(c,w, "hud_size.gauges_type", gauges_type);	//Param(c,w, "hud_size.gauges_layout", gauges_layout);
	//  cam
	Param(c,w, "hud_size.cam_loop_chng", cam_loop_chng); Param(c,w, "hud_size.cam_in_loop", cam_in_loop);
	Param(c,w, "hud_size.fov", fov_min);				Param(c,w, "hud_size.fov_boost", fov_boost);
	Param(c,w, "hud_size.fov_smooth", fov_smooth);
	Param(c,w, "hud_size.cam_bounce", cam_bounce);		Param(c,w, "hud_size.cam_bnc_mul", cam_bnc_mul);
	//  pacenotes
	Param(c,w, "pacenotes.show", pace_show);		Param(c,w, "pacenotes.dist", pace_dist);
	Param(c,w, "pacenotes.size", pace_size);		Param(c,w, "pacenotes.near", pace_near);
	Param(c,w, "pacenotes.next", pace_next);		Param(c,w, "pacenotes.alpha", pace_alpha);
	Param(c,w, "pacenotes.trail", trail_show);


	//  graphics
	Param(c,w, "graph_par.particles", particles);			Param(c,w, "graph_par.trails", trails);
	Param(c,w, "graph_par.particles_len", particles_len);	Param(c,w, "graph_par.trail_len", trails_len);

	Param(c,w, "graph_reflect.skip_frames", refl_skip);		Param(c,w, "graph_reflect.faces_once", refl_faces);
	Param(c,w, "graph_reflect.map_size", refl_size);		Param(c,w, "graph_reflect.dist", refl_dist);
	Param(c,w, "graph_reflect.mode", refl_mode);

	//  misc
	Param(c,w, "misc.version", version);
	Param(c,w, "misc.bulletDebug", bltDebug);		Param(c,w, "misc.bulletLines", bltLines);
	Param(c,w, "misc.profilerTxt", profilerTxt);	Param(c,w, "misc.bulletProfilerTxt", bltProfilerTxt);
	Param(c,w, "misc.dev_keys", dev_keys);			Param(c,w, "misc.dev_no_prvs", dev_no_prvs);

	Param(c,w, "misc.show_welcome", show_welcome);	Param(c,w, "misc.loadingback", loadingbackground);

	//  network
	Param(c,w, "network.master_server_address", master_server_address);	Param(c,w, "network.nickname", nickname);
	Param(c,w, "network.master_server_port", master_server_port);		Param(c,w, "network.local_port", local_port);
	Param(c,w, "network.connect_address", connect_address);				Param(c,w, "network.game_name", netGameName);
	Param(c,w, "network.connect_port", connect_port);

	//  replay
	Param(c,w, "replay.rec", rpl_rec);				Param(c,w, "replay.ghost", rpl_ghost);
	Param(c,w, "replay.bestonly", rpl_bestonly);	Param(c,w, "replay.trackghost", rpl_trackghost);
	Param(c,w, "replay.listview", rpl_listview);	Param(c,w, "replay.listghosts", rpl_listghosts);
	Param(c,w, "replay.ghostpar", rpl_ghostpar);	Param(c,w, "replay.ghostother", rpl_ghostother);
	Param(c,w, "replay.num_views", rpl_numViews);	Param(c,w, "replay.ghostrewind", rpl_ghostrewind);
	Param(c,w, "replay.ghoHideDist", ghoHideDist);	Param(c,w, "replay.ghoHideDistTrk", ghoHideDistTrk);

	//  sim
	Param(c,w, "sim.game_freq", game_fq);			Param(c,w, "sim.multi_thr", multi_thr);
	Param(c,w, "sim.bullet_freq", blt_fq);			Param(c,w, "sim.bullet_iter", blt_iter);
	Param(c,w, "sim.dynamics_iter", dyn_iter);		Param(c,w, "sim.thread_sleep", thread_sleep);
	Param(c,w, "sim.perf_speed", perf_speed);		Param(c,w, "sim.gui_sleep", gui_sleep);


	//  sound
	Param(c,w, "sound.device", snd_device);			Param(c,w, "sound.reverb", snd_reverb);
	Param(c,w, "sound.volume", vol_master);			Param(c,w, "sound.vol_engine", vol_engine);
	Param(c,w, "sound.vol_tires", vol_tires);		Param(c,w, "sound.vol_env", vol_env);
	Param(c,w, "sound.vol_susp", vol_susp);
	Param(c,w, "sound.vol_fl_splash", vol_fl_splash);	Param(c,w, "sound.vol_fl_cont", vol_fl_cont);
	Param(c,w, "sound.vol_car_crash", vol_car_crash);	Param(c,w, "sound.vol_car_scrap", vol_car_scrap);
	Param(c,w, "sound.hud_vol", vol_hud);
	Param(c,w, "sound.hud_chk", snd_chk);			Param(c,w, "sound.hud_chk_wrong", snd_chkwr);

	//  effects
	Param(c,w, "video_eff.all_effects", all_effects);
	Param(c,w, "video_eff.bloom", bloom);				Param(c,w, "video_eff.bloomintensity", bloom_int);
	Param(c,w, "video_eff.bloomorig", bloom_orig);
	Param(c,w, "video_eff.motionblur", blur);			Param(c,w, "video_eff.motionblurintensity", blur_int);
	Param(c,w, "video_eff.ssao", ssao);					Param(c,w, "video_eff.softparticles", softparticles);
	Param(c,w, "video_eff.godrays", godrays);
	Param(c,w, "video_eff.dof", dof);
	Param(c,w, "video_eff.dof_focus", dof_focus);		Param(c,w, "video_eff.dof_far", dof_far);
	Param(c,w, "video_eff.boost_fov", boost_fov);
	//  effects hdr
	Param(c,w, "video_eff.hdr", hdr);					Param(c,w, "video_eff.hdr_p1", hdrParam1);
	Param(c,w, "video_eff.hdr_p2", hdrParam2);			Param(c,w, "video_eff.hdr_p3", hdrParam3);
	Param(c,w, "video_eff.hdr_bloomint", hdrBloomint);	Param(c,w, "video_eff.hdr_bloomorig", hdrBloomorig);
	Param(c,w, "video_eff.hdr_adaptationScale", hdrAdaptationScale);
	Param(c,w, "video_eff.hdr_vignettingRadius", vignRadius);  Param(c,w, "video_eff.hdr_vignettingDarkness", vignDarkness);
}

SETTINGS::SETTINGS()   ///  Defaults
	:version(100)  // old
	//  hud
	,show_gauges(1), trackmap(1)
	,show_cam(1), show_times(0), show_digits(1)
	,show_opponents(1), opplist_sort(true), cam_tilt(1)
	,car_dbgtxt(0), car_dbgbars(0), car_dbgsurf(0), show_graphs(0)
	,car_dbgtxtclr(0), car_dbgtxtcnt(0), car_tirevis(0), sounds_info(0)

	,size_gauges(0.18), size_minimap(0.2), size_minipos(0.1), zoom_minimap(1.0)
	,mini_zoomed(0), mini_rotated(1), mini_terrain(0), mini_border(1)
	,check_arrow(0),size_arrow(0.2), check_beam(1)
	,gauges_type(1),gauges_layout(1), graphs_type(Gh_Fps)
	//  cam
	,cam_loop_chng(1), cam_in_loop(1)
	,fov_min(90.f), fov_boost(5.f), fov_smooth(5.f)
	,cam_bounce(1), cam_bnc_mul(1.f)
	//  pace
	,pace_show(1), pace_next(4)
	,pace_dist(200.f), pace_size(1.f), pace_near(1.f), pace_alpha(1.f)
	,trail_show(1)

	//  gui
	,cars_view(0), cars_sort(1), cars_sortup(1)
	,champ_type(0),chall_type(0), champ_info(1)
	,car_ed_tab(0),tweak_tab(0)
	//  graphics
	, bFog(0)
	,refl_skip(200), refl_faces(1), refl_size(0), refl_dist(500.f), refl_mode(1)
	,particles(true), trails(true), particles_len(1.f), trails_len(1.f), boost_fov(true)

	//  car
	,autoshift(1), autorear(1), rear_inv(1), show_mph(0)
	//  misc
	,rpl_rec(0)
	,dev_keys(0), dev_no_prvs(0)
	,split_vertically(true)
	//  misc
	,bltDebug(0), bltLines(1),  bltProfilerTxt(0), profilerTxt(0)
	,loadingbackground(true), show_welcome(true)
	//  network
	,nickname("Player"), netGameName("Default Game")
	,master_server_address("")
	,master_server_port(protocol::DEFAULT_PORT)
	,local_port(protocol::DEFAULT_PORT)
	,connect_address("localhost")
	,connect_port(protocol::DEFAULT_PORT)
	//  replay
	,rpl_ghost(1), rpl_bestonly(1), rpl_trackghost(1)
	,rpl_ghostpar(0), rpl_ghostrewind(1), rpl_ghostother(1)
	,rpl_listview(0), rpl_listghosts(0), rpl_numViews(4)
	,ghoHideDist(5.f), ghoHideDistTrk(5.f)
	//  sim
	,game_fq(82.f), blt_fq(160.f), blt_iter(24), dyn_iter(30)
	,multi_thr(0), thread_sleep(5), gui_sleep(1), perf_speed(100000)
	//  graphs
	,tc_r(6000.f), tc_xr(1.f)
	,te_yf(8000.f), te_xf_pow(1.f), te_xfy(160.f), te_xfx(12.f)
	,te_reference(0), te_common(1)

	//  sound
	,vol_master(1.f), vol_engine(0.6f), vol_tires(1.f), vol_env(1.f), vol_susp(1.f)
	,vol_fl_splash(1.f),vol_fl_cont(1.f), vol_car_crash(1.f),vol_car_scrap(1.f)
	,vol_hud(1.f), snd_chk(0), snd_chkwr(1)
	,snd_device(""), snd_reverb(1)
	//  video eff
	,all_effects(false), godrays(false), filmgrain(false)
	,bloom(false), bloom_int(0.13), bloom_orig(0.9), hdr(false)
	,blur(false), blur_int(0.1)
	,dof_focus(100), dof_far(1000), dof(false)
	,ssao(false), softparticles(false)
	//  hdr
	,hdrParam1(0.62), hdrParam2(0.10), hdrParam3(0.79)
	,hdrBloomint(0.81), hdrBloomorig(0.34), hdrAdaptationScale(0.51)
	,vignRadius(2.85), vignDarkness(0.34)
	//  not in gui
	,net_local_plr(-1)
{
	//  car setup  (update in game-default.cfg)
	abs[0] = 0;  abs[1] = 0;
	tcs[0] = 0;  tcs[1] = 0;
	sss_effect[0] = 0.f;  sss_effect[1] = 0.85f;
	sss_velfactor[0] = 1.f;  sss_velfactor[1] = 1.f;
	steer_range[0] = 1.0;  steer_range[1] = 0.7;
	steer_sim[0] = 0.51;  steer_sim[1] = 0.81;

	cam_view.resize(4);
	for (int i=0; i < 4; ++i)
		cam_view[i] = 9;
}


SETTINGS::GameSet::GameSet()
	:track("Isl6-Flooded"), track_user(false)
	,trees(1.f)  // common
	,trackreverse(false)
	//  game setup
	,local_players(1), num_laps(2)
	,sim_mode("easy")
	,collis_veget(true), collis_cars(false), collis_roadw(false), dyn_objects(true)
	,boost_type(2), flip_type(1), damage_type(1), rewind_type(1), damage_dec(0.f)
	,rpl_rec(1)
	//  champ
	,champ_num(-1), chall_num(-1)
	,champ_rev(false)
	,pre_time(2.f), start_order(0)
{
	car_hue.resize(6);  car_sat.resize(6);  car_val.resize(6);
	car_gloss.resize(6);  car_refl.resize(6);
	car.resize(4);

	BoostDefault();

	//  cars
	for (int i=0; i < 6; ++i)
	{	if (i < 4)  car[i] = "ES";
		car_hue[i] = 0.4f+0.2f*i;  car_sat[i] = 1.f;  car_val[i] = 1.f;
		car_gloss[i] = 0.5f;  car_refl[i] = 1.f;
	}
}

void SETTINGS::GameSet::BoostDefault()
{
	boost_power = 1.f;
	boost_max = 6.f;  boost_min = 2.f;
	boost_per_km = 1.f;  boost_add_sec = 0.1f;
}
