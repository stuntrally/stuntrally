#include "pch.h"
#include "../common/Defines.h"
#include "../../road/Road.h"
#include "../../vdrift/pathmanager.h"
#ifndef SR_EDITOR
	#include "../../vdrift/game.h"
	#include "../OgreGame.h"
	#include "../SplitScreen.h"
#else
	#include "../../editor/OgreApp.h"
#endif
#include "MultiList2.h"
#include <OgreRoot.h>
#include <OgreMaterialManager.h>
#include <OgreSceneManager.h>
#include <OgreTerrain.h>
#include <OgreRenderWindow.h>
#include "Gui_Def.h"
#include <boost/filesystem.hpp>
using namespace MyGUI;
using namespace Ogre;
using namespace std;

//  sort	 . . . . . . . . . . . . . . . . . . . . . . 
//-----------------------------------------------------------------------------------------------------------
/*  common sort code,  no info only by name  */
#define sArg  const TrkL& t2, const TrkL& t1
#define sortDef  bool t = false/*t1.test < t2.test/**/;  if (!t1.ti || !t2.ti)  return t1.name > t2.name || t;
const int allSortFunc = 15;

//  sorting functions for columns
/* 0  name    */  bool Sort0 (sArg){  sortDef  return t1.name  < t2.name   || t;  }
/* 1  n       */  bool Sort1 (sArg){  sortDef  return t1.ti->n < t2.ti->n  || t;  }
/* 2  scenery */  bool Sort2 (sArg){  sortDef  return t1.ti->scenery < t2.ti->scenery || t;  }
/* 3  crtver  */  bool Sort3 (sArg){  sortDef  return t1.ti->crtver < t2.ti->crtver  || t;  }
/* 4  diff    */  bool Sort4 (sArg){  sortDef  return t1.ti->diff   < t2.ti->diff    || t;  }
/* 5  rating  */  bool Sort5 (sArg){  sortDef  return t1.ti->rating < t2.ti->rating  || t;  }
/* 6  objects */  bool Sort6 (sArg){  sortDef  return t1.ti->objects < t2.ti->objects || t;  }
/* 7  fluids  */  bool Sort7 (sArg){  sortDef  return t1.ti->fluids < t2.ti->fluids  || t;  }
/* 8  bumps   */  bool Sort8 (sArg){  sortDef  return t1.ti->bumps  < t2.ti->bumps   || t;  }
/* 9  jumps   */  bool Sort9 (sArg){  sortDef  return t1.ti->jumps  < t2.ti->jumps   || t;  }
/* 10 loops   */  bool Sort10(sArg){  sortDef  return t1.ti->loops  < t2.ti->loops   || t;  }
/* 11 pipes   */  bool Sort11(sArg){  sortDef  return t1.ti->pipes  < t2.ti->pipes   || t;  }
/* 12 banked  */  bool Sort12(sArg){  sortDef  return t1.ti->banked < t2.ti->banked  || t;  }
/* 13 frenzy  */  bool Sort13(sArg){  sortDef  return t1.ti->frenzy < t2.ti->frenzy  || t;  }
/* 14 longn   */  bool Sort14(sArg){  sortDef  return t1.ti->longn  < t2.ti->longn   || t;  }

//  sorting functions array (to access by column index)
bool (*TrkSort[allSortFunc])(const TrkL& t1, const TrkL& t2) = {
	Sort0, Sort1, Sort2, Sort3, Sort4, Sort5, Sort6, Sort7, Sort8, Sort9, Sort10, Sort11, Sort12, Sort13, Sort14 };


