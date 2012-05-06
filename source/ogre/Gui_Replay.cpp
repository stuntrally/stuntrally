#include "pch.h"
#include "common/Defines.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "OgreGame.h"
#include "FollowCamera.h"
#include <boost/filesystem.hpp>
#include <time.h>
#include "common/Gui_Def.h"
using namespace std;
using namespace Ogre;
using namespace MyGUI;


///  [Replay]  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
//------------------------------------------------------------------------------------------------------------------

void App::slRplPosEv(SL)  // change play pos
{
	if (!bRplPlay)  return;
	double oldt = pGame->timer.GetReplayTime(0);
	double v = val;  v = std::max(0.0, std::min(1.0, v));  v *= replay.GetTimeLength();
	pGame->timer.SetReplayTime(0, v);

	FollowCamera* fCam = (*carModels.begin())->fCam;
	fCam->first = true;  // instant change
	//for (int i=0; i < 10; ++i)
	//	fCam->update(abs(v-oldt)/10.f, 0);  //..?
}

void App::btnRplLoad(WP)  // Load
{
	//  from list
	int i = rplList->getIndexSelected();
	if (i == MyGUI::ITEM_NONE)  return;

	String name = rplList->getItemNameAt(i).substr(7);
	string file = (pSet->rpl_listview == 2 ? PATHMANAGER::GetGhostsPath() : PATHMANAGER::GetReplayPath()) + "/" + name + ".rpl";

	if (!replay.LoadFile(file))
	{
		Message::createMessageBox(  // #{.. translate
			"Message", "Load Replay", "Error: Can't load file.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
	}
	else  // car, track change
	{
		const ReplayHeader& h = replay.header;
		string car = h.car, trk = h.track;
		bool usr = h.track_user == 1;

		//  set game config from replay
		pSet->game.car[0] = car;  pSet->game.track = trk;  pSet->game.track_user = usr;
		pSet->game.car_hue[0] = h.hue[0];  pSet->game.car_sat[0] = h.sat[0];  pSet->game.car_val[0] = h.val[0];
		pSet->game.trees = h.ver < 6 ? 1.f : h.trees;  // older didnt have trees saved, so use default 1
		pSet->game.local_players = h.numPlayers;
		LogO("RPL btn Load  players: "+toStr(h.numPlayers)+" netw: "+ toStr(h.networked));

		for (int p=1; p < h.numPlayers; ++p)
		{	pSet->game.car[p] = h.cars[p-1];
			pSet->game.car_hue[p] = h.hue[p];  pSet->game.car_sat[p] = h.sat[p];  pSet->game.car_val[p] = h.val[p];
		}
		newGameRpl = true;
		btnNewGame(0);
		bRplPlay = 1;
	}
}

void App::btnRplSave(WP)  // Save
{
	String edit = edRplName->getCaption();
	String file = PATHMANAGER::GetReplayPath() + "/" + pSet->game.track + "_" + edit + ".rpl";
	///  save
	if (boost::filesystem::exists(file.c_str()))
	{
		Message::createMessageBox(  // #{..
			"Message", "Save Replay", "File already exists.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return;
	}
	if (!replay.SaveFile(file.c_str()))
	{
		Message::createMessageBox(  // #{..
			"Message", "Save Replay", "Error: Can't save file.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
	}
	updReplaysList();
}

//  list change
void App::listRplChng(List* li, size_t pos)
{
	size_t i = li->getIndexSelected();  if (i == ITEM_NONE)  return;
	String name = li->getItemNameAt(i).substr(7);
	string file = (pSet->rpl_listview == 2 ? PATHMANAGER::GetGhostsPath() : PATHMANAGER::GetReplayPath()) + "/" + name + ".rpl";
	if (!valRplName)  return;  valRplName->setCaption(name);
	if (!valRplInfo)  return;  edRplName->setCaption(name);
	
	//  load replay header, upd info text
	Replay rpl;  char stm[128];
	if (rpl.LoadFile(file,true))
	{
		String ss = String(TR("#{Track}: ")) + GetSceneryColor(rpl.header.track)+rpl.header.track + (rpl.header.track_user ? "  *user*" : "");
		valRplName->setCaption(ss);

		ss = String(TR("#{Car}: ")) + rpl.header.car + "       "+
			(rpl.header.numPlayers == 1 ? "" : (TR("#{Players}: ") + toStr(rpl.header.numPlayers))) + "  " +
			(rpl.header.networked == 0 ? "" : "M") +  //TR("#{Multiplayer}")
			"\n" + TR("#{RplTime}: ") + GetTimeString(rpl.GetTimeLength());
		if (rpl.header.networked == 1)  // list nicks
			ss += String("\n#90C0E0")+rpl.header.nicks[0]+"  "+rpl.header.nicks[1]+"  "+rpl.header.nicks[2]+"  "+rpl.header.nicks[3];
		//else  // other cars, car colors ..
			//ss += String("\n#90C0E0")+rpl.header.cars[0]+"  "+rpl.header.cars[1]+"  "+rpl.header.cars[2];
		valRplInfo->setCaption(ss);

		//  file stats
		int size = boost::filesystem::file_size(file);
		std::time_t ti = boost::filesystem::last_write_time(file);
		if (!std::strftime(stm, 126, "%d.%b'%y  %a %H:%M", std::localtime(&ti)))  stm[0]=0;
		
		ss =/*"Time: "+*/String(stm)+"\n"+
			String(TR("#{RplFileSize}: ")) + fToStr( float(size)/1000000.f, 2,5) + TR(" #{UnitMB}\n") +
			TR("#{RplVersion}: ") + toStr(rpl.header.ver) + "     " + toStr(rpl.header.frameSize) + "B";
		if (valRplInfo2)  valRplInfo2->setCaption(ss);
	}
	//edRplDesc
}


//  replay settings

void App::chkRplAutoRec(WP wp)
{
	bRplRec = !bRplRec;  // changes take effect next game start
	if (!wp)  return;
	ButtonPtr chk = wp->castType<MyGUI::Button>();
    chk->setStateSelected(bRplRec);
}

void App::chkRplChkGhost(WP wp){		ChkEv(rpl_ghost);		}
void App::chkRplChkBestOnly(WP wp){		ChkEv(rpl_bestonly);	}
void App::chkRplChkAlpha(WP wp){		ChkEv(rpl_alpha);		}
void App::chkRplChkPar(WP wp){			ChkEv(rpl_ghostpar);	}

void App::slRplNumViewports(SL)
{
	int v = 1.f + 3.f * val;	if (bGI)  pSet->rpl_numViews = v;
	if (valRplNumViewports)  valRplNumViewports->setCaption(toStr(v));
}


//  replays list filtering

void App::btnRplAll(WP)
{
	rbRplCur->setStateSelected(false);  rbRplAll->setStateSelected(true);  rbRplGhosts->setStateSelected(false);
	pSet->rpl_listview = 0;  updReplaysList();
}

void App::btnRplCur(WP)
{
	rbRplCur->setStateSelected(true);  rbRplAll->setStateSelected(false);  rbRplGhosts->setStateSelected(false);
	pSet->rpl_listview = 1;  updReplaysList();
}

void App::btnRplGhosts(WP)
{
	rbRplCur->setStateSelected(false);  rbRplAll->setStateSelected(false);  rbRplGhosts->setStateSelected(true);
	pSet->rpl_listview = 2;  updReplaysList();
}


//  replay controls

void App::btnRplToStart(WP)
{
	pGame->timer.RestartReplay(0);
}

void App::btnRplToEnd(WP)
{
}

void App::btnRplBackDn(WP, int, int, MouseButton){	bRplBack = true;  }
void App::btnRplBackUp(WP, int, int, MouseButton){	bRplBack = false;  }
void App::btnRplFwdDn(WP, int, int, MouseButton){	bRplFwd = true;  }
void App::btnRplFwdUp(WP, int, int, MouseButton){	bRplFwd = false;  }

void App::btnRplPlay(WP)  // play / pause
{
	bRplPause = !bRplPause;
	UpdRplPlayBtn();
}

void App::UpdRplPlayBtn()
{
	String sign = bRplPause ? "|>" : "||";
	if (btRplPl)
		btRplPl->setCaption(sign);
}


void App::updReplaysList()
{
	if (!rplList)  return;
	rplList->removeAllItems();

	strlist li;
	PATHMANAGER::GetFolderIndex((pSet->rpl_listview == 2 ? PATHMANAGER::GetGhostsPath() : PATHMANAGER::GetReplayPath()), li, "rpl");
	
	for (strlist::iterator i = li.begin(); i != li.end(); ++i)
	if (StringUtil::endsWith(*i, ".rpl"))
	{
		String s = *i;  s = StringUtil::replaceAll(s,".rpl","");
		if (pSet->rpl_listview != 1 || StringUtil::startsWith(s,pSet->game.track, false))
		{
			String ch = " ";  // 1st A-Z char for scenery
			int l = s.length();
			for (int i=0; i < l; ++i)
				if (s[i] >= 'A' && s[i] <= 'Z') {  ch = s[i];  break;  }

			rplList->addItem(GetSceneryColor(ch) + s);
		}
	}
}


//  Delete
void App::btnRplDelete(WP)
{
	size_t i = rplList->getIndexSelected();  if (i == ITEM_NONE)  return;
	string name = rplList->getItemNameAt(i).substr(7);

	Message* message = Message::createMessageBox(
		"Message", "Delete Replay ?", name,  // #{..
		MessageBoxStyle::IconQuest | MessageBoxStyle::Yes | MessageBoxStyle::No);
	message->eventMessageBoxResult += newDelegate(this, &App::msgRplDelete);
}
void App::msgRplDelete(Message* sender, MessageBoxStyle result)
{
	if (result != MessageBoxStyle::Yes)  return;
	size_t i = rplList->getIndexSelected();  if (i == ITEM_NONE)  return;
	string name = rplList->getItemNameAt(i).substr(7);
	string file = (pSet->rpl_listview == 2 ? PATHMANAGER::GetGhostsPath() : PATHMANAGER::GetReplayPath()) +"/"+ name + ".rpl";

	if (boost::filesystem::exists(file))
		boost::filesystem::remove(file);
	updReplaysList();
}

//  Rename
void App::btnRplRename(WP)
{
	if (pSet->rpl_listview == 2)  return;  // cant rename ghosts
	size_t i = rplList->getIndexSelected();  if (i == ITEM_NONE)  return;
	string name = rplList->getItemNameAt(i).substr(7);
	string edit = edRplName->getCaption();

	if (name == edit)  // same name
	{	Message::createMessageBox(
			"Message", "Rename Replay", "Same names.\n",  // #{..
			MessageBoxStyle::IconInfo | MessageBoxStyle::Ok);
		return;  }
	
	string file = PATHMANAGER::GetReplayPath() + "/" + name + ".rpl";
	string fileNew = PATHMANAGER::GetReplayPath() + "/" + edit + ".rpl";

	if (boost::filesystem::exists(fileNew))
	{	Message::createMessageBox(
			"Message", "Rename Replay", "File already exists.\n",  // #{..
			MessageBoxStyle::IconInfo | MessageBoxStyle::Ok);
		return;  }

	if (boost::filesystem::exists(file))
		boost::filesystem::rename(file, fileNew);
	updReplaysList();
}
