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


///  get race position  (champs stage result)
float App::GetRacePosCh(const string& trk, const string& car, int carId, const string& sim_mode)
{
	if (pSet->game.track_user)
		return 1.f;  // user tracks arent in xml
	
	//  last car lap time,  or best if no lap yet
	TIMER& tim = pGame->timer;
	float last = tim.GetLastLap(carId), best = tim.GetBestLap(carId, pSet->game.trackreverse);
	float timeCur = last < 0.1f ? best : last;

	//  track time, score
	float timeTrk = times.trks[pSet->game.track] /** laps*/;
	if (timeTrk < 0.1f)
		return 1.f;  // track not in xml!

	float carMul = GetCarTimeMul(car, sim_mode);
	
	return GetRacePos(timeCur, timeTrk, carMul, true/**/);
}

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
float App::GetRacePos(float timeCur, float timeTrk, float carTimeMul, bool coldStart)
{
	//  magic factor: seconds needed for 1 second of track time for 1 race place difference
	//  eg. if track time is 3min = 180 sec, then 180*magic = 2.16 sec
	//  and this is the difference between car race positions (1 and 2, 2 and 3 etc)
	const float magic = 0.008f;  // 0.006 .. 0.0012
										//par
	float timeC = timeCur + (coldStart ? -2 : 0);  // if already not driving at start add 2 sec (for gaining speed)
	float time = timeC * carTimeMul;

	float place = (time - timeTrk)/timeTrk / magic;
	place = std::max(1.f, place + 1.f);
	//float t1pl = magic * timeTrk;
	return place;
}


///  Load  championships.xml, progress.xml (once)
//---------------------------------------------------------------------
void App::ChampsXmlLoad()
{
	times.LoadXml(PATHMANAGER::GameConfigDir() + "/times.xml");
	champs.LoadXml(PATHMANAGER::GameConfigDir() + "/championships.xml", times);
	LogO(String("**** Loaded Championships: ") + toStr(champs.champs.size()));

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

			if (found)  //  found progress, score
			{	pc.curTrack = opc->curTrack;
				pc.score = opc->score;
			}

			//  fill tracks
			for (int t=0; t < ch.trks.size(); ++t)
			{
				ProgressTrack pt;
				if (found)  // found track score
					pt.score = opc->trks[t].score;
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
			liChamps->setSubItemNameAt(6,l, clr+ fToStr(pc.score,1,5));
			if (n-1 == pSet->gui.champ_num)  sel = l;
	}	}
	liChamps->setIndexSelected(sel);
}

///  Championships list  sel changed,  fill Stages list
//---------------------------------------------------------------------
void App::listChampChng(MyGUI::MultiList2* chlist, size_t id)
{
	if (id==ITEM_NONE)  return;
	
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

		float time = (times.trks[trk.name] * trk.laps + 2) * (1.f - trk.factor);

		liStages->setSubItemNameAt(2,l, clr+ ti.scenery);
		liStages->setSubItemNameAt(3,l, clrsDiff[ti.diff]+ TR("#{Diff"+toStr(ti.diff)+"}"));

		liStages->setSubItemNameAt(4,l, "#80C0F0"+GetTimeShort(time));  //toStr(trk.laps)
		liStages->setSubItemNameAt(5,l, "#E0F0FF"+fToStr(progress[p].champs[pos].trks[i].score,1,5));
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
	if (txt)  txt->setCaption(fToStr(progress[p].champs[pos].score,1,5));
}

///  Stages list  sel changed,  update Track info
//---------------------------------------------------------------------
void App::listStageChng(MyGUI::MultiList2* li, size_t pos)
{
	if (valStageNum)  valStageNum->setVisible(pos!=ITEM_NONE);
	if (pos==ITEM_NONE)  return;
	
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

	///  compute track :score:  --------------
	float timeBest = times.trks[trk.name];
	if (timeBest < 1.f)
	{	LogO("|| Error: Track has no best time !");  timeBest = 10.f;	}
	timeBest *= trk.laps;
	timeBest += 2;  // first lap longer, time at start spent to gain car valocity
	float factor = ch.trks[pc.curTrack].factor;  // how close to best you need to be
	timeBest *= 1.0f + factor;

	LogO("|| Track: " + trk.name);
	LogO("|| Your time: " + toStr(timeCur));
	LogO("|| Best time: " + toStr(timeBest));

	const float decFactor = 1.5f;  // more means score will drop faster for longer times
	/**/  // test score +-10 sec diff
	for (int i=-3; i <= 3; ++i)
	{
		float score = (1.f + (timeBest-timeCur-i*3)/timeBest * decFactor) * 100.f;
		LogO("|| var, add time: "+toStr(i*3)+" sec, score: "+toStr(score));
	}/**/
	float score = (1.f + (timeBest-timeCur)/timeBest * decFactor) * 100.f;
	float pass = trk.passScore * (pSet->game.sim_mode == "normal" ? 1.f : 0.75);  // 100 normal, 75 easy
	bool passed = score >= pass;  // didnt qualify, repeat current stage
	LogO("|| Score: " + toStr(score) + "  Passed: " + (passed ? "yes":"no"));
	pc.trks[pc.curTrack].score = score;

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
			sum += pc.trks[t].score;

		pc.curTrack++;  // end = 100 %
		//float old = pc.score;  // .. save only higher ?
		pc.score = sum / ntrk;  // average from all tracks
		ProgressSave();

		LogO("|| Champ finished");
		LogO("|| Total score: " + toStr(score));
		
		//  upd champ end [window]
		String s = 
			TR("#{Championship}") + ": " + ch.name + "\n" +
			TR("#{TotalScore}") + ": " + fToStr(pc.score,1,5);
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
		float score = pc.trks[pc.curTrack].score;
		float pass = trk.passScore * (pSet->game.sim_mode == "normal" ? 1.f : 0.75);  // 100 normal, 75 easy
		s += "#80C0FF"+TR("#{Finished}") + ".\n" +
			"#FFFF60"+TR("#{Score}") + ": " + fToStr(score,1,5) + "\n";
		s += "#80C0FF"+TR("#{ScoreNeeded}") + ": " + fToStr(pass,1,5) + "\n\n";
		
		bool passed = score >= pass;
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