//  done every list sort column change or find edit text change
//  fills gui track list
//-----------------------------------------------------------------------------------------------------------
void App::TrackListUpd(bool resetNotFound)
{
	if (trkList)
	{	trkList->removeAllItems();
		int ii = 0, si = -1;  bool bFound = false;

		//  sort
		int numFunc = min(allSortFunc-1, (int)trkList->mSortColumnIndex);
		std::list<TrkL> liTrk2 = liTrk;  // copy
		//liTrk2.sort(TrkSort[0]);
		liTrk2.sort(TrkSort[numFunc]);
		if (trkList->mSortUp)  liTrk2.reverse();
		
		//  original
		for (std::list<TrkL>::iterator i = liTrk2.begin(); i != liTrk2.end(); ++i)
		{
			String name = (*i).name, nlow = name;  StringUtil::toLowerCase(nlow);
			if (sTrkFind == "" || strstr(nlow.c_str(), sTrkFind.c_str()) != 0)
			{
				AddTrkL(name, 0, (*i).ti);
				if (!pSet->gui.track_user && name == pSet->gui.track)  {  si = ii;
					trkList->setIndexSelected(si);
					bFound = true;  bListTrackU = 0;  }
				ii++;
		}	}
		//  user
		for (strlist::iterator i = liTracksUser.begin(); i != liTracksUser.end(); ++i)
		{
			String name = *i, nlow = name;  StringUtil::toLowerCase(nlow);
			if (sTrkFind == "" || strstr(nlow.c_str(), sTrkFind.c_str()) != 0)
			{
				AddTrkL("*" + (*i) + "*", 1, 0);
				if (pSet->gui.track_user && name == pSet->gui.track)  {  si = ii;
					trkList->setIndexSelected(si);
					bFound = true;  bListTrackU = 1;  }
				ii++;
		}	}

		//  not found last track, set 1st  .. only 
		if (resetNotFound && !bFound && !liTracks.empty())
		{	pSet->gui.track = *liTracks.begin();  pSet->gui.track_user = 0;
			#ifdef SR_EDITOR
			UpdWndTitle();
			#endif
		}
		if (si > -1)  // center
			trkList->beginToItemAt(max(0, si-11));
	}
}

//  add track item to gui list
//-----------------------------------------------------------------------------------------------------------
String App::GetSceneryColor(String name)
{
	if (name.empty())  return "#707070";
	if (name.c_str()[0]=='*')  name = name.substr(1);

	String c = "#D0FFFF";  char ch = name.c_str()[0];
	switch (ch)  {
		case 'T':  c = (name.c_str()[1] != 'e') ? "#FFA020" : 
			(name.length() > 5 && name.c_str()[4] == 'C') ? "#A0C0D0" : "#A0A0A0";  break;  // Test,TestC
		case 'J':  c = "#50FF50";  break;  case 'S':  c = "#C0E080";  break;  case 'F':  c = "#A0C000";  break;
		case 'G':  c = "#B0FF00";  break;  case '0':  c = "#E8E8E8";  break;  case 'I':  c = "#FFFF80";  break;
		case 'A':  c = "#FFA080";  break;  case 'D':  c = "#F0F000";  break;  case 'C':  c = "#E0B090";  break;
		case 'V':  c = "#1E1E0E";  break;  case 'X':  c = "#8080D0";  break;  case 'M':  c = "#A0A000";  break;
		case 'O':  c = "#70F0B0";  break;  case 'E':  c = "#A0E080";  break;  case 'R':  c = "#A04840";  break;  }
	return c;
}

// track difficulties colors from value
const String App::clrsDiff[9] =  // difficulty
	{"#60C0FF", "#00FF00", "#60FF00", "#C0FF00", "#FFFF00", "#FFC000", "#FF6000", "#FF4040", "#B060B0"};
const String App::clrsRating[5] =  // rating
	{"#808080", "#606060", "#7090A0", "#60C8D8", "#E0F0FF"};
const String App::clrsLong[10] =  // long
	{"#E0D0D0", "#E8C0C0", "#F0B0B0", "#F8A0A0", "#FF9090", "#FF8080", "#F07070", "#F06060", "#E04040", "#D02020"};

