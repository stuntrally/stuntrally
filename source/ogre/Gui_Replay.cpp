#include "pch.h"
#include "common/Def_Str.h"
#include "common/Gui_Def.h"
#include "common/GuiCom.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/game.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "FollowCamera.h"
#include <time.h>
#include <boost/filesystem.hpp>
using namespace std;
using namespace Ogre;
using namespace MyGUI;
namespace fs = boost::filesystem;


///  [Replay]  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
//------------------------------------------------------------------------------------------------------------------

void CGui::slRplPosEv(SL)  // change play pos
{
	if (!app->bRplPlay)  return;
	double oldt = pGame->timer.GetReplayTime(0);
	double v = val;  v = std::max(0.0, std::min(1.0, v));  v *= app->replay.GetTimeLength();
	pGame->timer.SetReplayTime(0, v);

	FollowCamera* fCam = (*app->carModels.begin())->fCam;
	fCam->First();  // instant change
	//for (int i=0; i < 10; ++i)
	//	fCam->update(fabs(v-oldt)/10.f, 0);  //..?
}

String CGui::getRplName()
{
	String name;
	int i = rplList->getIndexSelected(), p;  if (i == ITEM_NONE)  return name;
	name = rplList->getItemNameAt(i);
	if (name.length() > 7 && name[0]=='#')  name = name.substr(7);
	return name;
}


