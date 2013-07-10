#ifndef _Settings_h_
#define _Settings_h_

#include "configfile.h"


#define SET_VER  2010  // 2.1


enum eGraphType  {
	Gh_Fps=0,
	Gh_CarAccelG, Gh_BulletHit,
	Gh_Sound,
	Gh_TireSlips, Gh_Suspension,
	Gh_TireEdit,
	Gh_TorqueCurve, Gh_Engine,
	Gh_Clutch, Gh_Diffs, //todo: tire friction circles,
	Gh_ALL  };  // total count
const static std::string csGraphNames[Gh_ALL] = {
	"Fps graphics perf.",
	"Car Accel G's", "Car Hit chassis",
	"Sound volume & pan, wave",
	"Tires slip| & slide-", "Suspension pos & vel",
	"Tire Edit (Pacejka coeffs)*",
	"Torque Curve, gears", "Engine torque & power",
	"Clutch, Rpm, Gear", "Differentials"
	};

enum eShadowType  {  Sh_None=0, Sh_Simple, Sh_Depth, Sh_Soft  };

class SETTINGS
{
public:
//------------------------------------------
	int version;  // file version

	//  show
	bool show_fps, show_gauges, check_beam, check_arrow, trackmap,
		mini_zoomed, mini_rotated, mini_terrain, mini_border,
		show_cam, show_times, show_digits, show_opponents, cam_tilt,
		car_dbgbars, car_dbgtxt, car_dbgsurf, ogre_dialog, show_graphs;
	float size_gauges, size_minimap, size_arrow, zoom_minimap;
	int gauges_type;  eGraphType graphs_type;
	int car_dbgtxtclr, car_dbgtxtcnt;
	//  gui
	bool tracks_sortup, cars_sortup, champ_info;
	int tracks_view, tracks_sort, cars_sort, champ_type, car_ed_tab;

	//  graphics
	int anisotropy, tex_size, ter_mtr, ter_tripl;  bool bFog;
	float shaders;
	float view_distance, terdetail,terdist, road_dist;
	float shadow_dist;  int shadow_size, lightmap_size, shadow_count, shadow_filter;  eShadowType shadow_type;
	int refl_skip, refl_faces, refl_size;  float refl_dist;
	bool water_reflect, water_refract; int water_rttsize;
	int refl_mode; // 0 static, 1 single, 2 full, explanation: see CarReflection.h
	bool particles, trails;  float grass, trees_dist, grass_dist;
	bool use_imposters, imposters_only;
	float particles_len, trails_len;
	bool boost_fov;

	std::string shader_mode;

	//---------------  car setup
	bool abs[2], tcs[2],  // [2] = 0 gravel 1 asphalt
		autoshift, autorear, rear_inv, show_mph;
	float sss_effect[2], sss_velfactor[2];
	//  steering range multipliers
	float steer_range[2],  // gravel/asphalt
		steer_sim_easy,steer_sim_normal;  // simulation modes
	int cam_view[4];

	//---------------  game config
	class GameSet
	{
	public:
		std::string track;  bool track_user, trackreverse;
		std::string car[4];
		float car_hue[4], car_sat[4], car_val[4], car_gloss[4], car_refl[4];

		int local_players, num_laps;  // split
		//  game setup
		std::string sim_mode;
		bool collis_veget, collis_cars, collis_roadw, dyn_objects;
		int boost_type, flip_type, damage_type;  float boost_power;
		float trees;
		
		bool rpl_rec;
		//  champ
		int champ_num;  // -1 none
		bool champ_rev;
		float pre_time;
	}  game,  // current game, changed only on new game start
		gui;  // gui only config
	//---------------
	
	//  misc
	bool split_vertically;  std::string language;
	bool isMain, startInMain, dev_keys;  int inMenu;  // last menu id
	
	//  joystick ff
	std::string ff_device;	float ff_gain;	bool ff_invert;

	//  other
	float vol_master, vol_engine, vol_tires, vol_susp, vol_env,
		vol_fl_splash,vol_fl_cont, vol_car_crash,vol_car_scrap;
	bool autostart, escquit;
	bool bltDebug, bltLines, bltProfilerTxt, profilerTxt;
	bool loadingbackground;
	bool capture_mouse, x11_hwmouse;
	bool opplist_sort;
	
	//  sim freq (1/interval timestep)
	float game_fq, blt_fq,  perf_speed;
	int blt_iter, dyn_iter,  multi_thr, thread_sleep;
	
	//  compositor
	bool bloom, hdr, motionblur, all_effects;
	float bloomintensity, bloomorig, motionblurintensity;
	float depthOfFieldFocus, depthOfFieldFar;
	//  hdr
	float hdrbloomint, hdrbloomorig;
	float hdrParam1, hdrParam2, hdrParam3;
	float hdrAdaptationScale;
	float vignettingRadius, vignettingDarkness;
	//  video
	int windowx, windowy, fsaa;
	bool fullscreen, vsync, ssao, godrays, softparticles, dof, filmgrain;
	std::string buffer, rendersystem;
	
	//  replay
	bool rpl_rec, rpl_ghost, rpl_bestonly, rpl_ghostother;
	bool rpl_alpha, rpl_ghostpar, rpl_ghostrewind;  int rpl_listview, rpl_numViews;
	
	// network
	std::string nickname, netGameName;
	std::string master_server_address;
	int master_server_port, local_port;

	// not in gui
	bool boostFromExhaust;  int net_local_plr;
	bool renderNotActive;
	
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

#endif