void App::AddTrkL(std::string name, int user, const TrackInfo* ti)
{
	String c = GetSceneryColor(name);

	MultiList2* li = trkList;
	li->addItem(c+name, 0);

	if (!ti)  return;  //  details
	int l = li->getItemCount()-1;
	
	li->setSubItemNameAt(1,l, c+toStr(ti->n/10)+toStr(ti->n%10));
	li->setSubItemNameAt(2,l, c+ti->scenery);
	li->setSubItemNameAt(3,l, c+fToStr(ti->crtver,1,3));
	//list->setSubItemNameAt(4,l, ti->created);  list->setSubItemNameAt(5,l, ti->modified);
	#define toS(clr,v)  (v > 0) ? (String(clr)+"  "+toStr(v)) : " "
	li->setSubItemNameAt(4,l, toS(clrsDiff[ti->diff], ti->diff));
	li->setSubItemNameAt(5,l, toS(clrsRating[ti->rating], ti->rating));
	//todo: rateuser drivenlaps
	li->setSubItemNameAt(6,l, toS("#D070A0",ti->objects));
	li->setSubItemNameAt(7,l, toS("#80C0FF",ti->fluids));
	li->setSubItemNameAt(8,l, toS("#40FF00",ti->bumps));
	li->setSubItemNameAt(9,l, toS("#FFA030",ti->jumps));
	li->setSubItemNameAt(10,l,toS("#00FFFF",ti->loops));
	li->setSubItemNameAt(11,l,toS("#FFFF00",ti->pipes));
	li->setSubItemNameAt(12,l,toS("#C0C0C0",ti->banked));
	li->setSubItemNameAt(13,l,toS("#C080FF",ti->frenzy));
	li->setSubItemNameAt(14,l,toS(clrsLong[ti->longn], ti->longn));
}


//  Gui Init  [Track]  . . . . . . . . . . . . . . . . . . . 
//  column widths on tabs: tracks, champs, stages
const int wi = 26;
const int App::TcolW[32] = {150, 40, 80, 40, wi, wi, wi, wi, wi, wi, wi, wi, wi, wi, wi, 20};
#ifndef SR_EDITOR
const int App::TcolC[6] = {34, 17, 35, 40, 20};
const int App::ChColW[9] = {30, 180, 120, 50, 80, 80, 60, 40};
const int App::StColW[8] = {30, 180, 100, 90, 80, 70};
#endif

//  done once with init gui
//-----------------------------------------------------------------------------------------------------------
void App::GuiInitTrack()
{
	#ifdef SR_EDITOR
	TabItem* trktab = (TabItem*)mWndEdit->findWidget("TabTrack");
	#else
	TabItem* trktab = (TabItem*)mWndGame->findWidget("TabTrack");
	#endif
	MultiList2* li = trktab->createWidget<MultiList2>("MultiListBox",0,0,500,300, Align::Left | Align::VStretch);
	li->setColour(Colour(0.8,0.9,0.8));
	//li->setUserString("RelativeTo", "OptionsWnd");
	//*li->setAlpha(0.8);*/  li->setInheritsAlpha(false);
	
	trkList = li;  if (!li)  LogO("Error: No MListTracks in layout !");
   	trkList->eventListChangePosition += newDelegate(this, &App::listTrackChng);
   	trkList->setVisible(false);
	
	//  preview images
	imgPrv[0] = mGUI->findWidget<StaticImage>("TrackImg");
	imgTer[0] = mGUI->findWidget<StaticImage>("TrkTerImg");
	imgMini[0] = mGUI->findWidget<StaticImage>("TrackMap");

	//  stats text
	for (int i=0; i < StTrk; ++i) //!
		stTrk[0][i] = mGUI->findWidget<StaticText>("iv"+toStr(i+1), false);
	for (int i=0; i < InfTrk; ++i)
		infTrk[0][i] = mGUI->findWidget<StaticText>("ti"+toStr(i+1), false);
		
	Edt(edFind, "TrackFind", edTrkFind);
	#ifndef SR_EDITOR
	EditPtr ed;
	Edt(ed, "RplFind", edRplFind);
	#endif

	ButtonPtr btn;
	Btn("TrkView1", btnTrkView1);	Btn("TrkView2", btnTrkView2);
	imgTrkIco1 = mGUI->findWidget<StaticImage>("TrkView2icons1");
	imgTrkIco2 = mGUI->findWidget<StaticImage>("TrkView2icons2");
	
	li->removeAllColumns();  int c=0;
	li->addColumn("#E0FFE0"+TR("#{Name}"), TcolW[c++]);
	li->addColumn("#80FF80""N", TcolW[c++]);
	li->addColumn("#80FF80"+TR("#{Scenery}"), TcolW[c++]);
	li->addColumn("#80FF80""ver", TcolW[c++]);  // created  modified  author-

	li->addColumn("#C0D0FF""diff", TcolW[c++]);
	li->addColumn("#C0E0FF""*", TcolW[c++]);  // rateuser  drivenlaps ..
	li->addColumn("#FF80C0""o", TcolW[c++]);
	li->addColumn("#80C0FF""f", TcolW[c++]);
	li->addColumn("#40FF00""B", TcolW[c++]);
	li->addColumn("#FFA030""J", TcolW[c++]);
	li->addColumn("#00FFFF""L", TcolW[c++]);
	li->addColumn("#FFFF00""P", TcolW[c++]);
	li->addColumn("#C0C0C0""b", TcolW[c++]);
	li->addColumn("#C080FF""f", TcolW[c++]);
	li->addColumn("#FFA0A0""l", TcolW[c++]);
	li->addColumn(" ", TcolW[c++]);

	FillTrackLists();  //once

	trkList->mSortColumnIndex = pSet->tracks_sort;
	trkList->mSortUp = pSet->tracks_sortup;

    TrackListUpd(true);  //upd
	listTrackChng(trkList,0);

	ChangeTrackView();
}


