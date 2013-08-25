#include "pch.h"
#include "common/Defines.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "OgreGame.h"
#include "../road/Road.h"
#include "common/MultiList2.h"

using namespace std;
using namespace Ogre;
using namespace MyGUI;


///  common  ----
///______________________________________________________________________________________________
///
///  load championship or challenge track
///______________________________________________________________________________________________
void App::Ch_NewGame()
{
	if (pSet->game.champ_num >= champs.all.size())
		pSet->game.champ_num = -1;  // range

	if (pSet->game.chall_num >= chall.all.size())
		pSet->game.chall_num = -1;

	int iChamp = pSet->game.champ_num;
	int iChall = pSet->game.chall_num;
	if (iChall >= 0)
	{
		///  challenge stage
		int p = 0; //pSet->game.chall_rev ? 1 : 0;
		ProgressChall& pc = progressL[p].chs[iChall];
		const Chall& chl = chall.all[iChall];
		if (pc.curTrack >= chl.trks.size())  pc.curTrack = 0;  // restart
		const ChallTrack& trk = chl.trks[pc.curTrack];
		pSet->game.track = trk.name;  pSet->game.track_user = 0;
		pSet->game.trackreverse = /*pSet->game.champ_rev ? !trk.reversed :*/ trk.reversed;
		pSet->game.num_laps = trk.laps;

		pSet->game.sim_mode = chl.sim_mode;
		pSet->game.boost_type = chl.boost_type;
		pSet->game.flip_type = chl.flip_type;
		pSet->game.boost_power = 1.f;
		pSet->game.damage_type = chl.damage_type;
		//rewind_type

		std::string track;  bool track_user, trackreverse;
		std::vector<std::string> car;  //[4]
		std::vector<float> car_hue, car_sat, car_val, car_gloss, car_refl;  //[6] also for ghosts

		bool collis_veget, collis_cars, collis_roadw, dyn_objects;
		//int boost_type, flip_type, damage_type, rewind_type;  float boost_power;
		
		//bool minimap, chk_arr, chk_beam, trk_ghost;  // deny using it if true
		// abs, tcs, autoshift, autorear
		
		pSet->game.trees = 1.5f;  //-
		pSet->game.collis_veget = true;
		pSet->game.dyn_objects = true;  //-

		pGame->pause = true;  // wait for stage wnd close
		pGame->timer.waiting = true;
	}
	else if (iChamp >= 0)
	{
		///  championship stage
		int p = pSet->game.champ_rev ? 1 : 0;
		ProgressChamp& pc = progress[p].chs[iChamp];
		const Champ& ch = champs.all[iChamp];
		if (pc.curTrack >= ch.trks.size())  pc.curTrack = 0;  // restart
		const ChampTrack& trk = ch.trks[pc.curTrack];
		pSet->game.track = trk.name;  pSet->game.track_user = 0;
		pSet->game.trackreverse = pSet->game.champ_rev ? !trk.reversed : trk.reversed;
		pSet->game.num_laps = trk.laps;

		pSet->game.boost_type = 1;  // from trk.?
		pSet->game.flip_type = 2;
		pSet->game.boost_power = 1.f;
		//pSet->game.trees = 1.f;  // >=1 ?
		//pSet->game.collis_veget = true;

		pGame->pause = true;  // wait for stage wnd close
		pGame->timer.waiting = true;
	}else
	{	pGame->pause = false;  // single race
		pGame->timer.waiting = false;
	}
}


///  car time mul
float App::GetCarTimeMul(const string& car, const string& sim_mode)
{
	//  car factor (time mul, for less power)
	//  times.xml has ES or S1 best lap time from normal sim
	float carMul = 1.f;
	int id = carsXml.carmap[car];
	if (id > 0)
	{	const CarInfo& ci = carsXml.cars[id-1];
		bool easy = sim_mode == "easy";
		carMul = easy ? ci.easy : ci.norm;
	}
	return carMul;
}

///  compute race position,  basing on car and track time
int App::GetRacePos(float timeCur, float timeTrk, float carTimeMul, bool coldStart, float* pPoints)
{
	//  magic factor: seconds needed for 1 second of track time for 1 race place difference
	//  eg. if track time is 3min = 180 sec, then 180*magic = 2.16 sec
	//  and this is the difference between car race positions (1 and 2, 2 and 3 etc)
	//  0.006 .. 0.0012				// par
	//float time1pl = magic * timeTrk;

	//  if already driving at start, add 1 sec (times are for 1st lap)
	float timeC = timeCur + (coldStart ? 0 : 1);
	float time = timeC * carTimeMul;

	float place = (time - timeTrk)/timeTrk / carsXml.magic;
	// time = (place * magic * timeTrk + timeTrk) / carTimeMul;  //todo: show this in lists and hud..
	if (pPoints)
		*pPoints = std::max(0.f, (20.f - place) * 0.5f);

	int plc = place < 1.f ? 1 : std::min(30, (int)( floor(place +1.f) ));
	return plc;
}


