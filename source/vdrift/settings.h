#ifndef _Settings_h_
#define _Settings_h_

#include "configfile.h"
using namespace std;

class SETTINGS
{
public:
///  params
//------------------------------------------
	//  car, track
	std::string car,car_ai, track;  bool track_user;

	//  show
	bool show_fps, show_gauges, trackmap, racingline,
		show_cam, show_times, car_dbgbars, car_dbgtxt, ogre_dialog;
	float size_gauges, size_minimap;

	//  graphics
	int anisotropy, shaders;  bool bFog;
	float view_distance, terdetail,terdist, road_dist;
	float shadow_dist;  int shadow_size, shadow_count, shadow_type;
	int refl_skip, refl_faces, refl_size;  float refl_dist;
	bool particles, trails;  float trees, grass, trees_dist, grass_dist;
	float particles_len, trails_len;

	//  joy input
	std::string joytype;  bool joy200;  bool joystick_calibrated;
	float speed_sensitivity;  bool hgateshifter;
	float button_ramp;
	std::string ff_device;  float ff_gain;  bool ff_invert;

	//  car
	bool abs, tcs, autoclutch, autoshift, autorear, show_mph;
	float car_hue, car_sat, car_val;

	//  game
	bool recordreplay;  int selected_replay;
	bool trackreverse;	int number_of_laps;
	float ai_difficulty;

	//  other
	std::string skin;	float vol_master, vol_engine, vol_tires, vol_env;
	bool autostart, escquit;	bool bltDebug, bltLines;
	
	// loading
	bool loadingbackground;
	
	//  sim freq (1/interval timestep)
	float game_fq, blt_fq;  int blt_iter;
	int mult_thr;
	bool veget_collis;
	
	//  compositor
	bool bloom, hdr, motionblur;
	float bloomintensity, bloomorig, motionblurintensity;
	//  video
	int windowx, windowy;
	bool fullscreen, vsync;
	int fsaa;
	std::string buffer;
	std::string rendersystem;
	
	//  replay
	bool rpl_rec, rpl_play;
	
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
