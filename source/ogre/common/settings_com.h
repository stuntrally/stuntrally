#pragma once
#include "../vdrift/configfile.h"

enum eShadowType  {  Sh_None=0, Sh_Depth, Sh_Soft  };


class SETcom
{
public:
//------------------------------------------

	//  misc
	std::string language;  // "" autodetect lang

	//  startup, other
	bool autostart =0, escquit =0, startInMain =1;
	bool ogre_dialog =0, mouse_capture =1, screen_png =0;

	//  screen
	int windowx =800, windowy =600, fsaa =0;
	bool fullscreen =0, vsync =0;
	std::string buffer, rendersystem;
	//  limit
	bool limit_fps =0;  float limit_fps_val = 60.f;  int limit_sleep = -1;
	
	//  hud
	bool show_fps =0;


	//  graphics  ----
	int anisotropy,  tex_filt, tex_size;  // textures
	int ter_mtr, ter_tripl;  // terrain
	float view_distance, terdetail,  terdist, road_dist;  // detail
	
	bool water_reflect, water_refract;  int water_rttsize;  // water
	float shadow_dist;  int shadow_size, lightmap_size, shadow_count,  shadow_type; //eShadowType

	bool use_impostors =1, impostors_only =0;  //  veget
	float grass =1.f, trees_dist =1.f, grass_dist =1.f;  // trees in gui.

	//  graphics other
	int preset;  // last set, info only
	bool horizon;
	std::string shader_mode;


	//  track
	int tracks_view =0, tracks_sort =2;
	bool tracks_sortup =1, tracks_filter =0;

	#define COL_VIS 19
	#define COL_FIL 14
	
	bool col_vis[2][COL_VIS];  // visible columns for track views
	int  col_fil[2][COL_FIL];  // filtering range for columns 0min-1max

	const static bool colVisDef[2][COL_VIS];
	const static char colFilDef[2][COL_FIL];
	

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
