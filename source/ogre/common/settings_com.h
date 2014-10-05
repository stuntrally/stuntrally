#pragma once
#include "../vdrift/configfile.h"

enum eShadowType  {  Sh_None=0, Sh_Depth, Sh_Soft  };


class SETcom
{
public:
//------------------------------------------

	//  misc
	std::string language;
	bool isMain;  int inMenu;  // last menu id

	//  startup, other
	bool autostart, escquit, startInMain;
	bool ogre_dialog, mouse_capture, screen_png;

	//  screen
	int windowx, windowy, fsaa;
	bool fullscreen, vsync;
	std::string buffer, rendersystem;
	//  limit
	bool limit_fps;  float limit_fps_val;  int limit_sleep;
	
	//  hud
	bool show_fps;


	//  graphics  ----
	int anisotropy,  tex_filt, tex_size;  // textures
	int ter_mtr, ter_tripl;  // terrain
	float view_distance, terdetail,  terdist, road_dist;  // detail
	
	bool water_reflect, water_refract;  int water_rttsize;  // water
	float shadow_dist;  int shadow_size, lightmap_size, shadow_count,  shadow_type; //eShadowType

	bool use_imposters, imposters_only;  //  veget
	float grass, trees_dist, grass_dist;  // trees in gui.

	//  graphics other
	int preset;  // last set, info only
	bool horizon;
	std::string shader_mode;


	//  track
	std::vector<bool> col_vis[2];  //18 visible columns for track views
	std::vector<int>  col_fil[2];  //13 filtering range for columns 0min-1max
	

//------------------------------------------
	SETcom();

	template <typename T>
	bool Param(CONFIGFILE & conf, bool write, std::string pname, T & value)
	{
		if (write)
		{	conf.SetParam(pname, value);
			return true;
		}else
			return conf.GetParam(pname, value);
	}
	void SerializeCommon(bool write, CONFIGFILE & config);
};
