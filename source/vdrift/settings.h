#ifndef _Settings_h_
#define _Settings_h_

#include "configfile.h"

class SETTINGS
{
public:
///  params
//------------------------------------------
	//  car, track
	std::string car, track;  bool track_user;

	//  show
	bool show_fps, show_gauges, trackmap,
		show_cam, show_times, show_digits, car_dbgbars, car_dbgtxt, ogre_dialog;
	float size_gauges, size_minimap;

	//  graphics
	int anisotropy, shaders;  bool bFog;
	float view_distance, terdetail,terdist, road_dist;
	float shadow_dist;  int shadow_size, shadow_count, shadow_type;
	int refl_skip, refl_faces, refl_size;  float refl_dist;
	std::string refl_mode; // static, single, full [explanation: see CarReflection.h] 
	bool particles, trails;  float trees, grass, trees_dist, grass_dist;
	float particles_len, trails_len;

	//  car
	bool abs, tcs, autoclutch, autoshift, autorear, show_mph;
	float car_hue, car_sat, car_val;

	//  game
	bool trackreverse;	//int number_of_laps;  float ai_difficulty;
	int local_players;  bool split_vertically;
	std::string language;

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
	bool bloom, hdr, motionblur;
	float bloomintensity, bloomorig, motionblurintensity;
	//  video
	int windowx, windowy, fsaa;
	bool fullscreen, vsync;
	std::string buffer, rendersystem;
	
	//  input
	bool x11_capture_mouse;
	
	//  replay
	bool rpl_rec, rpl_ghost, rpl_bestonly;  int rpl_listview;
	
	// network
	std::string nickname;
	std::string master_server_address;
	int master_server_port;
	int local_port;

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
