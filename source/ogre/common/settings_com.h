#pragma once
#include "../vdrift/configfile.h"


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
	
	bool show_fps;

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
