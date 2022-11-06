#pragma once
#include "../vdrift/configfile.h"
#include "../ogre/common/settings_com.h"


#define SET_VER  2701  // 2.7


class SETTINGS : public SETcom
{
public:
///  params
//------------------------------------------
	int version;  // file version =
	
	//  menu
	bool bMain;  int inMenu;

	//  show
	bool trackmap, brush_prv;  int num_mini;
	float size_minimap;

	class GameSet
	{
	public:
		std::string track;  bool track_user;
		float trees;  // common
	} gui;
	
	//  misc
	bool allow_save, inputBar, camPos;
	bool check_load, check_save;

	//  settings
	bool bFog, bTrees, bWeather;
	int ter_skip, mini_skip;  float road_sphr;
	float cam_speed, cam_inert, cam_x,cam_y,cam_z, cam_dx,cam_dy,cam_dz;
	
	//  ter generate
	float gen_scale, gen_ofsx,gen_ofsy, gen_freq, gen_persist, gen_pow;  int gen_oct;
	float gen_mul, gen_ofsh, gen_roadsm;
	float gen_terMinA,gen_terMaxA,gen_terSmA, gen_terMinH,gen_terMaxH,gen_terSmH;

	//  align ter
	float al_w_mul, al_w_add, al_smooth;
	
	//  pacenotes
	int pace_show;  float pace_dist, pace_size, pace_near, pace_alpha;
	bool trk_reverse, show_mph;

	//  tweak
	std::string tweak_mtr, objGroup;
	//  pick
	bool pick_setpar;

	
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
