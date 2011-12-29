#ifndef _Settings_h_
#define _Settings_h_

#include "configfile.h"


#define SET_VER  1350  // 1.4


class SETTINGS
{
public:
///  params
//------------------------------------------
	int version;  // file version

	//  car, track
	std::string car[4], track;  bool track_user;

	//  show
	bool show_fps, show_gauges, check_arrow, trackmap,
		mini_zoomed, mini_rotated, mini_terrain,
		show_cam, show_times, show_digits,
		car_dbgbars, car_dbgtxt, ogre_dialog;
	float size_gauges, size_minimap, size_arrow, zoom_minimap;
	int tracks_view, tracks_sort;  bool tracks_sortup;

	//  graphics
	int anisotropy, tex_size, ter_mtr;  bool bFog;
	float shaders;
	float view_distance, terdetail,terdist, road_dist;
	float shadow_dist;  int shadow_size, lightmap_size, shadow_count, shadow_type;
	int refl_skip, refl_faces, refl_size;  float refl_dist;
	std::string refl_mode; // static, single, full [explanation: see CarReflection.h] 
	bool particles, trails;  float trees, grass, trees_dist, grass_dist;
	bool use_imposters;
	float particles_len, trails_len;

	//  car
	bool abs, tcs, autoshift, autorear, rear_inv, show_mph;
	float car_hue[4], car_sat[4], car_val[4];  int cam_view[4];

	//  game
	bool trackreverse;	int local_players, num_laps;
	bool split_vertically;  std::string language;

	//  other
	float vol_master, vol_engine, vol_tires, vol_env;
	bool autostart, escquit;
	bool bltDebug, bltLines, bltProfilerTxt;
	bool loadingbackground;
	
	//  sim freq (1/interval timestep)
	float game_fq, blt_fq;  int blt_iter;
	int mult_thr;
	bool veget_collis, car_collis;
	
	//  compositor
	bool bloom, hdr, motionblur, all_effects;
	float bloomintensity, bloomorig, motionblurintensity;
	//  video
	int windowx, windowy, fsaa;
	bool fullscreen, vsync, ssaa, ssao, godrays;
	std::string buffer, rendersystem;
	
	//  input
	bool x11_capture_mouse;
	
	//  replay
	bool rpl_rec, rpl_ghost, rpl_bestonly;
	bool rpl_alpha, rpl_ghostpar;  int rpl_listview;
	
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
