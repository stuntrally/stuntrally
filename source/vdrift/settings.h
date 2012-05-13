#ifndef _Settings_h_
#define _Settings_h_

#include "configfile.h"


#define SET_VER  1650  // 1.6+


const static int graph_types = 5;  // total graph types count

class SETTINGS
{
public:
//------------------------------------------
	int version;  // file version

	//  show
	bool show_fps, show_gauges, check_arrow, trackmap,
		mini_zoomed, mini_rotated, mini_terrain,
		show_cam, show_times, show_digits, show_opponents, cam_tilt,
		car_dbgbars, car_dbgtxt, ogre_dialog, show_graphs;
	float size_gauges, size_minimap, size_arrow, zoom_minimap;
	int tracks_view, tracks_sort;  bool tracks_sortup;
	int gauges_type, graphs_type;

	//  graphics
	int anisotropy, tex_size, ter_mtr;  bool bFog;
	float shaders;
	float view_distance, terdetail,terdist, road_dist;
	float shadow_dist;  int shadow_size, lightmap_size, shadow_count, shadow_type, shadow_filter;
	int refl_skip, refl_faces, refl_size;  float refl_dist;
	bool water_reflect, water_refract; int water_rttsize;
	int refl_mode; // 0 static, 1 single, 2 full, explanation: see CarReflection.h
	bool particles, trails;  float grass, trees_dist, grass_dist;
	bool use_imposters;
	float particles_len, trails_len;

	//  car
	bool abs, tcs, autoshift, autorear, rear_inv, show_mph;
	int cam_view[4];//

	//---------------  game config
	class GameSet
	{
	public:
		std::string track;  bool track_user, trackreverse;
		std::string car[4];
		float car_hue[4], car_sat[4], car_val[4];

		int local_players, num_laps;  // split
		//  game setup
		bool collis_veget, collis_cars, collis_roadw;
		int boost_type, flip_type;  float boost_power;
		float trees;
		
		bool rpl_rec;
		//  champ
		int champ_num;  // -1 none
		float pre_time;
	}  game,  // current game, changed only on new game start
		gui;  // gui only config
	//---------------
	
	//  misc
	bool split_vertically;  std::string language;
	bool isMain;  int inMenu;  // last menu id
	
	//  joystick ff
	std::string ff_device;	float ff_gain;	bool ff_invert;

	//  other
	float vol_master, vol_engine, vol_tires, vol_susp, vol_env,
		vol_fl_splash,vol_fl_cont, vol_car_crash,vol_car_scrap;
	bool autostart, escquit;
	bool bltDebug, bltLines, bltProfilerTxt, profilerTxt;
	bool loadingbackground;
	bool x11_capture_mouse, x11_hwmouse;
	bool opplist_sort;
	
	//  sim freq (1/interval timestep)
	float game_fq, blt_fq;  int blt_iter, dyn_iter, multi_thr, thread_sleep;
	
	//  compositor
	bool bloom, hdr, motionblur, all_effects;
	float bloomintensity, bloomorig, motionblurintensity;
	float depthOfFieldFocus, depthOfFieldFar;
	//  video
	int windowx, windowy, fsaa;
	bool fullscreen, vsync, ssaa, ssao, godrays, softparticles, dof, filmgrain;
	std::string buffer, rendersystem;
	
	//  replay
	bool rpl_rec, rpl_ghost, rpl_bestonly;
	bool rpl_alpha, rpl_ghostpar;  int rpl_listview, rpl_numViews;
	
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
