#pragma once
#include "../vdrift/configfile.h"
#include "../ogre/common/settings_com.h"


#define SET_VER  2700  // 2.7


enum eGraphType  {
	Gh_Fps=0, Gh_CarAccelG,
	Gh_CamBounce, Gh_BulletHit,
	Gh_Sound,
	Gh_Checks,
	Gh_Suspension,
	Gh_TireSlips,
	Gh_TireEdit, Gh_Tires4Edit,
	Gh_Diffs,
	Gh_TorqueCurve, Gh_Engine,
	Gh_Clutch,
	Gh_ALL  };  // total count
const static std::string csGraphNames[Gh_ALL] = {
	"Fps graphics perf.", "Car Accel G's",
	"Camera bounce", "Car Hit chassis",
	"Sound info, sources",
	"Checkpoints",
	"Suspension pos & vel",
	"Tires slip| & slide-",
	"Tire Edit (Pacejka coeffs)*", "All Tires Pacejka vis and edit*",
	"Differentials",
	"Torque Curve, gears", "Engine torque & power",
	"Clutch, Rpm, Gear" };


enum EMenu
{	MN1_Main, MN1_Race,  // small main menus
	MN_Single, MN_Tutorial, MN_Champ, MN_Chall,  // game, same window
	MN_HowTo, MN_Replays, MN_Help, MN_Options  };  // other windows


class SETTINGS : public SETcom
{
public:
//------------------------------------------
	int version;  // file version =

	//  menu
	int iMenu;  // EMenu,
	int yMain =0, yRace =0;  // kbd up/dn cursors
	int difficulty =0;

	//  show
	bool show_gauges, show_digits,
		trackmap, mini_zoomed, mini_rotated, mini_terrain, mini_border,
		check_beam, check_arrow,
		show_times, show_opponents, opplist_sort,
		show_cam, cam_tilt,
		car_dbgbars, car_dbgtxt, car_dbgsurf,
		car_tirevis, show_graphs,
		ch_all =0;  // show all champs/challs

	float size_gauges, size_minimap, size_minipos, size_arrow, zoom_minimap;
	int gauges_type, gauges_layout;
	//  cam
	float fov_min, fov_boost, fov_smooth;
	bool cam_loop_chng;  int cam_in_loop;
	bool cam_bounce;  float cam_bnc_mul;
	//  pacenotes
	bool pace_show;  int pace_next;
	float pace_dist, pace_size, pace_near, pace_alpha;
	bool trail_show;

	eGraphType graphs_type;
	int car_dbgtxtclr, car_dbgtxtcnt;
	bool sounds_info;
	//  gui
	bool cars_sortup;  int cars_view, cars_sort;
	int champ_type, chall_type;
	bool champ_info;
	int car_ed_tab, tweak_tab;


	//  graphics
	bool bFog;
	int refl_skip, refl_faces, refl_size;  float refl_dist;
	int refl_mode;  // 0 static, 1 single, 2 full

	bool particles, trails;
	float particles_len, trails_len;
	bool boost_fov;


	//---------------  car setup
	bool abs[2], tcs[2],  // [2] = 0 gravel 1 asphalt
		autoshift, autorear, rear_inv, show_mph;
	float sss_effect[2], sss_velfactor[2];
	//  steering range multipliers
	float steer_range[2],  // gravel/asphalt
		steer_sim[2];  // simulation modes  0 easy 1 normal
	std::vector<int> cam_view;  //[4]

	//---------------  game config
	class GameSet
	{
	public:
		std::string track;  bool track_user;
		float trees;  // common

		bool trackreverse;
		std::vector<std::string> car;  //[4] local players
		std::vector<float> car_hue, car_sat, car_val, car_gloss, car_refl;  //[6] also for ghosts

		int local_players, num_laps;  // split
		//  game setup
		std::string sim_mode;
		bool collis_veget, collis_cars, collis_roadw, dyn_objects;
		int boost_type, flip_type, damage_type, rewind_type;
		float damage_dec;

		float boost_power, boost_max, boost_min, boost_per_km, boost_add_sec;
		void BoostDefault();

		bool rpl_rec;
		//  champ
		int champ_num, chall_num;  // -1 none
		bool champ_rev;

		float pre_time;  int start_order;

		GameSet();
	}  game,  // current game, changed only on new game start
		gui;  // gui only config
	//---------------


	//  misc
	bool dev_keys, dev_no_prvs;  // dev
	bool split_vertically;

	//  startup, other
	bool bltDebug, bltLines, bltProfilerTxt, profilerTxt;
	bool loadingbackground, show_welcome;

	//  sound
	float vol_master, vol_hud,
		vol_engine, vol_tires, vol_susp, vol_env,
		vol_fl_splash,vol_fl_cont, vol_car_crash,vol_car_scrap;
	bool snd_chk, snd_chkwr;  // play hud
	bool snd_reverb;  std::string snd_device;

	//  sim freq (1/interval timestep)
	float game_fq, blt_fq,  perf_speed;
	int blt_iter, dyn_iter,  multi_thr, thread_sleep, gui_sleep;

	//  graphs vis
	float tc_r, tc_xr;  // tire circles max
	float te_yf, te_xfx, te_xfy, te_xf_pow;  // tire edit max
	bool te_reference, te_common;


	//  effects
	bool all_effects, bloom, blur, hdr;
	float bloom_int, bloom_orig, blur_int;  // intensity
	float dof_focus, dof_far;
	bool softparticles, ssao, godrays, dof, filmgrain;
	//  hdr
	float hdrBloomint, hdrBloomorig;
	float hdrParam1, hdrParam2, hdrParam3;
	float hdrAdaptationScale;
	float vignRadius, vignDarkness;

	//  replay
	bool rpl_rec, rpl_ghost, rpl_bestonly;
	bool rpl_ghostother, rpl_trackghost;
	bool rpl_ghostpar, rpl_ghostrewind, rpl_listghosts;
	int rpl_listview, rpl_numViews;
	float ghoHideDist, ghoHideDistTrk;  // ghost hide dist, when close

	//  network
	std::string nickname, netGameName;
	std::string master_server_address, connect_address;
	int master_server_port, local_port, connect_port;

	// not in gui
	int net_local_plr;


//------------------------------------------
	SETTINGS();

	template <typename T>
	bool Param(CONFIGFILE & conf, bool write, std::string pname, T & value)
	{
		if (write)
		{	conf.SetParam(pname, value);
			return true;
		}else
			return conf.GetParam(pname, value);
	}
	void Serialize(bool write, CONFIGFILE & config);
	void Load(std::string sfile), Save(std::string sfile);
};
