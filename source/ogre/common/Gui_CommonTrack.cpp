#include "pch.h"
#include "../Defines.h"
#include "../../road/Road.h"
#include "../../vdrift/pathmanager.h"
#ifndef ROAD_EDITOR
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


//  sort	 . . . . . . . . . . . . . . . . . . . . . . 
//-----------------------------------------------------------------------------------------------------------
/*  common sort code,  no info only by name  */
#define sArg  const TrkL& t2, const TrkL& t1
#define sortDef  bool t = false/*t1.test < t2.test/**/;  if (!t1.ti || !t2.ti)  return t1.name < t2.name || t;
const int allSortFunc = 14;

//  sorting functions for columns
/* 0  name    */  bool Sort0 (sArg){  sortDef  return t1.name  < t2.name   || t;  }
/* 1  n       */  bool Sort1 (sArg){  sortDef  return t1.ti->n < t2.ti->n  || t;  }
/* 2  scenery */  bool Sort2 (sArg){  sortDef  return t1.ti->scenery < t2.ti->scenery  || t;  }
/* 3  crtver  */  bool Sort3 (sArg){  sortDef  return t1.ti->crtver < t2.ti->crtver  || t;  }
/* 4  diff    */  bool Sort4 (sArg){  sortDef  return t1.ti->diff   < t2.ti->diff    || t;  }
/* 5  rating  */  bool Sort5 (sArg){  sortDef  return t1.ti->rating < t2.ti->rating  || t;  }
/* 6  fluids  */  bool Sort6 (sArg){  sortDef  return t1.ti->fluids < t2.ti->fluids  || t;  }
/* 7  bumps   */  bool Sort7 (sArg){  sortDef  return t1.ti->bumps  < t2.ti->bumps   || t;  }
/* 8  jumps   */  bool Sort8 (sArg){  sortDef  return t1.ti->jumps  < t2.ti->jumps   || t;  }
/* 9  loops   */  bool Sort9 (sArg){  sortDef  return t1.ti->loops  < t2.ti->loops   || t;  }
/* 10 pipes   */  bool Sort10(sArg){  sortDef  return t1.ti->pipes  < t2.ti->pipes   || t;  }
/* 11 banked  */  bool Sort11(sArg){  sortDef  return t1.ti->banked < t2.ti->banked  || t;  }
/* 12 frenzy  */  bool Sort12(sArg){  sortDef  return t1.ti->frenzy < t2.ti->frenzy  || t;  }
/* 13 longn   */  bool Sort13(sArg){  sortDef  return t1.ti->longn  < t2.ti->longn   || t;  }

//  sorting functions array (to access by column index)
bool (*TrkSort[allSortFunc])(const TrkL& t1, const TrkL& t2) = {
	Sort0, Sort1, Sort2, Sort3, Sort4, Sort5, Sort6, Sort7, Sort8, Sort9, Sort10, Sort11, Sort12, Sort13 };


//  done every list sort column change or find edit text change
//  fills gui track list
//-----------------------------------------------------------------------------------------------------------
void App::TrackListUpd(bool resetNotFound)
{
	if (trkMList)
	{	trkMList->removeAllItems();
		int ii = 0, si = -1;  bool bFound = false;

		//  sort
		int numFunc = min(allSortFunc-1, (int)trkMList->mSortColumnIndex);
		std::list<TrkL> liTrk2 = liTrk;  // copy
		//liTrk2.sort(TrkSort[0]);
		liTrk2.sort(TrkSort[numFunc]);
		if (trkMList->mSortUp)  liTrk2.reverse();
		
		//  original
		for (std::list<TrkL>::iterator i = liTrk2.begin(); i != liTrk2.end(); ++i)
		{
			String name = (*i).name, nlow = name;  StringUtil::toLowerCase(nlow);
			if (sTrkFind == "" || strstr(nlow.c_str(), sTrkFind.c_str()) != 0)
			{
				AddTrkL(name, 0, (*i).ti);
				if (!pSet->gui.track_user && name == pSet->gui.track)  {  si = ii;
					trkMList->setIndexSelected(si);
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
					trkMList->setIndexSelected(si);
					bFound = true;  bListTrackU = 1;  }
				ii++;
		}	}

		//  not found last track, set 1st  .. only 
		if (resetNotFound && !bFound && !liTracks.empty())
		{	pSet->gui.track = *liTracks.begin();  pSet->gui.track_user = 0;
			#ifdef ROAD_EDITOR
			UpdWndTitle();
			#endif
		}
		if (si > -1)  // center
			trkMList->beginToItemAt(max(0, si-11));
	}
}

