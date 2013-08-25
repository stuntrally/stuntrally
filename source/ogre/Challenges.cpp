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


void App::tabChallType(MyGUI::TabPtr wp, size_t id)
{
	//pSet->chall_type = id;
	ChallsListUpdate();
}

///  Challenges list  fill
//----------------------------------------------------------------------------------------------------------------------
void App::ChallsListUpdate()
{
	const char clrCh[7][8] = {
	// 0 tutorial  1 tutorial hard  // 2 normal  3 hard  4 very hard  // 5 scenery  6 test
		"#FFFFA0", "#E0E000",   "#A0F0FF", "#60C0FF", "#A0A0E0",   "#80FF80", "#909090"  };

	liChamps->removeAllItems();  int n=1;  size_t sel = ITEM_NONE;
	int p = 0; //pSet->gui.champ_rev ? 1 : 0;
	for (int i=0; i < chall.all.size(); ++i,++n)
	{
		const Chall& chl = chall.all[i];
		//if (pSet->inMenu == MNU_Tutorial && chl.type == pSet->tut_type ||
		//	pSet->inMenu == MNU_Champ && chl.type - 2 == pSet->champ_type)
		{
			const ProgressChall& pc = progressL[p].chs[i];
			int ntrks = pc.trks.size();
			const String& clr = clrCh[chl.type];
			liChamps->addItem(clr+ toStr(n/10)+toStr(n%10), 0);  int l = liChamps->getItemCount()-1;
			liChamps->setSubItemNameAt(1,l, clr+ chl.name.c_str());
			liChamps->setSubItemNameAt(2,l, clrsDiff[chl.diff]+ TR("#{Diff"+toStr(chl.diff)+"}"));
			liChamps->setSubItemNameAt(3,l, clrsDiff[std::min(8,ntrks*2/3+1)]+ toStr(ntrks));
			liChamps->setSubItemNameAt(4,l, clrsDiff[std::min(8,int(chl.time/3.f/60.f))]+ GetTimeShort(chl.time));
			liChamps->setSubItemNameAt(5,l, clr+ fToStr(100.f * pc.curTrack / ntrks,0,3)+" %");
			liChamps->setSubItemNameAt(6,l, clr+ fToStr(pc.points,1,5));
			if (n-1 == pSet->gui.chall_num)  sel = l;
	}	}
	liChamps->setIndexSelected(sel);
}

///  Challenges list  sel changed,  fill Stages list
//----------------------------------------------------------------------------------------------------------------------
void App::listChallChng(MyGUI::MultiList2* chlist, size_t id)
{
	if (id==ITEM_NONE || liChamps->getItemCount() == 0)  return;
	#if 0
	//  update champ stages
	liStages->removeAllItems();

	int pos = s2i(liChamps->getItemNameAt(id).substr(7))-1;
	if (pos < 0 || pos >= champs.champs.size())  {  LogO("Error champ sel > size.");  return;  }

	int n = 1, p = pSet->gui.champ_rev ? 1 : 0;
	const Champ& ch = champs.all[pos];
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
		liStages->setSubItemNameAt(5,l, "#E0F0FF"+fToStr(progress[p].chs[pos].trks[i].points,1,3));
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
	if (txt)  txt->setCaption(fToStr(100.f * progress[p].chs[pos].curTrack / champs.champs[pos].trks.size(),1,5));
	txt = (TextBox*)mWndGame->findWidget("valChScore");
	if (txt)  txt->setCaption(fToStr(progress[p].chs[pos].points,1,5));
	#endif
}


///  champ start
void App::btnChallStart(WP)
{
	if (liChamps->getIndexSelected()==ITEM_NONE)  return;
	pSet->gui.chall_num = s2i(liChamps->getItemNameAt(liChamps->getIndexSelected()).substr(7))-1;

	//  if already finished, restart - will loose progress and scores ..
	int chId = pSet->gui.chall_num, p = 0; //pSet->game.champ_rev ? 1 : 0;
	LogO("|| Starting chall: "+toStr(chId)+(p?" rev":""));
	ProgressChall& pc = progressL[p].chs[chId];
	if (pc.curTrack == pc.trks.size())
	{
		LogO("|| Was at 100%, restarting progress.");
		pc.curTrack = 0;  //pc.score = 0.f;
	}
	// change btn caption to start/continue/restart ?..

	btnNewGame(0);
}


///  save progressL and update it on gui
void App::ProgressLSave(bool upgGui)
{
	progressL[0].SaveXml(PATHMANAGER::UserConfigDir() + "/progressL.xml");
	progressL[1].SaveXml(PATHMANAGER::UserConfigDir() + "/progressL_rev.xml");
	if (!upgGui)
		return;
	ChallsListUpdate();
	listChallChng(liChamps, liChamps->getIndexSelected());
}

