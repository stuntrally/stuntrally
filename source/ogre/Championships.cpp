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

	//float timeC = timeCur; //+ (coldStart ? -2 : 0);  // if not already driving at start, sub 2 sec (for gaining speed)
	float timeC = timeCur + (coldStart ? 0 : 1);  // if already driving at start, add 1 sec (times are for 1st lap)
	float time = timeC * carTimeMul;

	float place = (time - timeTrk)/timeTrk / carsXml.magic;
	// time = (place * magic * timeTrk + timeTrk) / carTimeMul;  //todo: show this in lists and hud..
	if (pPoints)
		*pPoints = std::max(0.f, (20.f - place) * 0.5f);

	int plc = place < 1.f ? 1 : std::min(30, (int)( floor(place +1.f) ));
	return plc;
}


///  Load  championships.xml, progress.xml (once)
//---------------------------------------------------------------------
void App::ChampsXmlLoad()
{
	times.LoadXml(PATHMANAGER::GameConfigDir() + "/times.xml");
	champs.LoadXml(PATHMANAGER::GameConfigDir() + "/championships.xml", &times);
	LogO(String("**** Loaded Championships: ") + toStr(champs.champs.size()));
	chall.LoadXml(PATHMANAGER::GameConfigDir() + "/challenges.xml", &times);
	LogO(String("**** Loaded Challenges: ") + toStr(chall.ch.size()));

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
	
	ProgressXml oldprog[2];
	oldprog[0].LoadXml(PATHMANAGER::UserConfigDir() + "/progress.xml");
	oldprog[1].LoadXml(PATHMANAGER::UserConfigDir() + "/progress_rev.xml");

	int chs = champs.champs.size();
	
	///  this is for old progress ver loading, from game with newer champs
	///  it resets progress only for champs which ver has changed (or track count)
	//  fill progress
	for (int pr=0; pr < 2; ++pr)
	{
		progress[pr].champs.clear();
		for (int c=0; c < chs; ++c)
		{
			const Champ& ch = champs.champs[c];
			
			//  find this champ in loaded progress
			bool found = false;  int p = 0;
			ProgressChamp* opc = 0;
			while (!found && p < oldprog[pr].champs.size())
			{
				opc = &oldprog[pr].champs[p];
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

			progress[pr].champs.push_back(pc);
	}	}
	ProgressSave(false);  //will be later in guiInit
	
	if (progress[0].champs.size() != champs.champs.size() ||
		progress[1].champs.size() != champs.champs.size())
		LogO("|| ERROR: champs and progress sizes differ !");
}

///  load championship track
//---------------------------------------------------------------------
void App::ChampNewGame()
{
	if (pSet->game.champ_num >= champs.champs.size())
		pSet->game.champ_num = -1;  //0 range

	int chId = pSet->game.champ_num, p = pSet->game.champ_rev ? 1 : 0;
	if (chId >= 0)
	{
		//  champ stage, current track
		ProgressChamp& pc = progress[p].champs[chId];
		const Champ& ch = champs.champs[chId];
		if (pc.curTrack >= ch.trks.size())  pc.curTrack = 0;  // restart
		const ChampTrack& trk = ch.trks[pc.curTrack];
		pSet->game.track = trk.name;
		pSet->game.track_user = 0;
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

void App::btnChampInfo(WP)
{
	pSet->champ_info = !pSet->champ_info;
	if (edChampInfo)  edChampInfo->setVisible(pSet->champ_info);
}

void App::chkGhampRev(WP wp)
{
	pSet->gui.champ_rev = !pSet->gui.champ_rev;
	ButtonPtr chk = wp->castType<MyGUI::Button>();
    chk->setStateSelected(pSet->gui.champ_rev);
	ChampsListUpdate();
}

void App::tabTutType(MyGUI::TabPtr wp, size_t id)
{
	pSet->tut_type = id;
	ChampsListUpdate();
}
void App::tabChampType(MyGUI::TabPtr wp, size_t id)
{
	pSet->champ_type = id;
	ChampsListUpdate();
}

///  Championships list  fill
//---------------------------------------------------------------------
void App::ChampsListUpdate()
{
	const char clrCh[7][8] = {
	// 0 tutorial  1 tutorial hard  // 2 normal  3 hard  4 very hard  // 5 scenery  6 test
		"#FFFFA0", "#E0E000",   "#A0F0FF", "#60C0FF", "#A0A0E0",   "#80FF80", "#909090"  };

	liChamps->removeAllItems();  int n=1;  size_t sel = ITEM_NONE;
	int p = pSet->gui.champ_rev ? 1 : 0;
	for (int i=0; i < champs.champs.size(); ++i,++n)
	{
		const Champ& ch = champs.champs[i];
		if (pSet->inMenu == MNU_Tutorial && ch.type == pSet->tut_type ||
			pSet->inMenu == MNU_Champ && ch.type - 2 == pSet->champ_type)
		{
			const ProgressChamp& pc = progress[p].champs[i];
			int ntrks = pc.trks.size();
			const String& clr = clrCh[ch.type];
			liChamps->addItem(clr+ toStr(n/10)+toStr(n%10), 0);  int l = liChamps->getItemCount()-1;
			liChamps->setSubItemNameAt(1,l, clr+ ch.name.c_str());
			liChamps->setSubItemNameAt(2,l, clrsDiff[ch.diff]+ TR("#{Diff"+toStr(ch.diff)+"}"));
			liChamps->setSubItemNameAt(3,l, clrsDiff[std::min(8,ntrks*2/3+1)]+ toStr(ntrks));
			liChamps->setSubItemNameAt(4,l, clrsDiff[std::min(8,int(ch.time/3.f/60.f))]+ GetTimeShort(ch.time));
			liChamps->setSubItemNameAt(5,l, clr+ fToStr(100.f * pc.curTrack / ntrks,0,3)+" %");
			liChamps->setSubItemNameAt(6,l, clr+ fToStr(pc.points,1,5));
			if (n-1 == pSet->gui.champ_num)  sel = l;
	}	}
	liChamps->setIndexSelected(sel);
}

///  Championships list  sel changed,  fill Stages list
//---------------------------------------------------------------------
void App::listChampChng(MyGUI::MultiList2* chlist, size_t id)
{
	if (id==ITEM_NONE || liChamps->getItemCount() == 0)  return;
	
	//  update champ stages
	liStages->removeAllItems();

	int pos = s2i(liChamps->getItemNameAt(id).substr(7))-1;
	if (pos < 0 || pos >= champs.champs.size())  {  LogO("Error champ sel > size.");  return;  }

	int n = 1, p = pSet->gui.champ_rev ? 1 : 0;
	const Champ& ch = champs.champs[pos];
	for (int i=0; i < ch.trks.size(); ++i,++n)
	{
		const ChampTrack& trk = ch.trks[i];
		String clr = GetSceneryColor(trk.name);
		liStages->addItem(clr+ toStr(n/10)+toStr(n%10), 0);  int l = liStages->getItemCount()-1;
		liStages->setSubItemNameAt(1,l, clr+ trk.name.c_str());

		int id = tracksXml.trkmap[trk.name];  // if (id > 0)
		const TrackInfo& ti = tracksXml.trks[id-1];

		float carMul = GetCarTimeMul(pSet->game.car[0], pSet->game.sim_mode);
		float time = (times.trks[trk.name] * trk.laps /*+ 2*/) / carMul;

		liStages->setSubItemNameAt(2,l, clr+ ti.scenery);
		liStages->setSubItemNameAt(3,l, clrsDiff[ti.diff]+ TR("#{Diff"+toStr(ti.diff)+"}"));

		liStages->setSubItemNameAt(4,l, "#80C0F0"+GetTimeShort(time));  //toStr(trk.laps)
		liStages->setSubItemNameAt(5,l, "#E0F0FF"+fToStr(progress[p].champs[pos].trks[i].points,1,3));
	}
	//  descr
	EditBox* ed = mGUI->findWidget<EditBox>("ChampDescr");
	if (ed)  ed->setCaption(ch.descr);

	//  update champ details (on stages tab)
	TextBox* txt;
	txt = (TextBox*)mWndGame->findWidget("valChDiff");
	if (txt)  txt->setCaption(TR("#{Diff"+toStr(ch.diff)+"}"));
	txt = (TextBox*)mWndGame->findWidget("valChTracks");
	if (txt)  txt->setCaption(toStr(ch.trks.size()));

	txt = (TextBox*)mWndGame->findWidget("valChDist");
	if (txt)  txt->setCaption(/*toStr(ch.length)*/"-");  // sum from find tracks..
	txt = (TextBox*)mWndGame->findWidget("valChTime");
	if (txt)  txt->setCaption(GetTimeString(ch.time));

	txt = (TextBox*)mWndGame->findWidget("valChProgress");
	if (txt)  txt->setCaption(fToStr(100.f * progress[p].champs[pos].curTrack / champs.champs[pos].trks.size(),1,5));
	txt = (TextBox*)mWndGame->findWidget("valChScore");
	if (txt)  txt->setCaption(fToStr(progress[p].champs[pos].points,1,5));
}

///  Stages list  sel changed,  update Track info
//---------------------------------------------------------------------
void App::listStageChng(MyGUI::MultiList2* li, size_t pos)
{
	if (valStageNum)  valStageNum->setVisible(pos!=ITEM_NONE);
	if (pos==ITEM_NONE || liChamps->getIndexSelected()==ITEM_NONE)  return;
	
	int nch = s2i(liChamps->getItemNameAt(liChamps->getIndexSelected()).substr(7))-1;
	if (nch >= champs.champs.size())  {  LogO("Error champ sel > size.");  return;  }

	const Champ& ch = champs.champs[nch];
	if (pos >= ch.trks.size())  {  LogO("Error stage sel > tracks.");  return;  }
	const string& trkName = ch.trks[pos].name;
	bool reversed = ch.trks[pos].reversed;

	if (valTrkNet)  valTrkNet->setCaption(TR("#{Track}: ") + trkName);
	ReadTrkStatsChamp(trkName, reversed);
	
	if (valStageNum)  valStageNum->setCaption(toStr(pos+1) +" / "+ toStr(ch.trks.size()));
}
//---------------------------------------------------------------------


///  champ start
void App::btnChampStart(WP)
{
	if (liChamps->getIndexSelected()==ITEM_NONE)  return;
	pSet->gui.champ_num = s2i(liChamps->getItemNameAt(liChamps->getIndexSelected()).substr(7))-1;

	//  if already finished, restart - will loose progress and scores ..
	int chId = pSet->gui.champ_num, p = pSet->game.champ_rev ? 1 : 0;
	LogO("|| Starting champ: "+toStr(chId)+(p?" rev":""));
	ProgressChamp& pc = progress[p].champs[chId];
	if (pc.curTrack == pc.trks.size())
	{
		LogO("|| Was at 100%, restarting progress.");
		pc.curTrack = 0;  //pc.score = 0.f;
	}
	// change btn caption to start/continue/restart ?..

	btnNewGame(0);
}

//  stage back
void App::btnChampStageBack(WP)
{
	mWndChampStage->setVisible(false);
	isFocGui = true;  // show back gui
	toggleGui(false);
}

///  stage start / end
//---------------------------------------------------------------------
void App::btnChampStageStart(WP)
{
	//  check if champ ended
	int chId = pSet->game.champ_num, p = pSet->game.champ_rev ? 1 : 0;
	ProgressChamp& pc = progress[p].champs[chId];
	const Champ& ch = champs.champs[chId];
	bool last = pc.curTrack == ch.trks.size();

	LogO("|| This was stage " + toStr(pc.curTrack) + "/" + toStr(ch.trks.size()) + " btn");
	if (last)
	{	//  show end window, todo: start particles..
		mWndChampStage->setVisible(false);
		// tutorial, tutorial hard, normal, hard, very hard, scenery, test
		const int ui[8] = {0,1,2,3,4,5,0,0};
		if (imgChampEnd)
			imgChampEnd->setImageCoord(IntCoord(ui[std::min(7, std::max(0, ch.type))]*128,0,128,256));
		mWndChampEnd->setVisible(true);
		return;
	}

	bool finished = pGame->timer.GetLastLap(0) > 0.f;  //?-
	if (finished)
	{
		LogO("|| Loading next stage.");
		mWndChampStage->setVisible(false);
		btnNewGame(0);
	}else
	{
		LogO("|| Starting stage.");
		mWndChampStage->setVisible(false);
		pGame->pause = false;
		pGame->timer.waiting = false;
	}
}

//  champ end
void App::btnChampEndClose(WP)
{
	mWndChampEnd->setVisible(false);
}

//  stage loaded
void App::ChampLoadEnd()
{
	if (pSet->game.champ_num >= 0)
	{
		ChampFillStageInfo(false);
		mWndChampStage->setVisible(true);
	}
}

void App::btnStageNext(WP)
{
	size_t id = liStages->getIndexSelected(), all = liStages->getItemCount();
	if (all == 0)  return;
	if (id == ITEM_NONE)  id = 0;
	else
		id = (id +1) % all;
	liStages->setIndexSelected(id);
	listStageChng(liStages, id);
}

void App::btnStagePrev(WP)
{
	size_t id = liStages->getIndexSelected(), all = liStages->getItemCount();
	if (all == 0)  return;
	if (id == ITEM_NONE)  id = 0;
	id = (id + all -1) % all;
	liStages->setIndexSelected(id);
	listStageChng(liStages, id);
}


///  save progress and update it on gui
void App::ProgressSave(bool upgGui)
{
	progress[0].SaveXml(PATHMANAGER::UserConfigDir() + "/progress.xml");
	progress[1].SaveXml(PATHMANAGER::UserConfigDir() + "/progress_rev.xml");
	if (!upgGui)
		return;
	ChampsListUpdate();
	listChampChng(liChamps, liChamps->getIndexSelected());
}


///  championship advance logic
//  caution: called from GAME, 2nd thread, no Ogre stuff here
//---------------------------------------------------------------------
void App::ChampionshipAdvance(float timeCur)
{
	int chId = pSet->game.champ_num, p = pSet->game.champ_rev ? 1 : 0;
	ProgressChamp& pc = progress[p].champs[chId];
	const Champ& ch = champs.champs[chId];
	const ChampTrack& trk = ch.trks[pc.curTrack];
	LogO("|| --- Champ end: " + ch.name);

	///  compute track  poins  --------------
	float timeTrk = times.trks[trk.name];
	if (timeTrk < 1.f)
	{	LogO("|| Error: Track has no best time !");  timeTrk = 10.f;	}
	timeTrk *= trk.laps;

	LogO("|| Track: " + trk.name);
	LogO("|| Your time: " + toStr(timeCur));
	LogO("|| Best time: " + toStr(timeTrk));

	float carMul = GetCarTimeMul(pSet->game.car[0], pSet->game.sim_mode);
	float points = 0.f;  int pos;

	#if 1  // test score +- sec diff
	for (int i=-2; i <= 4; ++i)
	{
		pos = GetRacePos(timeCur + i*2.f, timeTrk, carMul, true, &points);
		LogO("|| var, add time: "+toStr(i*2)+" sec, points: "+fToStr(points,2));
	}
	#endif
	pos = GetRacePos(timeCur, timeTrk, carMul, true, &points);

	float pass = (pSet->game.sim_mode == "normal") ? 5.f : 2.f;  ///..
	bool passed = points >= pass;  // didnt qualify, repeat current stage
	
	LogO("|| Points: " + fToStr(points,1) + "  pos: " + toStr(pos) + "  Passed: " + (passed ? "yes":"no"));
	pc.trks[pc.curTrack].points = points;

	//  --------------  advance  --------------
	bool last = pc.curTrack+1 == ch.trks.size();
	LogO("|| This was stage " + toStr(pc.curTrack+1) + "/" + toStr(ch.trks.size()));
	if (!last || (last && !passed))
	{
		//  show stage end [window]
		pGame->pause = true;
		pGame->timer.waiting = true;

		ChampFillStageInfo(true);  // cur track
		mWndChampStage->setVisible(true);
		
		if (passed)
			pc.curTrack++;  // next stage
		ProgressSave();
	}else
	{
		//  champ ended
		pGame->pause = true;
		pGame->timer.waiting = true;

		ChampFillStageInfo(true);  // cur track
		mWndChampStage->setVisible(true);

		///  compute champ :score:  --------------
		int ntrk = pc.trks.size();  float sum = 0.f;
		for (int t=0; t < ntrk; ++t)
			sum += pc.trks[t].points;

		pc.curTrack++;  // end = 100 %
		//float old = pc.score;  // .. save only higher ?
		pc.points = sum / ntrk;  // average from all tracks
		ProgressSave();

		LogO("|| Champ finished");
		LogO("|| Total points: " + toStr(points));
		
		//  upd champ end [window]
		String s = 
			TR("#{Championship}") + ": " + ch.name + "\n" +
			TR("#{TotalScore}") + ": " + fToStr(pc.points,1,5);
		edChampEnd->setCaption(s);
		//mWndChampEnd->setVisible(true);  // show after stage end
	}
}

void App::ChampFillStageInfo(bool finished)
{
	int chId = pSet->game.champ_num, p = pSet->game.champ_rev ? 1 : 0;
	ProgressChamp& pc = progress[p].champs[chId];
	const Champ& ch = champs.champs[chId];
	const ChampTrack& trk = ch.trks[pc.curTrack];

	String s;
	s = "#80FFE0"+ ch.name + "\n\n" +
		"#80FFC0"+ TR("#{Stage}") + ":  " + toStr(pc.curTrack+1) + " / " + toStr(ch.trks.size()) + "\n" +
		"#80FF80"+ TR("#{Track}") + ":  " + trk.name + "\n\n";

	if (!finished)  // track info at start
	{
		int id = tracksXml.trkmap[trk.name];
		if (id > 0)
		{
			const TrackInfo* ti = &tracksXml.trks[id-1];
			s += "#A0D0FF"+ TR("#{Difficulty}:  ") + clrsDiff[ti->diff] + TR("#{Diff"+toStr(ti->diff)+"}") + "\n";
			if (road)
			{	Real len = road->st.Length*0.001f * (pSet->show_mph ? 0.621371f : 1.f);
				s += "#A0D0FF"+ TR("#{Distance}:  ") + "#B0E0FF" + fToStr(len, 1,4) + (pSet->show_mph ? " mi" : " km") + "\n\n";
				s += "#A8B8C8"+ road->sTxtDesc;
		}	}
	}

	if (finished)
	{
		float points = pc.trks[pc.curTrack].points;
		float pass = (pSet->game.sim_mode == "normal") ? 5.f : 2.f;  ///..
		s += "#80C0FF"+TR("#{Finished}") + ".\n" +
			"#FFFF60"+TR("#{Score}") + ": " + fToStr(points,1,5) + "\n";
		s += "#80C0FF"+TR("#{ScoreNeeded}") + ": " + fToStr(pass,1,5) + "\n\n";
		
		bool passed = points >= pass;
		if (passed)
			s += "#00FF00"+TR("#{Passed}")+".\n"+TR("#{NextStage}.");
		else
			s += "#FF8000"+TR("#{DidntPass}")+".\n"+TR("#{RepeatStage}.");
	}
	edChampStage->setCaption(s);
	btChampStage->setCaption(finished ? TR("#{MessageBox_Continue}") : TR("#{ChampStart}"));
	
	//  preview image
	if (!finished)  // only at champ start
	{
		ResourceGroupManager& resMgr = ResourceGroupManager::getSingleton();
		Ogre::TextureManager& texMgr = Ogre::TextureManager::getSingleton();

		String path = PathListTrkPrv(0, trk.name), sGrp = "TrkPrv";
		resMgr.addResourceLocation(path, "FileSystem", sGrp);  // add for this track
		resMgr.unloadResourceGroup(sGrp);
		resMgr.initialiseResourceGroup(sGrp);

		if (imgChampStage)
		{	try
			{	s = "view.jpg";
				texMgr.load(path+s, sGrp, TEX_TYPE_2D, MIP_UNLIMITED);  // need to load it first
				imgChampStage->setImageTexture(s);  // just for dim, doesnt set texture
				imgChampStage->_setTextureName(path+s);
			} catch(...) {  }
		}
		resMgr.removeResourceLocation(path, sGrp);
	}
}