//  add track item to gui list
//-----------------------------------------------------------------------------------------------------------
void App::AddTrkL(std::string name, int user, const TrackInfo* ti)
{
	String c = "#FFFFFF";  char ch = name.c_str()[0];
	switch (ch)  {
		case '0':  c = "#E0E0E0";  break;   case 'I':  c = "#FFFF80";  break;
		case 'A':  c = "#FFA080";  break;   case 'J':  c = "#50FF50";  break;
		case 'D':  c = "#F0F000";  break;   case 'M':  c = "#A0A000";  break;
		case 'F':  c = "#A0D000";  break;   case 'S':  c = "#D0FF00";  break;
		case 'G':  c = "#C0FF00";  break;   case 'T':  c = "#A0A0A0";  break;
		case 'V':  c = "#202008";  break;   case 'X':  c = "#5858C0";  break;  }

	MultiList2* li = trkMList;
	li->addItem(name, 0);

	if (!ti)  return;  //  details
	int l = li->getItemCount()-1;
	
	li->setSubItemNameAt(1,l, c+toStr(ti->n/10)+toStr(ti->n%10));  li->setSubItemNameAt(2,l, c+ti->scenery);
	li->setSubItemNameAt(3,l, c+toStr(ti->crtver));
	//list->setSubItemNameAt(4,l, ti->created);  list->setSubItemNameAt(5,l, ti->modified);
	#define toS(clr,v)  (v > 0) ? (clr "  "+toStr(v)) : " "
	li->setSubItemNameAt(4,l, toS("#C0D0FF",ti->diff));   li->setSubItemNameAt(5,l, toS("#C0E0FF",ti->rating));  //rateuser="0" drivenlaps="0"
	li->setSubItemNameAt(6,l, toS("#80C0FF",ti->fluids));
	li->setSubItemNameAt(7,l, toS("#40FF00",ti->bumps));  li->setSubItemNameAt(8,l, toS("#FFA030",ti->jumps));
	li->setSubItemNameAt(9,l, toS("#00FFFF",ti->loops));  li->setSubItemNameAt(10,l, toS("#FFFF00",ti->pipes));
	li->setSubItemNameAt(11,l,toS("#C0C0C0",ti->banked)); li->setSubItemNameAt(12,l,toS("#C080FF",ti->frenzy));
	li->setSubItemNameAt(13,l,toS("#FFA0A0",ti->longn));
}


//  Gui Init  [Track]  . . . . . . . . . . . . . . . . . . . 
const int wi = 32;  const int App::TcolW[32] = {
	150, 32, 80, 40, wi, wi, wi, wi, wi, wi, wi, wi, wi, wi, 20};

