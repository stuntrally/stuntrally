#ifndef _Settings_h_
#define _Settings_h_

#include "../vdrift/configfile.h"


#define SET_VER  1900  // 1.9


enum eShadowType  {  Sh_None=0, Sh_Simple, Sh_Depth, Sh_Soft  };

class SETTINGS
{
public:
///  params
//------------------------------------------
	int version;  // file version

	//  show
	bool show_fps, trackmap, brush_prv;  int num_mini;
	float size_minimap;
	int tracks_view, tracks_sort;  bool tracks_sortup;

	//  graphics
	int anisotropy, tex_size, ter_mtr;
	float shaders;
	float view_distance, terdetail,terdist, road_dist;
	float shadow_dist;  int shadow_size, lightmap_size, shadow_count, shadow_type, shadow_filter;
	float grass, trees_dist, grass_dist;
	bool water_reflect, water_refract; int water_rttsize;
	bool use_imposters, imposters_only;
	std::string shader_mode;

	class GameSet
	{
	public:
		//  track
		std::string track;  bool track_user;
		float trees;
	} gui;
	
	//  startup
	bool autostart, escquit, ogre_dialog, allow_save;
	bool capture_mouse, inputBar,camPos;
	//  misc
	std::string language;
	bool isMain;  int inMenu;  // last menu id

	//  settings
	bool bFog, bTrees, bWeather, autoBlendmap;
	int ter_skip, mini_skip;  float road_sphr;
	float cam_speed, cam_inert, cam_x,cam_y,cam_z, cam_dx,cam_dy,cam_dz;
	
	//  video
	int windowx, windowy;
	bool fullscreen;  int fsaa;  bool vsync;
	std::string buffer, rendersystem;
	
	//  ter generate
	float gen_freq, gen_persist, gen_pow, gen_scale, gen_ofsx, gen_ofsy;  int gen_oct;
	//  align ter
	float al_w_mul, al_w_add, al_smooth;
	//  tweak
	std::string tweak_mtr;
	
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
