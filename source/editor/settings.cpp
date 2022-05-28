#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "settings.h"
#include "../vdrift/tracksurface.h"


void SETTINGS::Load(std::string sfile)
{
	CONFIGFILE c;  c.Load(sfile);
	Serialize(false, c);
}
void SETTINGS::Save(std::string sfile)
{
	CONFIGFILE c;  c.Load(sfile);  version = SET_VER;
	Serialize(true, c);  c.Write();
}


void SETTINGS::Serialize(bool w, CONFIGFILE & c)
{
	c.bFltFull = false;
	
	SerializeCommon(w,c);
	
	Param(c,w, "game.in_main", bMain);		Param(c,w, "game.in_menu", inMenu);

	//  game common
	Param(c,w, "game.track", gui.track);				Param(c,w, "game.track_user", gui.track_user);
	Param(c,w, "graph_veget.trees", gui.trees);

	Param(c,w, "pacenotes.show", pace_show);		Param(c,w, "pacenotes.dist", pace_dist);
	Param(c,w, "pacenotes.size", pace_size);
	Param(c,w, "pacenotes.near", pace_near);		Param(c,w, "pacenotes.alpha", pace_alpha);
	Param(c,w, "pacenotes.trk_reverse", trk_reverse);


	Param(c,w, "hud_show.trackmap", trackmap);			Param(c,w, "hud_size.minimap", size_minimap);
	Param(c,w, "hud_show.mini_num", num_mini);			Param(c,w, "hud_show.brushpreview", brush_prv);
	
	Param(c,w, "misc.allow_save", allow_save);
	Param(c,w, "misc.inputBar", inputBar);			Param(c,w, "misc.camPos", camPos);
	Param(c,w, "misc.version", version);
	Param(c,w, "misc.check_load", check_load);		Param(c,w, "misc.check_save", check_save);
	
	Param(c,w, "set_cam.px",cam_x);  Param(c,w, "set_cam.py",cam_y);  Param(c,w, "set_cam.pz",cam_z);
	Param(c,w, "set_cam.dx",cam_dx); Param(c,w, "set_cam.dy",cam_dy); Param(c,w, "set_cam.dz",cam_dz);

	Param(c,w, "set.fog", bFog);					Param(c,w, "set.trees", bTrees);
	Param(c,w, "set.weather", bWeather);
	Param(c,w, "set.cam_speed", cam_speed);			Param(c,w, "set.cam_inert", cam_inert);
	Param(c,w, "set.ter_skip", ter_skip);			Param(c,w, "set.road_sphr", road_sphr);
	Param(c,w, "set.mini_skip", mini_skip);
		

	Param(c,w, "ter_gen.scale", gen_scale);
	Param(c,w, "ter_gen.ofsx", gen_ofsx);			Param(c,w, "ter_gen.ofsy", gen_ofsy);
	Param(c,w, "ter_gen.freq", gen_freq);			Param(c,w, "ter_gen.persist", gen_persist);
	Param(c,w, "ter_gen.pow", gen_pow);				Param(c,w, "ter_gen.oct", gen_oct);
	Param(c,w, "ter_gen.mul", gen_mul);				Param(c,w, "ter_gen.ofsh", gen_ofsh);
	Param(c,w, "ter_gen.roadsm", gen_roadsm);
	Param(c,w, "ter_gen.terMinA", gen_terMinA);		Param(c,w, "ter_gen.terMaxA",gen_terMaxA);
	Param(c,w, "ter_gen.terSmA", gen_terSmA);		Param(c,w, "ter_gen.terSmH",gen_terSmH);
	Param(c,w, "ter_gen.terMinH", gen_terMinH);		Param(c,w, "ter_gen.terMaxH",gen_terMaxH);	

	Param(c,w, "teralign.w_mul", al_w_mul);			Param(c,w, "teralign.smooth", al_smooth);
	Param(c,w, "teralign.w_add", al_w_add);

	Param(c,w, "tweak.mtr", tweak_mtr);
	Param(c,w, "pick.set_par", pick_setpar);		Param(c,w, "pick.objGroup", objGroup);
}


SETTINGS::SETTINGS()  ///  Defaults
	:version(100)  // old
	,bMain(1), inMenu(0)
	//  show
	,trackmap(1), size_minimap(0.5), num_mini(0), brush_prv(1)
	//  misc
	,allow_save(0)
	,check_load(0), check_save(1)
	,inputBar(0), camPos(0)
	//  settings
	,cam_x(0), cam_y(50),cam_z(-120),  cam_dx(0), cam_dy(0), cam_dz(1)
	,bFog(0), bTrees(0), bWeather(0)
	,cam_speed(1.f), cam_inert(1.f)
	,ter_skip(4), road_sphr(2.f), mini_skip(4)
	//  ter gen
	,gen_scale(20.f), gen_freq(0.73f), gen_oct(4), gen_persist(0.4f)
	,gen_pow(1.0f), gen_ofsx(0.f), gen_ofsy(0.f)
	,gen_mul(1.f), gen_ofsh(0.f), gen_roadsm(0.f)
	,gen_terMinA(0.f),gen_terMaxA(90.f),gen_terSmA(10.f)
	,gen_terMinH(-300.f),gen_terMaxH(300.f),gen_terSmH(10.f)
	//  align ter
	,al_w_mul(1.f), al_w_add(8.f), al_smooth(2.f)
	//  pacenotes
	,pace_show(3), pace_dist(1000.f), pace_size(1.f)
	,pace_near(1.f), pace_alpha(1.f)
	,trk_reverse(0), show_mph(0)
	//  tweak
	,tweak_mtr("")
	//  pick
	,pick_setpar(1), objGroup("rock")
{

	//  track common
	gui.track = "Isl6-Flooded";  gui.track_user = false;
	gui.trees = 1.f;
}