//  done once with init gui
//-----------------------------------------------------------------------------------------------------------
void App::GuiInitTrack()
{
	//  Tracks detailed list
	//MultiListPtr li = mGUI->findWidget<MultiList>("MListTracks");
	MyGUI::FactoryManager::getInstance().registerFactory<MultiList2>("Widget");
	//MyGUI::FactoryManager::getInstance().unregisterFactory<MultiList2>("Widget");

	#ifdef ROAD_EDITOR
	TabItem* trktab = (TabItem*)mWndOpts->findWidget("TabTrack");
	#else
	TabItem* trktab = (TabItem*)mWndGame->findWidget("TabTrack");
	#endif
	MultiList2* li = trktab->createWidget<MultiList2>("MultiListBox",0,0,600,300, Align::Left | Align::VStretch);
	//li->setUserString("RelativeTo", "OptionsWnd");
	//*li->setAlpha(0.8);*/  li->setInheritsAlpha(false);
	
	trkMList = li;  if (!li)  LogO("Error: No MListTracks in layout !");
   	trkMList->eventListChangePosition += newDelegate(this, &App::listTrackChng);
   	//..trkMList->eventListSelectAccept += newDelegate(this, &App::btnNewGameStart);
   	trkMList->setVisible(false);
	
	//  preview images
	imgPrv = mGUI->findWidget<StaticImage>("TrackImg");
	imgTer = mGUI->findWidget<StaticImage>("TrkTerImg");
	imgMini = mGUI->findWidget<StaticImage>("TrackMap");
	//  stats text
	for (int i=0; i < StTrk; ++i) //!
		stTrk[i] = mGUI->findWidget<StaticText>("iv"+toStr(i+1), false);
		
	for (int i=0; i < InfTrk; ++i)
		infTrk[i] = mGUI->findWidget<StaticText>("ti"+toStr(i+1), false);
		
	Edt(edFind, "TrackFind", edTrkFind);

	ButtonPtr btn;
	Btn("TrkView1", btnTrkView1);	Btn("TrkView2", btnTrkView2);
	imgTrkIco1 = mGUI->findWidget<StaticImage>("TrkView2icons1");
	imgTrkIco2 = mGUI->findWidget<StaticImage>("TrkView2icons2");
	
	li->removeAllColumns();  int c=0;
	li->addColumn("Name", TcolW[c++]);  li->addColumn("N", TcolW[c++]);
	  li->addColumn("Scenery", TcolW[c++]);
	li->addColumn("ver", TcolW[c++]);  // created  modified  author-
	li->addColumn("diff", TcolW[c++]);  li->addColumn("*", TcolW[c++]);
	// rateuser  drivenlaps ..
	li->addColumn("f", TcolW[c++]);
	li->addColumn("B", TcolW[c++]);  li->addColumn("J", TcolW[c++]);
	li->addColumn("L", TcolW[c++]);  li->addColumn("P", TcolW[c++]);
	li->addColumn("b", TcolW[c++]);  li->addColumn("f", TcolW[c++]);
	li->addColumn("l", TcolW[c++]);  li->addColumn(" ", TcolW[c++]);

	FillTrackLists();  //once

	trkMList->mSortColumnIndex = pSet->tracks_sort;  // from set
	trkMList->mSortUp = pSet->tracks_sortup;

    TrackListUpd(true);  //upd
	listTrackChng(trkMList,0);

	ChangeTrackView(pSet->tracks_view);
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
	sListTrack = s;
	bListTrackU = s1 != s ? 1 : 0;

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


//  view change
//-----------------------------------------------------------------------------------------------------------
void App::btnTrkView1(WP wp)
{
	pSet->tracks_view = 0;  ChangeTrackView(pSet->tracks_view);
}
void App::btnTrkView2(WP wp)
{
	pSet->tracks_view = 1;  ChangeTrackView(pSet->tracks_view);
}

void App::ChangeTrackView(bool full)
{
	imgPrv->setVisible(!full);
	trkDesc->setVisible(!full);
	imgTrkIco1->setVisible(full);
	imgTrkIco2->setVisible(full);

	updTrkListDim();
}

void App::updTrkListDim()
{
	if (!trkMList)  return;
	const IntCoord& wi = mWndOpts->getCoord();
	bool full = pSet->tracks_view;

	int sum = 0, cnt = trkMList->getColumnCount();
	for (int c=0; c < cnt; ++c)  sum += TcolW[c];
	
	int sw = 0, xico1 = 0, xico2 = 0, wico = 0;
	for (int c=0; c < cnt; ++c)
	{
		int w = c==cnt-1 ? 18 : (full || c==0 || c==cnt-1 ?
			float(TcolW[c]) / sum * 0.63/*width*/ * wi.width * 0.97/*frame*/ : 0);
		trkMList->setColumnWidthAt(c, w);
		sw += w;
		if (c == 4)  wico = w;
		if (c < 4)  xico1 += w;
		if (c < 6)  xico2 += w;
	}

	int xt = 0.018*wi.width, yt = 0.052*wi.height, yico = yt - wico - 1;  //0.02*wi.height;
	trkMList->setCoord(xt, yt, sw + 8/*frame*/, 0.70/*height*/*wi.height);
	imgTrkIco1->setCoord(xt + xico1+2, yico, 2*wico, wico);
	imgTrkIco2->setCoord(xt + xico2+2, yico, 8*wico, wico);
	trkMList->setVisible(true);
	
	#ifndef ROAD_EDITOR
	if (panelNetTrack)  {
		TabItem* trkTab = mGUI->findWidget<TabItem>("TabTrack");
		const IntCoord& tc = trkTab->getCoord();
		panelNetTrack->setCoord(0,0,tc.width*0.66f,tc.height);  }
	#endif
}


//  done once to fill tracks list from dirs
//-----------------------------------------------------------------------------------------------------------
void App::FillTrackLists()
{
	liTracks.clear();  liTracksUser.clear();
	#ifdef ROAD_EDITOR
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
		trl.test = Ogre::StringUtil::startsWith(trl.name,"Test");

		int id = tracksXml.trkmap[*i];
		const TrackInfo* pTrk = id==0 ? 0 : &tracksXml.trks[id-1];
		trl.ti = pTrk;  // 0 if not in tracksXml
		liTrk.push_back(trl);
	}
}


///  . .  util tracks stats  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
//----------------------------------------------------------------------------------------------------------------

