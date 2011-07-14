#ifndef _Settings_h_
#define _Settings_h_

#include "../vdrift/configfile.h"


#define SET_VER  1200  // 1.2


class SETTINGS
{
public:
///  params
//------------------------------------------
	int version;  // file version

	//  track
	std::string track;  bool track_user;

	//  show
	bool show_fps, trackmap;  int num_mini;
	float size_minimap;

	//  graphics
	int anisotropy, shaders;
	float view_distance, terdetail,terdist, road_dist;
	float shadow_dist;  int shadow_size, shadow_count, shadow_type;
	float trees, grass, trees_dist, grass_dist;

	//  startup
	bool autostart, escquit, ogre_dialog;
	std::string language;

	//  settings
	bool bFog, bTrees;
	int ter_skip, mini_skip;  float road_sphr;
	float cam_speed, cam_inert, cam_x,cam_y,cam_z, cam_dx,cam_dy,cam_dz;
	
	// video
	int windowx, windowy;
	bool fullscreen;
	int fsaa;
	bool vsync;
	std::string buffer;
	std::string rendersystem;
	
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
