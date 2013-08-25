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


///
void App::BackFromChs()
{
	pSet->game.champ_num = -1;
	pSet->game.chall_num = -1;
	CarListUpd();  // off filtering
}

bool App::isChallGui()
{
	return imgChall && imgChall->getVisible();
}


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

	liChalls->removeAllItems();  int n=1;  size_t sel = ITEM_NONE;
	int p = 0; //pSet->gui.champ_rev ? 1 : 0;
	for (int i=0; i < chall.all.size(); ++i,++n)
	{
		const Chall& chl = chall.all[i];

		const ProgressChall& pc = progressL[p].chs[i];
		int ntrks = pc.trks.size();
		const String& clr = clrCh[chl.type];
		//String cars = carsXml.colormap[chl.ci->type];  if (cars.length() != 7)  clr = "#C0D0E0";
		
		liChalls->addItem(clr+ toStr(n/10)+toStr(n%10), 0);  int l = liChalls->getItemCount()-1;
		liChalls->setSubItemNameAt(1,l, clr+ chl.name.c_str());
		liChalls->setSubItemNameAt(2,l, clrsDiff[chl.diff]+ TR("#{Diff"+toStr(chl.diff)+"}"));
		liChalls->setSubItemNameAt(3,l, StrChallCars(chl));
		
		liChalls->setSubItemNameAt(4,l, clrsDiff[std::min(8,ntrks*2/3+1)]+ toStr(ntrks));
		liChalls->setSubItemNameAt(5,l, clrsDiff[std::min(8,int(chl.time/3.f/60.f))]+ GetTimeShort(chl.time));
		liChalls->setSubItemNameAt(6,l, clr+ fToStr(100.f * pc.curTrack / ntrks,0,3)+" %");
		liChalls->setSubItemNameAt(7,l, clr+ fToStr(pc.points,1,5));
		if (n-1 == pSet->gui.chall_num)  sel = l;
	}
	liChalls->setIndexSelected(sel);
}


///  Challenges list  sel changed,  fill Stages list
//----------------------------------------------------------------------------------------------------------------------
void App::listChallChng(MyGUI::MultiList2* chlist, size_t id)
{
	if (id==ITEM_NONE || liChalls->getItemCount() == 0)  return;

	int pos = s2i(liChalls->getItemNameAt(id).substr(7))-1;
	if (pos < 0 || pos >= chall.all.size())  {  LogO("Error chall sel > size.");  return;  }

	CarListUpd();  // filter car list

	//  fill stages
	liStages->removeAllItems();

	int n = 1, p = 0; //pSet->gui.champ_rev ? 1 : 0;
	const Chall& ch = chall.all[pos];
	int ntrks = ch.trks.size();
	for (int i=0; i < ntrks; ++i,++n)
	{
		const ChallTrack& trk = ch.trks[i];
		StageListAdd(n, trk.name, trk.laps,
			"#E0F0FF"+fToStr(progressL[p].chs[pos].trks[i].points,1,3));
	}
	//  descr
	EditBox* ed = mGUI->findWidget<EditBox>("ChampDescr");
	if (ed)  ed->setCaption(ch.descr);


	//  chall details  -----------------------------------
	String s1,s2,clr;
	s1 += "\n";  s2 += "\n";

	clr = clrsDiff[ch.diff];  // track
	s1 += clr+ TR("#{Difficulty}\n");    s2 += clr+ TR("#{Diff"+toStr(ch.diff)+"}")+"\n";

	clr = clrsDiff[std::min(8,ntrks*2/3+1)];
	s1 += clr+ TR("#{Tracks}\n");        s2 += clr+ toStr(ntrks)+"\n";

	s1 += "\n";  s2 += "\n";
	clr = clrsDiff[std::min(8,int(ch.time/3.f/60.f))];
	s1 += TR("#80F0E0#{Time} [m:s.]\n"); s2 += "#C0FFE0"+clr+ GetTimeShort(ch.time)+"\n";

	s1 += "\n";  s2 += "\n";  // cars
	s1 += TR("#F08080#{Cars}\n");      s2 += TR("#FFA0A0") + StrChallCars(ch)+"\n";
	
	s1 += "\n\n";  s2 += "\n\n";  // game
	#define cmbs(cmb, i)  (i>=0 ? cmb->getItemNameAt(i) : TR("#{Any}"))
	s1 += TR("#A0B0C0#{Simulation}\n");  s2 += "#B0C0D0"+ ch.sim_mode +"\n";
	s1 += TR("#A090E0#{Damage}\n");      s2 += "#B090FF"+ cmbs(cmbDamage, ch.damage_type) +"\n";
	s1 += "\n";  s2 += "\n";
	s1 += TR("#80C0F8#{Boost}\n");       s2 += "#A0D0FF"+ cmbs(cmbBoost, ch.boost_type) +"\n";
	s1 += TR("#80C0C0#{Flip}\n");        s2 += "#90D0D0"+ cmbs(cmbFlip, ch.flip_type) +"\n";
														//cmbs(cmbRewind, ch.rewind_type)
	//  hud
	//bool minimap, chk_arr, chk_beam, trk_ghost;  // deny using it if false
	//  pass ..
	//float totalTime, avgPoints, avgPos;
	
	s1 += "\n\n";  s2 += "\n\n";  // prog
	s1 += TR("#B0C0E0#{Progress}\n");    s2 += "#C0E0FF"+fToStr(100.f * progressL[p].chs[pos].curTrack / chall.all[pos].trks.size(),1,5)+" %\n";
	s1 += TR("#D8C0FF#{Score}\n");       s2 += "#F0D8FF"+fToStr(progressL[p].chs[pos].points,1,5)+"\n";

	txtCh->setCaption(s1);  valCh->setCaption(s2);
}


