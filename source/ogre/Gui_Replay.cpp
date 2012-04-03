#include "pch.h"
#include "common/Defines.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "OgreGame.h"
#include "FollowCamera.h"
#include <boost/filesystem.hpp>
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
	double v = val/res;  v = std::max(0.0, std::min(1.0, v));  v *= replay.GetTimeLength();
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

	String name = rplList->getItemNameAt(i);
	string file = (pSet->rpl_listview == 2 ? PATHMANAGER::GetGhostsPath() : PATHMANAGER::GetReplayPath()) + "/" + name + ".rpl";

	if (!replay.LoadFile(file))
	{
		Message::createMessageBox(  // #{.. translate
			"Message", "Load Replay", "Error: Can't load file.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
	}
	else  // car, track change
	{
		string car = replay.header.car, trk = replay.header.track;
		bool usr = replay.header.track_user == 1;

		//  set game config from replay
		pSet->game.car[0] = car;  pSet->game.track = trk;  pSet->game.track_user = usr;
		pSet->game.car_hue[0] = replay.header.hue[0];  pSet->game.car_sat[0] = replay.header.sat[0];  pSet->game.car_val[0] = replay.header.val[0];
		pSet->game.trees = replay.header.ver < 6 ? 1.f : replay.header.trees;  // older didnt have trees saved, so use default 1
		pSet->game.local_players = replay.header.numPlayers;
		LogO("RPL btn Load  players: "+toStr(replay.header.numPlayers)+" netw: "+ toStr(replay.header.networked));

		for (int p=1; p < replay.header.numPlayers; ++p)
		{	pSet->game.car[p] = replay.header.cars[p-1];
			pSet->game.car_hue[p] = replay.header.hue[p];  pSet->game.car_sat[p] = replay.header.sat[p];  pSet->game.car_val[p] = replay.header.val[p];
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
	String name = li->getItemNameAt(i);
	string file = (pSet->rpl_listview == 2 ? PATHMANAGER::GetGhostsPath() : PATHMANAGER::GetReplayPath()) + "/" + name + ".rpl";
	if (!valRplName)  return;  valRplName->setCaption(name);
	if (!valRplInfo)  return;
	
	//  load replay header upd text descr
	Replay rpl;
	if (rpl.LoadFile(file,true))
	{
		String ss = String(TR("#{Track}: ")) + rpl.header.track + (rpl.header.track_user ? "  *user*" : "");
		valRplName->setCaption(ss);

		ss = String(TR("#{Car}: ")) + rpl.header.car + "       "+
			(rpl.header.numPlayers == 1 ? "" : (TR("#{Players}: ") + toStr(rpl.header.numPlayers))) + "  " +
			(rpl.header.networked == 0 ? "" : "M"/*TR("#{Multiplayer}")*/) +
			//(rpl.header.cars[0][1] != 0 ? " , " + rpl.header.cars[1] : "") +
			"\n" + TR("#{RplTime}: ") + GetTimeString(rpl.GetTimeLength());
		valRplInfo->setCaption(ss);

		int size = boost::filesystem::file_size(file);
		ss = String(TR("#{RplFileSize}:")) + fToStr( float(size)/1000000.f, 2,5) + TR(" #{UnitMB}\n") +  //todo: file date..
			TR("#{RplVersion}: ") + toStr(rpl.header.ver) + "     " + toStr(rpl.header.frameSize) + "B";
		if (valRplInfo2)  valRplInfo2->setCaption(ss);
	}
	//edRplDesc  edRplName
}


//  replay settings

void App::chkRplAutoRec(WP wp)
{
	bRplRec = !bRplRec;  // changes take effect next game start
	if (!wp)  return;
	ButtonPtr chk = wp->castType<MyGUI::Button>();
    chk->setStateSelected(bRplRec);
}

void App::chkRplChkGhost(WP wp)
{
	ChkEv(rpl_ghost);
}

void App::chkRplChkBestOnly(WP wp)
{
	ChkEv(rpl_bestonly);
}

void App::chkRplChkAlpha(WP wp)
{
	ChkEv(rpl_alpha);
}

void App::chkRplChkPar(WP wp)
{
	ChkEv(rpl_ghostpar);
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
			rplList->addItem(s);
	}
}


//  Delete
void App::btnRplDelete(WP)
{
	size_t i = rplList->getIndexSelected();  if (i == ITEM_NONE)  return;
	String name = rplList->getItemNameAt(i);
	Message* message = Message::createMessageBox(
		"Message", "Delete Replay ?", name,  // #{..
		MessageBoxStyle::IconQuest | MessageBoxStyle::Yes | MessageBoxStyle::No);
	message->eventMessageBoxResult += newDelegate(this, &App::msgRplDelete);
	//message->setUserString("FileName", fileName);
}
void App::msgRplDelete(Message* sender, MessageBoxStyle result)
{
	if (result != MessageBoxStyle::Yes)
		return;
	size_t i = rplList->getIndexSelected();  if (i == ITEM_NONE)  return;
	String name = rplList->getItemNameAt(i);
	
	string file = (pSet->rpl_listview == 2 ? PATHMANAGER::GetGhostsPath() : PATHMANAGER::GetReplayPath()) +"/"+ name + ".rpl";
	if (boost::filesystem::exists(file))
		boost::filesystem::remove(file);
	updReplaysList();
}

//  Rename
void App::btnRplRename(WP)
{
	//if (boost::filesystem::exists(from.c_str()))
	//	boost::filesystem::rename(from.c_str(), to.c_str());
}