//  events  . . . . . . . . . . . . . . . . . . . . . . . . . 
//-----------------------------------------------------------------------------------------------------------

//  list changed position
void App::listTrackChng(MultiList2* li, size_t pos)
{
	if (!li)  return;
	size_t i = li->getIndexSelected();  if (i==ITEM_NONE)  return;

	const UString& sl = li->getItemNameAt(i);  String s = sl, s1 = s;
	s = StringUtil::replaceAll(s, "*", "");
	bListTrackU = s1 != s ? 1 : 0;
	if (s[0] == '#')  s = s.substr(7);
	sListTrack = s;

#ifndef SR_EDITOR
	changeTrack();
#endif
#ifdef SR_EDITOR
	if (iLoadNext==0)
#endif
		ReadTrkStats();
}

//  find edit changed text
void App::edTrkFind(EditPtr ed)
{
	String s = ed->getCaption();
	if (s == "")
		sTrkFind = "";
	else
	{	sTrkFind = s;
		StringUtil::toLowerCase(sTrkFind);
	}
	TrackListUpd(false);
}

#ifndef SR_EDITOR
void App::edRplFind(EditPtr ed)
{
	String s = ed->getCaption();
	if (s == "")
		sRplFind = "";
	else
	{	sRplFind = s;
		StringUtil::toLowerCase(sRplFind);
	}
	updReplaysList();
}
#endif


//  view change
//-----------------------------------------------------------------------------------------------------------
void App::btnTrkView1(WP wp)
{
	pSet->tracks_view = 0;  ChangeTrackView();
}
void App::btnTrkView2(WP wp)
{
	pSet->tracks_view = 1;  ChangeTrackView();
}

void App::ChangeTrackView()
{
	bool full = pSet->tracks_view;

	if (!imgPrv[0])  return;
	imgPrv[0]->setVisible(!full);
	trkDesc[0]->setVisible(!full);
	imgTrkIco1->setVisible(full);
	imgTrkIco2->setVisible(full);

	updTrkListDim();  // change size, columns
}