void App::ReadTrkStats()
{
	String sRd = PathListTrk() + "/road.xml";
	String sSc = PathListTrk() + "/scene.xml";

	Scene sc;  sc.LoadXml(sSc);  // fails to defaults
#ifndef ROAD_EDITOR  // game
	SplineRoad rd(pGame);  rd.LoadFile(sRd,false);  // load
	TIMER tim;  tim.Load(PATHMANAGER::GetTrackRecordsPath()+"/"+sListTrack+".txt", 0.f, pGame->error_output);
	tim.AddCar(sListCar);  tim.SetPlayerCarID(0);
	UpdGuiRdStats(&rd,sc, tim.GetBestLap(pSet->gui.trackreverse));
#else
	SplineRoad rd;  rd.LoadFile(sRd,false);  // load
	UpdGuiRdStats(&rd,sc, 0.f);
#endif
}

void App::UpdGuiRdStats(const SplineRoad* rd, const Scene& sc, float time)
{
	//  won't refresh if same-...  road disappears if not found...
	if (imgPrv)  imgPrv->setImageTexture(sListTrack+".jpg");
	if (imgTer)  imgTer->setImageTexture(sListTrack+"_ter.jpg");
	if (imgMini)  imgMini->setImageTexture(sListTrack+"_mini.png");


	Fmt(s, "%5.3f km", sc.td.fTerWorldSize / 1000.f);	if (stTrk[1])  stTrk[1]->setCaption(s);
	if (!rd)  return;
	Fmt(s, "%5.3f km", rd->st.Length / 1000.f);			if (stTrk[0])  stTrk[0]->setCaption(s);

	Fmt(s, "%4.2f m", rd->st.WidthAvg);		if (stTrk[2])  stTrk[2]->setCaption(s);
	Fmt(s, "%3.1f m", rd->st.HeightDiff);	if (stTrk[3])  stTrk[3]->setCaption(s);

	Fmt(s, "%3.1f%%", rd->st.OnTer);	if (stTrk[4])  stTrk[4]->setCaption(s);
	Fmt(s, "%3.1f%%", rd->st.Pipes);	if (stTrk[5])  stTrk[5]->setCaption(s);
					
	//Fmt(s, "%4.2f%%", rd->st.Yaw);	if (stTrk[6])  stTrk[6]->setCaption(s);
	//Fmt(s, "%4.2f%%", rd->st.Pitch);	if (stTrk[7])  stTrk[7]->setCaption(s);
	//Fmt(s, "%4.2f%%", rd->st.Roll);	if (stTrk[8])  stTrk[8]->setCaption(s);
	
	int id = tracksXml.trkmap[sListTrack];
	for (int i=0; i < InfTrk; ++i)
		if (infTrk[i])  infTrk[i]->setCaption("");
	if (id > 0)
	{	const TrackInfo& ti = tracksXml.trks[id-1];
		#define str0(v)  ((v)==0 ? "" : toStr(v))
		if (infTrk[0])  infTrk[0]->setCaption(str0(ti.fluids));
		if (infTrk[1])  infTrk[1]->setCaption(str0(ti.bumps));		if (infTrk[2])  infTrk[2]->setCaption(str0(ti.jumps));
		if (infTrk[3])  infTrk[3]->setCaption(str0(ti.loops));		if (infTrk[4])  infTrk[4]->setCaption(str0(ti.pipes));
		if (infTrk[5])  infTrk[5]->setCaption(str0(ti.banked));		if (infTrk[6])  infTrk[6]->setCaption(str0(ti.frenzy));
		if (infTrk[7])  infTrk[7]->setCaption(str0(ti.longn));
		if (infTrk[8])  infTrk[8]->setCaption(toStr(ti.diff));		if (infTrk[9])  infTrk[9]->setCaption(toStr(ti.rating));
	}

#ifndef ROAD_EDITOR  // game
	//  best time, avg vel,
	if (time < 0.1f)
	{	Fmt(s, "%s", GetTimeString(0.f).c_str());	if (stTrk[6])  stTrk[6]->setCaption(s);
		if (pSet->show_mph)	Fmt(s, "0 mph");
		else				Fmt(s, "0 km/h");		if (stTrk[7])  stTrk[7]->setCaption(s);
	}else
	{	Fmt(s, "%s", GetTimeString(time).c_str());	if (stTrk[6])  stTrk[6]->setCaption(s);
		if (pSet->show_mph)	Fmt(s, "%4.1f mph", rd->st.Length / time * 2.23693629f);
		else				Fmt(s, "%4.1f km/h", rd->st.Length / time * 3.6f);
		if (stTrk[7])  stTrk[7]->setCaption(s);
		//Fmt(s, "%4.2f%%", rd->st.Pitch);	if (stTrk[8])  stTrk[8]->setCaption(s);
	}
#else
	if (trkName)  //?.
		trkName->setCaption(sListTrack.c_str());
#endif
	if (trkDesc)  // desc
		trkDesc->setCaption(rd->sTxtDesc.c_str());
}