void CGui::btnRplLoad(WP)  // Load
{
	//  from list
	String name = getRplName();  if (name.empty())  return;
	string file = GetRplListDir() + "/" + name + ".rpl";

	if (!app->replay.LoadFile(file))
	{
		Message::createMessageBox(
			"Message", TR("#{Replay} - #{RplLoad}"), TR("#{Error}."),
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
	}else
	{	//  car, track change
		const ReplayHeader2& h = app->replay.header;
		string trk = h.track;
		bool usr = h.track_user == 1;

		//  check if cars, track exist
		String er;  int p;
		if (!h.track_user && !fs::exists(PATHMANAGER::Tracks()+"/"+trk))
			er += TR("#{Track}: ")+trk+TR(" - #{DoesntExist}.\n");
		if (h.track_user && !fs::exists(PATHMANAGER::TracksUser()+"/"+trk))
			er += TR("#{Track} (#{TweakUser}): ")+trk+TR(" - #{DoesntExist}.\n");

		for (p=0; p < h.numPlayers; ++p)
			if (!fs::exists(PATHMANAGER::Cars()+"/"+h.cars[p]))
				er += TR("#{Vehicle}: ")+h.cars[p]+TR(" - #{DoesntExist}.\n");

		if (!er.empty())
		{	Message::createMessageBox(
				"Message", TR("#{Replay} - #{RplLoad} - #{Error}"), "\n"+er,
				MessageBoxStyle::IconError | MessageBoxStyle::Ok);
			return;
		}
		//trackreverse 	num_laps
		//collis_veget, collis_cars, collis_roadw, dyn_objects;
		//int boost_type, flip_type;  float boost_power;
		//float pre_time;

		//  set game config from replay
		pSet->game = pSet->gui;
		pSet->game.track = trk;  pSet->game.track_user = usr;

		pSet->game.trees = h.trees;
		pSet->game.local_players = h.numPlayers;
		BackFromChs();
		LogO("RPL btn Load  players: "+toStr(h.numPlayers)+" netw: "+ toStr(h.networked));

		for (p=0; p < h.numPlayers; ++p)
		{	pSet->game.car[p] = h.cars[p];
		}
		app->newGameRpl = true;
		btnNewGame(0);
		app->bRplPlay = 1;  app->iRplSkip = 0;
	}
}

void CGui::btnRplSave(WP)  // Save
{
	String edit = edRplName->getCaption();
	String file = PATHMANAGER::Replays() + "/" + pSet->game.track + "_" + edit + ".rpl";
	///  save
	if (PATHMANAGER::FileExists(file))
	{
		Message::createMessageBox(
			"Message", TR("#{Replay} - #{RplSave}"), TR("#{AlreadyExists}."),
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return;
	}
	if (!app->replay.SaveFile(file.c_str()))
	{
		Message::createMessageBox(
			"Message", TR("#{Replay} - #{RplSave}"), TR("#{Error}."),
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
	}
	updReplaysList();
}

//  list change
void CGui::listRplChng(List* li, size_t pos)
{
	String name = getRplName();  if (name.empty())  return;
	string file = GetRplListDir() + "/" + name + ".rpl";
	valRplName->setCaption(name);
	edRplName->setCaption(name);
	
	//  load replay header, upd info text
	Replay2 rpl;  char stm[128];
	if (rpl.LoadFile(file,true))
	{
		const ReplayHeader2& rh = rpl.header;
		String ss = String(TR("#{Track}: ")) + gcom->GetSceneryColor(rh.track) +
			rh.track + (rh.track_user ? "  *"+TR("#{TweakUser}")+"*" : "");
		valRplName->setCaption(ss);
		char pp = rh.numPlayers, netw = rh.networked, n;

		ss = String(TR("#{Vehicles}: "));
		for (n=0; n < pp; ++n)  ss += rh.cars[n] + "  ";
		ss += //(netw == 0 ? "" : "M") +  //TR("#{Multiplayer}")
			"\n#C0D8F0" + TR("#{RplTime}: ") + StrTime(rpl.GetTimeLength()) +
			"\n#90A0B0" + TR("#{Simulation}: ") + rh.sim_mode;

		if (netw == 1)  // list nicks
		{	ss += String("\n#90C0E0");
			for (n=0; n < pp; ++n)
				ss += rh.nicks[n]+"  ";
		}
		valRplInfo->setCaption(ss);

		//  file stats
		int size = fs::file_size(file);
		std::time_t ti = fs::last_write_time(file);
		if (!std::strftime(stm, 126, "%d.%b'%y  %a %H:%M", std::localtime(&ti)))  stm[0]=0;
		
		ss = String(stm)+"\n#A0A0A0"+  // date
			String(TR("#{RplFileSize}: ")) + fToStr( float(size)/1000000.f, 2,5) + TR(" #{UnitMB}") + "\n#808080" +
			TR("#{RplVersion}: ") + toStr(rh.ver);
		if (valRplInfo2)  valRplInfo2->setCaption(ss);
	}
	//edRplDesc
}


//  replay settings

void CGui::slRplNumViewports(SL)
{
	int v = 1.f + 3.f * val;	if (bGI)  pSet->rpl_numViews = v;
	if (valRplNumViewports)  valRplNumViewports->setCaption(toStr(v));
}


//  replays list filtering

void CGui::btnRplAll(WP)
{
	rbRplCur->setStateSelected(false);  rbRplAll->setStateSelected(true);
	pSet->rpl_listview = 0;  updReplaysList();
}

void CGui::btnRplCur(WP)
{
	rbRplCur->setStateSelected(true);  rbRplAll->setStateSelected(false);
	pSet->rpl_listview = 1;  updReplaysList();
}

void CGui::chkRplGhosts(Ck*)
{
	updReplaysList();
}


//  replay controls

void CGui::btnRplToStart(WP)
{
	pGame->timer.RestartReplay(0);
}

void CGui::btnRplToEnd(WP)
{
}

void CGui::btnRplBackDn(WP, int, int, MouseButton){	bRplBack = true;  }
void CGui::btnRplBackUp(WP, int, int, MouseButton){	bRplBack = false;  }
void CGui::btnRplFwdDn(WP, int, int, MouseButton){	bRplFwd = true;  }
void CGui::btnRplFwdUp(WP, int, int, MouseButton){	bRplFwd = false;  }

void CGui::btnRplPlay(WP)  // play / pause
{
	app->bRplPause = !app->bRplPause;
	UpdRplPlayBtn();
}

void CGui::UpdRplPlayBtn()
{
	String sign = app->bRplPause ? "|>" : "||";
	if (btRplPl)
		btRplPl->setCaption(sign);
}


//  List
void CGui::updReplaysList()
{
	if (!rplList)  return;
	//Ogre::Timer ti;

	rplList->removeAllItems();

	strlist li;
	PATHMANAGER::DirList(GetRplListDir(), li, "rpl");
	
	for (strlist::iterator i = li.begin(); i != li.end(); ++i)
	if (StringUtil::endsWith(*i, ".rpl"))
	{
		String s = *i;  s = StringUtil::replaceAll(s,".rpl","");
		String slow = s;  StringUtil::toLowerCase(slow);
		if (sRplFind == "" || strstr(slow.c_str(), sRplFind.c_str()) != 0)
		if (pSet->rpl_listview != 1 || StringUtil::startsWith(s,pSet->gui.track, false))
		{	String t = s;
			size_t p = t.find_first_of("_");
			if (p != string::npos)
				t = s.substr(0, p);
			rplList->addItem(gcom->GetSceneryColor(t) + s);
	}	}
	//LogO(String("::: Time ReplaysList: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}


//  Delete
void CGui::btnRplDelete(WP)
{
	String name = getRplName();  if (name.empty())  return;

	Message* message = Message::createMessageBox(
		"Message", TR("#{Replay} - #{RplDelete} ?"), name,
		MessageBoxStyle::IconQuest | MessageBoxStyle::Yes | MessageBoxStyle::No);
	message->eventMessageBoxResult += newDelegate(this, &CGui::msgRplDelete);
}
void CGui::msgRplDelete(Message* sender, MessageBoxStyle result)
{
	if (result != MessageBoxStyle::Yes)  return;
	String name = getRplName();  if (name.empty())  return;
	string file = GetRplListDir() +"/"+ name + ".rpl";

	if (fs::exists(file))
		fs::remove(file);
	updReplaysList();
}

//  Rename
void CGui::btnRplRename(WP)
{
	if (pSet->rpl_listghosts)  return;  // cant rename ghosts
	String name = getRplName();  if (name.empty())  return;
	string edit = edRplName->getCaption();

	if (name == edit)  // same name
	{	Message::createMessageBox(
			"Message", TR("#{Replay} - #{RplRename}"), TR("#{AlreadyExists}."),
			MessageBoxStyle::IconInfo | MessageBoxStyle::Ok);
		return;  }
	
	string file = PATHMANAGER::Replays() + "/" + name + ".rpl";
	string fileNew = PATHMANAGER::Replays() + "/" + edit + ".rpl";

	if (fs::exists(fileNew))
	{	Message::createMessageBox(
			"Message", TR("#{Replay} - #{RplRename}"), TR("#{AlreadyExists}."),
			MessageBoxStyle::IconInfo | MessageBoxStyle::Ok);
		return;  }

	if (fs::exists(file))
		fs::rename(file, fileNew);
	updReplaysList();
}



//  Game replay Tools
//--------------------------------------------------------------------------------
bool Replay::fixOldTrkName(string& s)
{
	if (s.length() <= 4)  return false;
	
	if (s[0]=='0' && s[1]=='W')
		s = s.substr(1);
	if (s[0]>='A' && s[0]<='Z' && (s[2]=='-' || s[3]=='-'))
	{
		if (s[0]=='J')  s= "Jng" +s.substr(1);  else
		if (s[0]=='D')  s= "Des" +s.substr(1);  else
		if (s[0]=='S')  s= "Sav" +s.substr(1);  else
		if (s[0]=='W')  s= "Wnt" +s.substr(1);  else
		if (s[0]=='F')  s= "For" +s.substr(1);  else
		if (s[0]=='E')  s= "Fin" +s.substr(1);  else
		if (s[0]=='I')  s= "Isl" +s.substr(1);  else
		if (s[0]=='M')  s= "Mud" +s.substr(1);  else
		if (s[0]=='A')  s= "Aus" +s.substr(1);  else
		if (s[0]=='G')  s= "Grc" +s.substr(1);  else
		if (s[0]=='C')  s= "Can" +s.substr(1);  else
		if (s[0]=='T')  s= "Atm" +s.substr(1);  else
		if (s[0]=='O')  s= "Mos" +s.substr(1);  else          
		if (s[0]=='V')  s= "Vlc" +s.substr(1);  else
		if (s[0]=='X')  s= "Uni" +s.substr(1);  else
		if (s[0]=='R')  s= "Mrs" +s.substr(1);  else
		if (s[0]=='Y')  s= "Cry" +s.substr(1);  else  return false;
		return true;
	}
	return false;
}

//  Rename old track names
void CGui::btnRenameOldTrk(WP)
{
	LogO("==------  Renaming old replays and ghosts");
	std::vector<string> pp;
	pp.push_back(PATHMANAGER::Replays());
	pp.push_back(PATHMANAGER::Ghosts() +"/easy");
	pp.push_back(PATHMANAGER::Ghosts() +"/normal");
	pp.push_back(PATHMANAGER::Records()+"/easy");
	pp.push_back(PATHMANAGER::Records()+"/normal");
	strlist li;
	for (int ip=0; ip < pp.size(); ++ip)
	{
		li.clear();
		string p = pp[ip];
		PATHMANAGER::DirList(p, li);
		LogO("PATH: "+p);
		for (strlist::iterator i = li.begin(); i != li.end(); ++i)
		{
			String s = *i, sn;
			if (s.length() > 3)
			if (s[0]>='A' && s[0]<='Z' && (s[2]=='-' || s[3]=='-'))
			{	sn = s;
				if (Replay::fixOldTrkName(sn))
				if (!fs::exists(p+"/"+s))       LogO(s+" to "+sn+"  source doesnt exist!");
				/**/else if (fs::exists(p+"/"+sn))  LogO(s+" to "+sn+"  destination already exists");
				else
				{	LogO(s+" to "+sn);
					fs::rename(p+"/"+s, p+"/"+sn);
			}	}
	}	}
	LogO("==------  Renaming End");
}

//  Convert to Replay2
void CGui::btnConvertAllRpl(WP)
{
	if (bConvertRpl)  return;
	bConvertRpl = true;

	txtConvert->setVisible(true);

	iConvCur = -1;  iConvAll = 1;
	mThrConvert = boost::thread(boost::bind(&CGui::ThreadConvert, boost::ref(*this)));
}

void CGui::ThreadConvert()
{
	LogO("====----  Converting old replays and ghosts");
	Ogre::Timer ti;
	
	std::vector<string> paths, files[3];
	//  paths
	paths.push_back(PATHMANAGER::Ghosts() +"/easy");
	paths.push_back(PATHMANAGER::Ghosts() +"/normal");
	paths.push_back(PATHMANAGER::Replays());

	iConvPathCur = 0;  iConvPathAll = paths.size();
	iConvFiles = 0;  totalConv = 0;  totalConvNew = 0;  totalConvCur = 0;

	//  List files  ------------
	strlist li;  int p;
	Replay2 rpl;
	for (p=0; p < iConvPathAll; ++p)
	{	iConvPathCur = p;

		const string& path = paths[p];
		li.clear();
		PATHMANAGER::DirList(path, li, "rpl");
		iConvCur = 0;  iConvAll = li.size();

		for (strlist::iterator i = li.begin(); i != li.end(); ++i)
		{
			if (app->mShutDown)  return;
			String file = *i, s = path +"/"+ file;

			//Replay2 rpl;
			rpl.LoadFile(s, true);  // header
			if (rpl.header.ver <= 10)  // old, not converted, 10 was last 2.5
			{
				++iConvFiles;
				files[p].push_back(file);
				boost::uintmax_t size = fs::file_size(s);
				totalConv += size;
			}
			++iConvCur;
		}
		LogO("PATH: "+path+" total size:   "+fToStr( float(totalConv)/1000000.f, 2,5)+" MiB");
		LogO("PATH: "+path+" total after:  "+fToStr( float(totalConvNew)/1000000.f, 2,5)+" MiB");
	}
	LogO(String("::: Time Convert get list: ") + fToStr(ti.getMilliseconds()/1000.f,1,4) + " s");
	LogO("====----  Converting Start");

	iConvCur = 0;  iConvAll = iConvFiles;


	LogO(String("FILES to convert: ") + toStr(iConvFiles));

	//  Convert  ------------
	for (p=0; p < iConvPathAll; ++p)
	{	iConvPathCur = p;

		const string& path = paths[p];
		iConvCur = 0;  iConvAll = files[p].size();

		while (iConvCur < iConvAll && !app->mShutDown)
		{
			const string s = path +"/"+ files[p][iConvCur];
			//Replay2 rpl;
			rpl.LoadFile(s);  // converts old
			boost::uintmax_t size = fs::file_size(s);  // for progress
			totalConvCur += size;
			std::time_t tim = fs::last_write_time(s);

			rpl.SaveFile(s);  // same name, no backup
			fs::last_write_time(s, tim);  // restore original date
			boost::uintmax_t sizeNew = fs::file_size(s);
			totalConvNew += sizeNew;
			++iConvCur;
		}
	}

	//  Results  ------------
	bConvertRpl = false;
	LogO("====----  Converting Results");
	LogO("  Sizes");
	LogO("  old:   "+ fToStr( float(totalConv)/1000000.f, 2,5) +" MiB");
	LogO("  new:  "+ fToStr( float(totalConvNew)/1000000.f, 2,5) +" MiB");
	if (totalConvCur!=totalConv || totalConv==0){} else
	LogO("  ratio:  "+ fToStr(100.f* float(totalConvNew)/float(totalConv), 2,5) +" %");

	LogO(String("::: Time Convert: ") + fToStr(ti.getMilliseconds()/1000.f,1,4) + " s");
	LogO("====----  Converting End");
}