///______________________________________________________________________________________________
///  Load  championships.xml, progress.xml (once)
//-----------------------------------------------------------------------------------------------
void App::ChampsXmlLoad()
{
	times.LoadXml(PATHMANAGER::GameConfigDir() + "/times.xml");
	champs.LoadXml(PATHMANAGER::GameConfigDir() + "/championships.xml", &times);
	LogO(String("**** Loaded Championships: ") + toStr(champs.all.size()));
	chall.LoadXml(PATHMANAGER::GameConfigDir() + "/challenges.xml", &times);
	LogO(String("**** Loaded Challenges: ") + toStr(chall.all.size()));

	/* stats */
	float time = 0.f;  int trks = 0;
	for (std::map<std::string, float>::const_iterator it = times.trks.begin();
		it != times.trks.end(); ++it)
	{
		const string& trk = (*it).first;
		if (trk.substr(0,4) != "Test")
		{
			//if (!(trk[0] >= 'a' && trk[0] <= 'z'))  // sr only
				time += (*it).second;
			++trks;
	}	}
	LogO("Total tracks: "+ toStr(trks) + ", total time: "+ GetTimeShort(time/60.f)+" h:m");
	/**/
	

///  Champs  ---------------------------
	ProgressXml oldprog[2];
	oldprog[0].LoadXml(PATHMANAGER::UserConfigDir() + "/progress.xml");
	oldprog[1].LoadXml(PATHMANAGER::UserConfigDir() + "/progress_rev.xml");

	int chs = champs.all.size();
	
	///  this is for old progress ver loading, from game with newer champs
	///  it resets progress only for champs which ver has changed (or track count)
	//  fill progress
	for (int pr=0; pr < 2; ++pr)
	{
		progress[pr].chs.clear();
		for (int c=0; c < chs; ++c)
		{
			const Champ& ch = champs.all[c];
			
			//  find this champ in loaded progress
			bool found = false;  int p = 0;
			ProgressChamp* opc = 0;
			while (!found && p < oldprog[pr].chs.size())
			{
				opc = &oldprog[pr].chs[p];
				//  same name, ver and trks count
				if (opc->name == ch.name && opc->ver == ch.ver &&
					opc->trks.size() == ch.trks.size())
					found = true;
				++p;
			}
			if (!found)
				LogO("|| reset progress for champ: " + ch.name);
			
			ProgressChamp pc;
			pc.name = ch.name;  pc.ver = ch.ver;

			if (found)  //  found progress, points
			{	pc.curTrack = opc->curTrack;
				pc.points = opc->points;
			}

			//  fill tracks
			for (int t=0; t < ch.trks.size(); ++t)
			{
				ProgressTrack pt;
				if (found)  // found track points
					pt.points = opc->trks[t].points;
				pc.trks.push_back(pt);
			}

			progress[pr].chs.push_back(pc);
	}	}
	ProgressSave(false);  //will be later in guiInit
	
	if (progress[0].chs.size() != champs.all.size() ||
		progress[1].chs.size() != champs.all.size())
		LogO("|| ERROR: champs and progress sizes differ !");


///  Challenges  ---------------------------
	ProgressLXml oldpr[2];
	oldpr[0].LoadXml(PATHMANAGER::UserConfigDir() + "/progressL.xml");
	oldpr[1].LoadXml(PATHMANAGER::UserConfigDir() + "/progressL_rev.xml");

	chs = chall.all.size();
	
	///  this is for old progress ver loading, from game with newer challs
	///  it resets progress only for challs which ver has changed (or track count)
	//  fill progress
	for (int pr=0; pr < 2; ++pr)
	{
		progressL[pr].chs.clear();
		for (int c=0; c < chs; ++c)
		{
			const Chall& ch = chall.all[c];
			
			//  find this chall in loaded progress
			bool found = false;  int p = 0;
			ProgressChall* opc = 0;
			while (!found && p < oldpr[pr].chs.size())
			{
				opc = &oldpr[pr].chs[p];
				//  same name, ver and trks count
				if (opc->name == ch.name && opc->ver == ch.ver &&
					opc->trks.size() == ch.trks.size())
					found = true;
				++p;
			}
			if (!found)
				LogO("|| reset progressL for chall: " + ch.name);
			
			ProgressChall pc;
			pc.name = ch.name;  pc.ver = ch.ver;

			if (found)  //  found progress, points
			{	pc.curTrack = opc->curTrack;
				pc.points = opc->points;
			}

			//  fill tracks
			for (int t=0; t < ch.trks.size(); ++t)
			{
				ProgressTrackL pt;
				if (found)  // found track points
					pt.points = opc->trks[t].points;
				pc.trks.push_back(pt);
			}

			progressL[pr].chs.push_back(pc);
	}	}
	ProgressLSave(false);  //will be later in guiInit
	
	if (progressL[0].chs.size() != chall.all.size() ||
		progressL[1].chs.size() != chall.all.size())
		LogO("|| ERROR: challs and progressL sizes differ !");
}


void App::UpdChampTabVis()
{
	if (!liChamps || !tabChamp || !btStChamp)  return;
	static int oldMenu = pSet->inMenu;
	bool tutor = pSet->inMenu == MNU_Tutorial, champ = pSet->inMenu == MNU_Champ, chall = pSet->inMenu == MNU_Challenge;

	tabTut->setVisible(tutor);    imgTut->setVisible(tutor);    btStTut->setVisible(tutor);
	tabChamp->setVisible(champ);  imgChamp->setVisible(champ);  btStChamp->setVisible(champ);
	tabChall->setVisible(chall);  imgChall->setVisible(chall);  btStChall->setVisible(chall);

	liChamps->setVisible(!chall);  liChamps->setColour(tutor ? Colour(0.85,0.8,0.75) : Colour(0.7,0.78,0.85));
	liChalls->setVisible( chall);  liChalls->setColour(Colour(0.75,0.85,0.8));

	if (oldMenu != pSet->inMenu && (tutor || champ || chall))
	{	oldMenu = pSet->inMenu;
		if (chall)  ChallsListUpdate();
		else  ChampsListUpdate();
	}
	if (edChampInfo->getVisible())
		edChampInfo->setCaption(chall ? TR("#{ChallInfo}") : TR("#{ChampInfo}"));
}


void App::btnChampInfo(WP)
{
	pSet->champ_info = !pSet->champ_info;
	if (edChampInfo)  edChampInfo->setVisible(pSet->champ_info);
}
