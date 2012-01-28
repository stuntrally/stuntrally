#include "pch.h"
#include "Defines.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "OgreGame.h"

using namespace std;
using namespace Ogre;
using namespace MyGUI;


///  Load  championships.xml, progress.xml (once)
void App::ChampsXmlLoad()
{
	champs.LoadXml(PATHMANAGER::GetGameConfigDir() + "/championships.xml");
	LogO(String("**** Loaded Championships: ") + toStr(champs.champs.size()));
	
	progress.LoadXml(PATHMANAGER::GetUserConfigDir() + "/progress.xml");

	//  if progress empty (?or size differs than champs) update progress
	if (progress.champs.size() == 0)
	{
		for (int i=0; i < champs.champs.size(); ++i)
		{
			ProgressChamp pc;
			ProgressTrack pt;  // empty, 0 progress
			for (int t=0; t < champs.champs[i].trks.size(); ++t)
				pc.trks.push_back(pt);
			progress.champs.push_back(pc);
		}
		ProgressSave();
	}
	//TODO: check ver or trks size, if different reset that champ only ...
}

///  load championship track
void App::ChampNewGame()
{
	if (pSet->game.champ_num >= champs.champs.size())
		pSet->game.champ_num = -1;  //0 range

	int chId = pSet->game.champ_num;
	if (chId >= 0)
	{
		//  champ stage, current track
		const ProgressChamp& pc = progress.champs[chId];
		const Champ& ch = champs.champs[chId];
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

///  Championships list  sel changed
//---------------------------------------------------------------------
void App::listChampChng(MyGUI::MultiListBox* chlist, size_t pos)
{
	if (pos < 0)  return;
	if (pos >= champs.champs.size())  {  LogO("Error champ sel > size.");  return;  }
	//if (pos >= progress.champs.size())  {  LogO("Error progres sel > size.");  return;  }
	
	//  update champ stages
	MultiListBox* li = mGUI->findWidget<MultiListBox>("MListStages");
	li->removeAllItems();
	const Champ& ch = champs.champs[pos];
	for (int i=0; i < ch.trks.size(); ++i)
	{
		const ChampTrack& tr = ch.trks[i];
		li->addItem(toStr(i/10)+toStr(i%10), 0);  int l = li->getItemCount()-1;
		li->setSubItemNameAt(1,l, tr.name.c_str());
		li->setSubItemNameAt(2,l, "-");
		li->setSubItemNameAt(3,l, "-");  //scenery..
		li->setSubItemNameAt(4,l, toStr(tr.laps));
		li->setSubItemNameAt(5,l, "0");
	}
	//  descr
	EditBox* ed = mGUI->findWidget<EditBox>("ChampDescr");
	if (ed)  ed->setCaption(ch.descr);

	//  update champ details (on stages tab)
	TextBox* txt;
	txt = mGUI->findWidget<TextBox>("valChDiff");
	if (txt)  txt->setCaption(TR("#{Diff"+toStr(ch.diff)+"}"));
	txt = mGUI->findWidget<TextBox>("valChTracks");
	if (txt)  txt->setCaption(toStr(ch.trks.size()));

	txt = mGUI->findWidget<TextBox>("valChDist");
	if (txt)  txt->setCaption(toStr(ch.length));  // sum from find tracks..
	txt = mGUI->findWidget<TextBox>("valChTime");
	if (txt)  txt->setCaption(toStr(ch.time));    // sum champs.trkTimes..

	txt = mGUI->findWidget<TextBox>("valChProgress");
	if (txt)  txt->setCaption(toStr(100.f * progress.champs[pos].curTrack / champs.champs[pos].trks.size())+" %");
	txt = mGUI->findWidget<TextBox>("valChScore");
	if (txt)  txt->setCaption(toStr(progress.champs[pos].score));
}

///  champ start  -----
void App::btnChampStart(WP)
{
	pSet->gui.champ_num = liChamps->getIndexSelected();
	LogO("|| Starting champ: "+toStr(pSet->gui.champ_num));

	// if already finished, restart - will loose progress and score ..
	int chId = pSet->gui.champ_num;
	ProgressChamp& pc = progress.champs[chId];
	if (pc.curTrack == pc.trks.size())
		pc.curTrack = 0;

	btnNewGame(0);
}

//  stage back
void App::btnChampStageBack(WP)
{
	mWndChampStage->setVisible(false);
	isFocGui = true;  // show back gui
	toggleGui(false);
}

///  stage start / end  -----
void App::btnChampStageStart(WP)
{
	//  check if champ ended
	int chId = pSet->game.champ_num;
	ProgressChamp& pc = progress.champs[chId];
	const Champ& ch = champs.champs[chId];
	bool last = pc.curTrack == ch.trks.size();
	LogO("|| This was stage 2 close" + toStr(pc.curTrack) + "/" + toStr(ch.trks.size()));
	if (last)
	{	//  show end window
		mWndChampStage->setVisible(false);
		mWndChampEnd->setVisible(true);
		return;
	}

	bool finished = pGame->timer.GetLastLap() > 0.f;  //?-
	if (finished)
	{
		LogO("|| Loading next stage: "/* + ch.trks[pc.curTrack].name*/);
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

void App::ProgressSave()
{
	progress.SaveXml(PATHMANAGER::GetUserConfigDir() + "/progress.xml");
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

	///  compute track :score:
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

	float score = timeCur / timeBest * 100.f;	//(timeBest-timeCur)/timeBest * 100.f;  //-
	LogO("|| Score: " + toStr(score));
	pc.trks[pc.curTrack].score = score;

	//  advance
	bool last = pc.curTrack+1 == ch.trks.size();
	LogO("|| This was stage " + toStr(pc.curTrack+1) + "/" + toStr(ch.trks.size()));
	if (!last)
	{
		//  show stage end window
		pGame->pause = true;
		pGame->timer.waiting = true;

		ChampFillStageInfo(true);  // cur track
		mWndChampStage->setVisible(true);
		
		pc.curTrack++;  // next stage
		ProgressSave();
	}else
	{	//  champ ended
		pGame->pause = true;
		pGame->timer.waiting = true;

		ChampFillStageInfo(true);  // cur track
		mWndChampStage->setVisible(true);

		///  compute champ :score:
		int ntrk = pc.trks.size();  float sum = 0.f;
		for (int t=0; t < ntrk; ++t)
			sum += pc.trks[t].score;

		pc.curTrack++;  // = end 100 %
		pc.score = sum / ntrk;  // average from all tracks
		ProgressSave();
		LogO("|| Champ finished");
		LogO("|| Total score: " + toStr(score));  //..
		
		//  show end window
		String s = "Total score: " + toStr(score);
		edChampEnd->setCaption(s);
		//mWndChampEnd->setVisible(true);  // show after stage end
	}
}

void App::ChampFillStageInfo(bool finished)
{
	int chId = pSet->game.champ_num;
	ProgressChamp& pc = progress.champs[chId];
	const Champ& ch = champs.champs[chId];
	const std::string& trkName = ch.trks[pc.curTrack].name;

	String s;
	s = "Champ: " + ch.name + "\n" +
		"Stage: " + toStr(pc.curTrack+1) + "/" + toStr(ch.trks.size()) + "\n" +
		"Track: " + trkName + "\n\n" /*+
		"Difficulty: " + tracksXml. + "\n"*/;
	if (finished)
	s += String("Finished.\n") +
		"Score: " + toStr(pc.trks[pc.curTrack].score) + "\n";
	edChampStage->setCaption(s);
	
	imgChampStage->setImageTexture(trkName+".jpg");
}
