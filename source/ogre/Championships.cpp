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



void App::chkChampRev(WP wp)
{
	pSet->gui.champ_rev = !pSet->gui.champ_rev;
	ButtonPtr chk = wp->castType<MyGUI::Button>();
    chk->setStateSelected(pSet->gui.champ_rev);
	ChampsListUpdate();
	ChallsListUpdate();
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
//----------------------------------------------------------------------------------------------------------------------
void App::ChampsListUpdate()
{
	const char clrCh[7][8] = {
	// 0 tutorial  1 tutorial hard  // 2 normal  3 hard  4 very hard  // 5 scenery  6 test
		"#FFFFA0", "#E0E000",   "#A0F0FF", "#60C0FF", "#A0A0E0",   "#80FF80", "#909090"  };

	liChamps->removeAllItems();  int n=1;  size_t sel = ITEM_NONE;
	int p = pSet->gui.champ_rev ? 1 : 0;
	for (int i=0; i < champs.all.size(); ++i,++n)
	{
		const Champ& ch = champs.all[i];
		if (pSet->inMenu == MNU_Tutorial && ch.type == pSet->tut_type ||
			pSet->inMenu == MNU_Champ && ch.type - 2 == pSet->champ_type)
		{
			const ProgressChamp& pc = progress[p].chs[i];
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
//----------------------------------------------------------------------------------------------------------------------
void App::listChampChng(MyGUI::MultiList2* chlist, size_t id)
{
	if (id==ITEM_NONE || liChamps->getItemCount() == 0)  return;
	
	//  fill stages
	liStages->removeAllItems();

	int pos = s2i(liChamps->getItemNameAt(id).substr(7))-1;
	if (pos < 0 || pos >= champs.all.size())  {  LogO("Error champ sel > size.");  return;  }

	int n = 1, p = pSet->gui.champ_rev ? 1 : 0;
	const Champ& ch = champs.all[pos];
	int ntrks = ch.trks.size();
	for (int i=0; i < ntrks; ++i,++n)
	{
		const ChampTrack& trk = ch.trks[i];
		StageListAdd(n, trk.name, trk.laps,
			"#E0F0FF"+fToStr(progress[p].chs[pos].trks[i].points,1,3));
	}
	if (edChDesc)  edChDesc->setCaption(ch.descr);


	//  champ details  -----------------------------------
	String s1,s2,clr;
	s1 += "\n";  s2 += "\n";

	clr = clrsDiff[ch.diff];
	s1 += clr+ TR("#{Difficulty}\n");    s2 += clr+ TR("#{Diff"+toStr(ch.diff)+"}")+"\n";

	clr = clrsDiff[std::min(8,ntrks*2/3+1)];
	s1 += clr+ TR("#{Tracks}\n");        s2 += clr+ toStr(ntrks)+"\n";

	s1 += "\n";  s2 += "\n";
	clr = clrsDiff[std::min(8,int(ch.time/3.f/60.f))];
	s1 += TR("#80F0E0#{Time} [m:s.]\n"); s2 += "#C0FFE0"+clr+ GetTimeShort(ch.time)+"\n";

	s1 += "\n";  s2 += "\n";
	s1 += TR("#B0C0E0#{Progress}\n");    s2 += "#C0E0FF"+fToStr(100.f * progress[p].chs[pos].curTrack / champs.all[pos].trks.size(),1,5)+" %\n";
	s1 += TR("#D8C0FF#{Score}\n");       s2 += "#F0D8FF"+fToStr(progress[p].chs[pos].points,1,5)+"\n";

	txtCh->setCaption(s1);  valCh->setCaption(s2);
}


///  champ start
//---------------------------------------------------------------------
void App::btnChampStart(WP)
{
	if (liChamps->getIndexSelected()==ITEM_NONE)  return;
	pSet->gui.champ_num = s2i(liChamps->getItemNameAt(liChamps->getIndexSelected()).substr(7))-1;

	//  if already finished, restart - will loose progress and scores ..
	int chId = pSet->gui.champ_num, p = pSet->game.champ_rev ? 1 : 0;
	LogO("|| Starting champ: "+toStr(chId)+(p?" rev":""));
	ProgressChamp& pc = progress[p].chs[chId];
	if (pc.curTrack == pc.trks.size())
	{
		LogO("|| Was at 100%, restarting progress.");
		pc.curTrack = 0;  //pc.score = 0.f;
	}
	// change btn caption to start/continue/restart ?..

	btnNewGame(0);
}

///  stage start / end
//----------------------------------------------------------------------------------------------------------------------
void App::btnChampStageStart(WP)
{
	//  check if champ ended
	int chId = pSet->game.champ_num, p = pSet->game.champ_rev ? 1 : 0;
	ProgressChamp& pc = progress[p].chs[chId];
	const Champ& ch = champs.all[chId];
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

//  stage back
void App::btnChampStageBack(WP)
{
	mWndChampStage->setVisible(false);
	isFocGui = true;  // show back gui
	toggleGui(false);
}

//  champ end
void App::btnChampEndClose(WP)
{
	mWndChampEnd->setVisible(false);
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
//----------------------------------------------------------------------------------------------------------------------
void App::ChampionshipAdvance(float timeCur)
{
	int chId = pSet->game.champ_num, p = pSet->game.champ_rev ? 1 : 0;
	ProgressChamp& pc = progress[p].chs[chId];
	const Champ& ch = champs.all[chId];
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


//  stage wnd text
//----------------------------------------------------------------------------------------------------------------------
void App::ChampFillStageInfo(bool finished)
{
	int chId = pSet->game.champ_num, p = pSet->game.champ_rev ? 1 : 0;
	ProgressChamp& pc = progress[p].chs[chId];
	const Champ& ch = champs.all[chId];
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

		String path = PathListTrkPrv(0, trk.name), sGrp = "TrkPrvCh";
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