//  adjust list size, columns
void App::updTrkListDim()
{
	//  tracks list  ----------
	if (!trkList)  return;
	bool full = pSet->tracks_view;

	int c, sum = 0, cnt = trkList->getColumnCount();
	for (c=0; c < cnt; ++c)  sum += TcolW[c];

	const IntCoord& wi = mWndOpts->getCoord();
	int sw = 0, xico1 = 0, xico2 = 0, wico = 0;

	for (c=0; c < cnt; ++c)
	{
		int w = c==cnt-1 ? 18 : (full || c==0 || c==cnt-1 ?
			float(TcolW[c]) / sum * 0.63/*width*/ * wi.width * 0.97/*frame*/ : 0);
		trkList->setColumnWidthAt(c, w);
		sw += w;
		if (c == 4)  wico = w;
		if (c < 4)  xico1 += w;
		if (c < 7)  xico2 += w;
	}

	int xt = 0.018*wi.width, yt = 0.06*wi.height, yico = yt - wico - 1;  //0.02*wi.height;
	trkList->setCoord(xt, yt, sw + 8/*frame*/, 0.70/*height*/*wi.height);
	imgTrkIco1->setCoord(xt + xico1+2, yico, 3*wico, wico);
	imgTrkIco2->setCoord(xt + xico2+2, yico, 8*wico, wico);
	trkList->setVisible(true);

	//  car list  ----------
	#ifndef SR_EDITOR
	sum = 0;  sw = 0;  cnt = carList->getColumnCount();
	for (c=0; c < cnt; ++c)  sum += TcolC[c];

	for (c=0; c < cnt; ++c)
	{
		int w = (c==cnt-1) ? 18 : (float(TcolC[c]) / sum * 0.21/*width*/ * wi.width * 0.97/*frame*/);
		carList->setColumnWidthAt(c, w);
		sw += w;
	}

	xt = 0.018*wi.width;  yt = 0.024*wi.height, yico = yt - wico - 1;  //0.02*wi.height;
	carList->setCoord(xt, yt, sw + 8/*frame*/, 0.41/*height*/*wi.height);
	#endif
	
	#ifndef SR_EDITOR
	if (panelNetTrack)  {
		TabItem* trkTab = mGUI->findWidget<TabItem>("TabTrack");
		const IntCoord& tc = trkTab->getCoord();
		panelNetTrack->setCoord(0,0, tc.width*0.66f, tc.height);  }
	#endif
}

#ifndef SR_EDITOR
///  champ list  ----------
void App::updChampListDim()
{
	if (!liChamps)  return;
	const IntCoord& wi = mWndGame->getCoord();

	int sum = 0, cnt = liChamps->getColumnCount(), sw = 0;
	for (int c=0; c < cnt; ++c)  sum += ChColW[c];
	for (int c=0; c < cnt; ++c)
	{
		int w = c==cnt-1 ? 18 : float(ChColW[c]) / sum * 0.72/*width*/ * wi.width * 0.97/*frame*/;
		liChamps->setColumnWidthAt(c, w);
		sw += w;
	}

	int xt = 0.038*wi.width, yt = 0.10*wi.height;  // pos
	liChamps->setCoord(xt, yt, sw + 8/*frame*/, 0.32/*height*/*wi.height);
	liChamps->setVisible(true);

	//  Stages
	if (!liStages)  return;

	sum = 0;  cnt = liStages->getColumnCount();  sw = 0;
	for (int c=0; c < cnt; ++c)  sum += StColW[c];  sum += 43;//-
	for (int c=0; c < cnt; ++c)
	{
		int w = c==cnt-1 ? 18 : float(StColW[c]) / sum * 0.57/*width*/ * wi.width * 0.97/*frame*/;
		liStages->setColumnWidthAt(c, w);
		sw += w;
	}

	liStages->setCoord(xt, yt, sw + 8/*frame*/, 0.50/*height*/*wi.height);
	liStages->setVisible(true);
}
#endif


