#ifndef _Settings_h_
#define _Settings_h_

#include "../vdrift/configfile.h"


#define SET_VER  1500  // 1.5


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
	bool use_imposters;

	class GameSet
	{
	public:
		//  track
		std::string track;  bool track_user;
		float trees;
	} gui;
	
	//  startup
	bool autostart, escquit, ogre_dialog, allow_save;
	bool x11_capture_mouse;
	//  misc
	std::string language;
	bool isMain;  int inMenu;  // last menu id

	//  settings
	bool bFog, bTrees;
	int ter_skip, mini_skip;  float road_sphr;
	float cam_speed, cam_inert, cam_x,cam_y,cam_z, cam_dx,cam_dy,cam_dz;
	
	//  video
	int windowx, windowy;
	bool fullscreen;  int fsaa;  bool vsync;
	std::string buffer, rendersystem;
	
	//  ter generate
	float gen_freq, gen_persist, gen_pow, gen_scale, gen_ofsx, gen_ofsy;  int gen_oct;
	
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
