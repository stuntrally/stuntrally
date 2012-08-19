#include "pch.h"
#include "common/Defines.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "OgreGame.h"
#include "common/MultiList2.h"

using namespace std;
using namespace Ogre;
using namespace MyGUI;


///  Load  championships.xml, progress.xml (once)
//---------------------------------------------------------------------
void App::ChampsXmlLoad()
{
	champs.LoadXml(PATHMANAGER::GetGameConfigDir() + "/championships.xml");
	LogO(String("**** Loaded Championships: ") + toStr(champs.champs.size()));
	
	ProgressXml oldprog;
	oldprog.LoadXml(PATHMANAGER::GetUserConfigDir() + "/progress.xml");

	int chs = champs.champs.size();
	
	///  this is for old progress ver loading, from game with newer champs
	///  it resets progress only for champs which ver has changed (or track count)
	//  fill progress
	progress.champs.clear();
	for (int c=0; c < chs; ++c)
	{
		const Champ& ch = champs.champs[c];
		
		//  find this champ in loaded progress
		bool found = false;  int p = 0;
		ProgressChamp* opc = 0;
		while (!found && p < oldprog.champs.size())
		{
			opc = &oldprog.champs[p];
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

		progress.champs.push_back(pc);
	}
	ProgressSave(false);  //will be later in guiInit
	
	if (progress.champs.size() != champs.champs.size())
		LogO("|| ERROR: champs and progress sizes differ !");
}

///  load championship track
//---------------------------------------------------------------------
void App::ChampNewGame()
{
	if (pSet->game.champ_num >= champs.champs.size())
		pSet->game.champ_num = -1;  //0 range

	int chId = pSet->game.champ_num;
	if (chId >= 0)
	{
		//  champ stage, current track
		ProgressChamp& pc = progress.champs[chId];
		const Champ& ch = champs.champs[chId];
		if (pc.curTrack >= ch.trks.size())  pc.curTrack = 0;  // restart
		const ChampTrack& trk = ch.trks[pc.curTrack];
		pSet->game.track = trk.name;
		pSet->game.track_user = 0;
		pSet->game.trackreverse = trk.reversed;
		pSet->game.num_laps = trk.laps;

		pSet->game.boost_type = 1;  // from trk.?
		pSet->game.flip_type = 1;
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

///  Championships list  fill
//---------------------------------------------------------------------
void App::ChampsListUpdate()
{
	//  0 normal  1 tutorial  2 test  3 scenery
	const char clrCh[4][8] = {"#A0F0FF", "#FFFFB0", "#FFA0A0", "#80FF80"};

	liChamps->removeAllItems();  int n=1;
	for (int i=0; i < champs.champs.size(); ++i,++n)
	{
		const Champ& ch = champs.champs[i];
		const ProgressChamp& pc = progress.champs[i];
		int ntrks = pc.trks.size();
		const String& clr = clrCh[ch.tutorial];
		liChamps->addItem(toStr(n/10)+toStr(n%10), 0);  int l = liChamps->getItemCount()-1;
		liChamps->setSubItemNameAt(1,l, clr+ ch.name.c_str());
		liChamps->setSubItemNameAt(2,l, clrsDiff[ch.diff]+ TR("#{Diff"+toStr(ch.diff)+"}"));
		liChamps->setSubItemNameAt(3,l, clrsDiff[std::min(7,ntrks*2/3+1)]+ toStr(ntrks));
		liChamps->setSubItemNameAt(4,l, clr+ fToStr(100.f * pc.curTrack / ntrks,0,3)+" %");
		liChamps->setSubItemNameAt(5,l, clr+ fToStr(pc.score,1,5));
		//length,time;
	}
	liChamps->setIndexSelected(pSet->gui.champ_num);  //range..
}

///  Championships list  sel changed,  fill Stages list
//---------------------------------------------------------------------
void App::listChampChng(MyGUI::MultiList2* chlist, size_t pos)
{
	if (pos==ITEM_NONE)  return;
	if (pos >= champs.champs.size())  {  LogO("Error champ sel > size.");  return;  }
	
	//  update champ stages
	liStages->removeAllItems();
	float allTime = 0.f;  int n=1;
	const Champ& ch = champs.champs[pos];
	for (int i=0; i < ch.trks.size(); ++i,++n)
	{
		const ChampTrack& trk = ch.trks[i];
		String clr = GetSceneryColor(trk.name);
		liStages->addItem(clr+ toStr(n/10)+toStr(n%10), 0);  int l = liStages->getItemCount()-1;
		liStages->setSubItemNameAt(1,l, clr+ trk.name.c_str());

		int id = tracksXml.trkmap[trk.name];  // if (id > 0)
		const TrackInfo& ti = tracksXml.trks[id-1];

		float time = (champs.trkTimes[trk.name] * trk.laps + 2) * (1.f - trk.factor);
		allTime += time;  // sum trk time, total champ time

		liStages->setSubItemNameAt(2,l, clr+ ti.scenery);
		liStages->setSubItemNameAt(3,l, clrsDiff[ti.diff]+ TR("#{Diff"+toStr(ti.diff)+"}"));

		liStages->setSubItemNameAt(4,l, "#80C0F0"+GetTimeString(time));  //toStr(trk.laps)
		liStages->setSubItemNameAt(5,l, "#E0F0FF"+fToStr(progress.champs[pos].trks[i].score,1,5));
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
	if (txt)  txt->setCaption(GetTimeString(allTime));

	txt = (TextBox*)mWndGame->findWidget("valChProgress");
	if (txt)  txt->setCaption(fToStr(100.f * progress.champs[pos].curTrack / champs.champs[pos].trks.size(),1,5));
	txt = (TextBox*)mWndGame->findWidget("valChScore");
	if (txt)  txt->setCaption(fToStr(progress.champs[pos].score,1,5));
}

///  Stages list  sel changed,  update Track info
//---------------------------------------------------------------------
void App::listStageChng(MyGUI::MultiList2* li, size_t pos)
{
	if (pos==ITEM_NONE)  return;
	int nch = liChamps->getIndexSelected();
	if (nch >= champs.champs.size())  {  LogO("Error champ sel > size.");  return;  }

	const Champ& ch = champs.champs[nch];
	if (pos >= ch.trks.size())  {  LogO("Error stagh sel > tracks.");  return;  }
	const string& trkName = ch.trks[pos].name;
	bool reversed = ch.trks[pos].reversed;

	if (valTrkNet)  valTrkNet->setCaption(TR("#{Track}: ") + trkName);
	ReadTrkStatsChamp(trkName, reversed);
}
//---------------------------------------------------------------------


///  champ start
void App::btnChampStart(WP)
{
	pSet->gui.champ_num = liChamps->getIndexSelected();
	LogO("|| Starting champ: "+toStr(pSet->gui.champ_num));

	//  if already finished, restart - will loose progress and scores ..
	int chId = pSet->gui.champ_num;
	ProgressChamp& pc = progress.champs[chId];
	if (pc.curTrack == pc.trks.size())
	{
		LogO("|| Was at 100%, restarting progress.");
		pc.curTrack = 0;
		//pc.score = 0.f;
	}

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
	int chId = pSet->game.champ_num;
	ProgressChamp& pc = progress.champs[chId];
	const Champ& ch = champs.champs[chId];
	bool last = pc.curTrack == ch.trks.size();
	LogO("|| This was stage " + toStr(pc.curTrack) + "/" + toStr(ch.trks.size()) + " btn");
	if (last)
	{	//  show end window
		mWndChampStage->setVisible(false);
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

///  save progress and update it on gui
void App::ProgressSave(bool upgGui)
{
	progress.SaveXml(PATHMANAGER::GetUserConfigDir() + "/progress.xml");
	if (!upgGui)
		return;
	ChampsListUpdate();
	listChampChng(liChamps, liChamps->getIndexSelected());
}


///  championship advance logic
//---------------------------------------------------------------------
void App::ChampionshipAdvance(float timeCur)
{
	int chId = pSet->game.champ_num;
	ProgressChamp& pc = progress.champs[chId];
	const Champ& ch = champs.champs[chId];
	const ChampTrack& trk = ch.trks[pc.curTrack];
	LogO("|| --- Champ end: " + ch.name);

	///  compute track :score:  --------------
	float timeBest = champs.trkTimes[trk.name];
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
	for (int i=-10; i <= 10; ++i)
	{
		float score = (1.f + (timeBest-timeCur-i)/timeBest * decFactor) * 100.f;
		LogO("|| var, add time: "+toStr(i)+" sec, score: "+toStr(score));
	}/**/
	float score = (1.f + (timeBest-timeCur)/timeBest * decFactor) * 100.f;
	bool passed = score >= trk.passScore;  // didnt qualify, repeat current stage
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
	int chId = pSet->game.champ_num;
	ProgressChamp& pc = progress.champs[chId];
	const Champ& ch = champs.champs[chId];
	const ChampTrack& trk = ch.trks[pc.curTrack];

	String s;
	s = "#80FFE0"+ ch.name + "\n\n" +
		"#80FFC0"+TR("#{Stage}") + ": " + toStr(pc.curTrack+1) + "/" + toStr(ch.trks.size()) + "\n" +
		"#80FF80"+TR("#{Track}") + ": " + trk.name + "\n\n";

	if (finished)
	{
		float score = pc.trks[pc.curTrack].score;
		s += "#80C0FF"+TR("#{Finished}") + ".\n" +
			"#FFFF60"+TR("#{Score}") + ": " + fToStr(score,1,5) + "\n";
		s += "#80C0FF"+TR("#{ScoreNeeded}") + ": " + fToStr(trk.passScore,1,5) + "\n\n";
		
		bool passed = score >= trk.passScore;
		if (passed)
			s += "#00FF00"+TR("#{Passed}")+".\n"+TR("#{NextStage}.");
		else
			s += "#FF8000"+TR("#{DidntPass}")+".\n"+TR("#{RepeatStage}.");
	}
	edChampStage->setCaption(s);
	
	//  preview image
	ResourceGroupManager& resMgr = ResourceGroupManager::getSingleton();
	Ogre::TextureManager& texMgr = Ogre::TextureManager::getSingleton();

	String path = PathListTrkPrv(0, trk.name), sGrp = "TrkPrv";
	resMgr.addResourceLocation(path, "FileSystem", sGrp);  // add for this track
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