//  done once to fill tracks list from dirs
//-----------------------------------------------------------------------------------------------------------
void App::FillTrackLists()
{
	liTracks.clear();  liTracksUser.clear();
	#ifdef SR_EDITOR
	std::string chkfile = "/scene.xml";
	#else
	std::string chkfile = "/track.txt";
	#endif

	PATHMANAGER::GetFolderIndex(pathTrk[0], liTracks);
	PATHMANAGER::GetFolderIndex(pathTrk[1], liTracksUser);  //name duplicates
	if (liTracks.size() == 0)
		LogO("Error: no tracks !!! in data/tracks/ crashing.");

	//  original
	strlist::iterator i;
	i = liTracks.begin();
	while (i != liTracks.end())
	{
		std::string s = pathTrk[0] + *i + chkfile;
		if (!boost::filesystem::exists(s))
			i = liTracks.erase(i);
		else  ++i;
	}
	//  user
	i = liTracksUser.begin();
	while (i != liTracksUser.end())
	{
		std::string s = pathTrk[1] + *i + chkfile;
		if (!boost::filesystem::exists(s))
			i = liTracksUser.erase(i);
		else  ++i;
	}

	//  get info for track name, from tracksXml
	liTrk.clear();
	for (strlist::iterator i = liTracks.begin(); i != liTracks.end(); ++i)
	{
		TrkL trl;  trl.name = *i;  trl.pA = this;
		trl.test = StringUtil::startsWith(trl.name,"test");

		int id = tracksXml.trkmap[*i];
		const TrackInfo* pTrk = id==0 ? 0 : &tracksXml.trks[id-1];
		trl.ti = pTrk;  // 0 if not in tracksXml
		liTrk.push_back(trl);
	}
}

///  _Tool_ write sceneryID
#ifdef SR_EDITOR
void App::ToolListSceneryID()
{
	LogO("ALL tracks ---------");

	std::list<TrkL> liTrk2 = liTrk;
	liTrk2.sort(Sort0);  liTrk2.reverse();

	//  foreach track
	for (std::list<TrkL>::iterator it = liTrk2.begin(); it != liTrk2.end(); ++it)
	{
		std::string trk = (*it).name, path = pathTrk[0] +"/"+ trk +"/";
		Scene sc;  sc.LoadXml(path +"scene.xml");

		std::ostringstream s;
		s << std::fixed << std::left << std::setw(18) << trk;
		s << " scID: " << std::setw(3) << sc.sceneryId;
		s << "  pitch " << fToStr(sc.ldPitch,0,2);
		s << "  yaw" << fToStr(sc.ldYaw,0,4);
		s << "  amb "<<fToStr(sc.lAmb.x, 2,4)<<" "<<fToStr(sc.lAmb.y, 2,4)<<" "<<fToStr(sc.lAmb.z, 2,4);
		s << "  diff "<<fToStr(sc.lDiff.x,2,4)<<" "<<fToStr(sc.lDiff.y,2,4)<<" "<<fToStr(sc.lDiff.z,2,4);
		s << "  spec "<<fToStr(sc.lSpec.x,2,4)<<" "<<fToStr(sc.lSpec.y,2,4)<<" "<<fToStr(sc.lSpec.z,2,4);
		LogO(s.str());
	}
	LogO("ALL tracks ---------");
}
#endif


///  . .  util tracks stats  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
//----------------------------------------------------------------------------------------------------------------

void App::ReadTrkStats()
{
	String sRd = PathListTrk() + "/road.xml";
	String sSc = PathListTrk() + "/scene.xml";

	Scene* sc = new Scene();  sc->LoadXml(sSc);  // fails to defaults
#ifndef SR_EDITOR  // game
	SplineRoad rd(pGame);  rd.LoadFile(sRd,false);  // load

	TIMER tim;  tim.Load(PATHMANAGER::Records()+"/"+ pSet->gui.sim_mode+"/"+ sListTrack+".txt", 0.f, pGame->error_output);
	tim.AddCar(sListCar);

	UpdGuiRdStats(&rd,sc, sListTrack, tim.GetBestLap(0, pSet->gui.trackreverse));
#else
	SplineRoad rd(this);  rd.LoadFile(sRd,false);  // load
	UpdGuiRdStats(&rd,sc, sListTrack, 0.f);
#endif
	delete sc;
}