//  list allowed cars types and cars
String App::StrChallCars(const Chall& ch)
{
	String str;
	int i,s;

	s = ch.carTypes.size();
	for (i=0; i < s; ++i)
	{
		const String& ct = ch.carTypes[i];
			str += carsXml.colormap[ct];  // car type color
		str += ct;
		if (i+1 < s)  str += ",";
	}
	//int id = carsXml.carmap[*i];
	//carsXml.cars[id-1];
	if (!str.empty())
		str += " ";
	
	s = ch.cars.size();
	for (i=0; i < s; ++i)
	{
		const String& c = ch.cars[i];
			int id = carsXml.carmap[c]-1;  // get car color from type
			if (id >= 0)  str += carsXml.colormap[ carsXml.cars[id].type ];
		str += c;
		if (i+1 < s)  str += ",";
	}
	return str;
}

//  test if car is in challenge allowed cars or types
bool App::IsChallCar(String name)
{
	if (!liChalls || liChalls->getIndexSelected()==ITEM_NONE)  return true;

	int chId = s2i(liChalls->getItemNameAt(liChalls->getIndexSelected()).substr(7))-1;
	const Chall& ch = chall.all[chId];

	int i,s;
	if (!ch.carTypes.empty())
	{	s = ch.carTypes.size();

		int id = carsXml.carmap[name]-1;
		if (id >= 0)
		{
			String type = carsXml.cars[id].type;

			for (i=0; i < s; ++i)
				if (type == ch.carTypes[i])  return true;
	}	}
	if (!ch.cars.empty())
	{	s = ch.cars.size();

		for (i=0; i < s; ++i)
			if (name == ch.cars[i])  return true;
	}
	return false;
}


///  champ start
void App::btnChallStart(WP)
{
	if (liChalls->getIndexSelected()==ITEM_NONE)  return;
	pSet->gui.chall_num = s2i(liChalls->getItemNameAt(liChalls->getIndexSelected()).substr(7))-1;

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
	//..ChallsListUpdate();
	//..listChallChng(liChalls, liChalls->getIndexSelected());
}