#ifndef SR_EDITOR  // game
void App::ReadTrkStatsChamp(String track, bool reverse)
{
	String sRd = pathTrk[0] + track + "/road.xml";
	String sSc = pathTrk[0] + track + "/scene.xml";

	Scene* sc = new Scene();  sc->LoadXml(sSc);  // fails to defaults
	SplineRoad rd(pGame);  rd.LoadFile(sRd,false);  // load

	TIMER tim;  tim.Load(PATHMANAGER::Records()+"/"+ pSet->gui.sim_mode+"/"+ track+".txt", 0.f, pGame->error_output);
	tim.AddCar(sListCar);

	UpdGuiRdStats(&rd,sc, track, tim.GetBestLap(0, reverse), true);
}
#endif

void App::UpdGuiRdStats(const SplineRoad* rd, const Scene* sc, const String& sTrack, float timeCur, bool champ)
{
#ifndef SR_EDITOR  // game
	bool mph = pSet->show_mph;
#else
	bool mph = false;
#endif
	float m = mph ? 0.621371f : 1.f;
	string km = mph ? " mi" : " km";
	int ch = champ ? 1 : 0;
	
	//  road stats
	//---------------------------------------------------------------------------
	if (stTrk[ch][1])  stTrk[ch][1]->setCaption(fToStr(sc->td.fTerWorldSize*0.001f*m ,3,5)+km);
	if (!rd)  return;
	float len = rd->st.Length;
	if (stTrk[ch][0])  stTrk[ch][0]->setCaption(fToStr(len*0.001f*m ,3,5)+km);

	if (stTrk[ch][2])  stTrk[ch][2]->setCaption(fToStr(rd->st.WidthAvg ,1,3)+" m");
	if (stTrk[ch][3])  stTrk[ch][3]->setCaption(fToStr(rd->st.HeightDiff ,0,2)+" m");

	if (stTrk[ch][4])  stTrk[ch][4]->setCaption(fToStr(rd->st.OnTer ,0,2)+"%");
	if (stTrk[ch][5])  stTrk[ch][5]->setCaption(fToStr(rd->st.Pipes ,0,2)+"%");
					
	int id = tracksXml.trkmap[sTrack];
	for (int i=0; i < InfTrk; ++i)
		if (infTrk[ch][i])  infTrk[ch][i]->setCaption("");
	if (id > 0)
	{	const TrackInfo& ti = tracksXml.trks[id-1];
		#define str0(v)  ((v)==0 ? "" : toStr(v))
		if (infTrk[ch][0])  infTrk[ch][0]->setCaption(str0(ti.fluids));
		if (infTrk[ch][1])  infTrk[ch][1]->setCaption(str0(ti.bumps));		if (infTrk[ch][2])  infTrk[ch][2]->setCaption(str0(ti.jumps));
		if (infTrk[ch][3])  infTrk[ch][3]->setCaption(str0(ti.loops));		if (infTrk[ch][4])  infTrk[ch][4]->setCaption(str0(ti.pipes));
		if (infTrk[ch][5])  infTrk[ch][5]->setCaption(str0(ti.banked));		if (infTrk[ch][6])  infTrk[ch][6]->setCaption(str0(ti.frenzy));
		if (infTrk[ch][7])  infTrk[ch][7]->setCaption(clrsLong[ti.longn] + str0(ti.longn));
		if (infTrk[ch][8])  infTrk[ch][8]->setCaption(ti.diff==0   ? "" : (clrsDiff[ti.diff] + toStr(ti.diff)));
		if (infTrk[ch][9])  infTrk[ch][9]->setCaption(ti.rating==0 ? "" : (clrsRating[ti.rating] + toStr(ti.rating)));
		if (infTrk[ch][10]) infTrk[ch][10]->setCaption(str0(ti.objects));
		#ifndef SR_EDITOR
		if (txTrackAuthor)  txTrackAuthor->setCaption(ti.author=="CH" ? "CryHam" : ti.author);
		#endif
	}

#ifndef SR_EDITOR  // game
	//  best time, avg vel
	std::string unit = mph ? " mph" : " kmh";
	m = pSet->show_mph ? 2.23693629f : 3.6f;

	//  track time
	float carMul = GetCarTimeMul(pSet->gui.car[0], pSet->gui.sim_mode);
	float timeTrk = times.trks[sTrack];
	std::string speedTrk = fToStr(len / timeTrk * m, 0,3) + unit;
	float timeT = (/*place*/1 * carsXml.magic * timeTrk + timeTrk) / carMul;
	if (stTrk[ch][6])  stTrk[ch][6]->setCaption(GetTimeString(timeT));
	if (stTrk[ch][7])  stTrk[ch][7]->setCaption(speedTrk);

	if (timeCur < 0.1f)
	{
		if (stTrk[ch][8])  stTrk[ch][8]->setCaption(GetTimeString(0.f));
		if (stTrk[ch][9])  stTrk[ch][9]->setCaption("--");
		if (stTrk[ch][10])  stTrk[ch][10]->setCaption("--");
	}else
	{	//  car record
		std::string speed = fToStr(len / timeCur * m, 0,3) + unit;
		if (stTrk[ch][8])  stTrk[ch][8]->setCaption(GetTimeString(timeCur));
		if (stTrk[ch][9])  stTrk[ch][9]->setCaption(speed);
		//  points
		float points = 0.f;
		GetRacePos(timeCur, timeTrk, carMul, false, &points);
		if (stTrk[ch][10])  stTrk[ch][10]->setCaption(fToStr(points ,1,3));
	}
#else
	if (trkName)  //
		trkName->setCaption(sTrack.c_str());
#endif
	if (trkDesc[ch])  // desc
		trkDesc[ch]->setCaption(rd->sTxtDesc.c_str());

	
	//  preview images
	//---------------------------------------------------------------------------
	ResourceGroupManager& resMgr = ResourceGroupManager::getSingleton();
	Ogre::TextureManager& texMgr = Ogre::TextureManager::getSingleton();

	String path = PathListTrkPrv(-1, sTrack), s, sGrp = "TrkPrv"+toStr(ch);
	resMgr.addResourceLocation(path, "FileSystem", sGrp);  // add for this track
	resMgr.unloadResourceGroup(sGrp);
	resMgr.initialiseResourceGroup(sGrp);

	if (imgPrv[ch])  // track view, preview shot
	{	try
		{	s = "view.jpg";
			texMgr.load(path+s, sGrp, TEX_TYPE_2D, MIP_UNLIMITED);  // need to load it first
			imgPrv[ch]->setImageTexture(s);  // just for dim, doesnt set texture
			imgPrv[ch]->_setTextureName(path+s);  imgPrv[ch]->setVisible(ch == 0 && pSet->tracks_view == 0 || ch == 1);
			//texMgr.unload(path+s);
		} catch(...) {  imgPrv[ch]->setVisible(false);  }  // hide if not found
	}
	if (imgTer[ch])  // terrain background
	{	try
		{	s = "terrain.jpg";
			texMgr.load(path+s, sGrp, TEX_TYPE_2D, MIP_UNLIMITED);
			imgTer[ch]->setImageTexture(s);
			imgTer[ch]->_setTextureName(path+s);  imgTer[ch]->setVisible(true);
		} catch(...) {  imgTer[ch]->setVisible(false);  }
	}
	if (imgMini[ch])  // road alpha
	{	try
		{	s = "road.png";
			texMgr.load(path+s, sGrp, TEX_TYPE_2D, MIP_UNLIMITED);
			imgMini[ch]->setImageTexture(s);
			imgMini[ch]->_setTextureName(path+s);  imgMini[ch]->setVisible(true);
		} catch(...) {  imgMini[ch]->setVisible(false);  }
	}
	resMgr.removeResourceLocation(path, sGrp);
}
