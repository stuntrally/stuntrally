#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "../../road/Road.h"
#include "../../vdrift/pathmanager.h"
#include "data/SceneXml.h"
#include "data/TracksXml.h"
#include "data/CData.h"
#ifndef SR_EDITOR
	#include "../../vdrift/game.h"
	#include "../CGame.h"
	#include "../CHud.h"
	#include "../CGui.h"
	#include "../SplitScreen.h"
#else
	#include "../../editor/CApp.h"
	#include "../../editor/CGui.h"
	#include "../../editor/settings.h"
#endif
#include "MultiList2.h"
#include <OgreRoot.h>
#include <OgreMaterialManager.h>
#include <OgreSceneManager.h>
#include <OgreTerrain.h>
#include <OgreRenderWindow.h>
#include <MyGUI.h>
//#include <MyGUI_Delegate.h>
//#include <MyGUI_Widget.h>
//#include <MyGUI_EditBox.h>
//#include <MyGUI_ImageBox.h>
//#include <MyGUI_Gui.h>
//#include <MyGUI_Window.h>
//#include <MyGUI_TabItem.h>
using namespace MyGUI;
using namespace Ogre;
using namespace std;


///  * * * *  CONST  * * * *
//  add track item to gui list
//-----------------------------------------------------------------------------------------------------------
String CGuiCom::GetSceneryColor(String name)
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
const String CGuiCom::clrsDiff[9] =  // difficulty
	{"#60C0FF", "#00FF00", "#60FF00", "#C0FF00", "#FFFF00", "#FFC000", "#FF6000", "#FF4040", "#B060B0"};
const String CGuiCom::clrsRating[6] =  // rating
	{"#808080", "#606060", "#7090A0", "#60C8D8", "#A0D0F0", "#E0F0FF"};
const String CGuiCom::clrsLong[10] =  // long
	{"#E0D0D0", "#E8C0C0", "#F0B0B0", "#F8A0A0", "#FF9090", "#FF8080", "#F07070", "#F06060", "#E04040", "#D02020"};

void CGuiCom::AddTrkL(std::string name, int user, const TrackInfo* ti)
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

//  * * * *  CONST  * * * *
//  column widths in MultiList2
const int wi = 26;  // track detailed
const int CGuiCom::colTrk[32] = {150, 40, 80, 40, wi, wi, wi, wi, wi, wi, wi, wi, wi, wi, wi, 20};
#ifndef SR_EDITOR
const int CGui::colCar[16] = {34, 17, 35, 40, 20};  // car
const int CGui::colCh [16] = {30, 180, 120, 50, 80, 80, 60, 40};  // champs
const int CGui::colChL[16] = {36, 180, 90, 100, 50, 60, 60, 60, 50};  // challs
const int CGui::colSt [16] = {30, 170, 100, 90, 50, 80, 70};  // stages
#endif


//  Gui Init [Track] once
//-----------------------------------------------------------------------------------------------------------
void CGuiCom::GuiInitTrack()
{
	#ifdef SR_EDITOR
	TabItem* trktab = (TabItem*)app->mWndEdit->findWidget("TabTrack");
	#else
	TabItem* trktab = (TabItem*)app->mWndGame->findWidget("TabTrack");
	#endif
	MultiList2* li = trktab->createWidget<MultiList2>("MultiListBox",0,0,500,300, Align::Left | Align::VStretch);
	li->setColour(Colour(0.8,0.9,0.8));
	//li->setUserString("RelativeTo", "OptionsWnd");
	//*li->setAlpha(0.8);*/  li->setInheritsAlpha(false);
	
	trkList = li;  if (!li)  LogO("Error: No MListTracks in layout !");
   	trkList->eventListChangePosition += newDelegate(this, &CGuiCom::listTrackChng);
   	trkList->setVisible(false);
	
	//  preview images
	imgPrv[0] = fImg("TrackImg");
	imgTer[0] = fImg("TrkTerImg");
	imgMini[0] = fImg("TrackMap");

	//  stats text
	for (int i=0; i < StTrk; ++i)
		stTrk[0][i] = fTxt("iv"+toStr(i));

	#ifdef SR_EDITOR
	for (int i=0; i < 8; ++i)
		infTrk[0][i] = fTxt("ti"+toStr(i));
	#else
	for (int i=0; i < InfTrk; ++i)
		infTrk[0][i] = fTxt("ti"+toStr(i));
	#endif
		
	//EdC(TrkFind);
	#ifndef SR_EDITOR
	EditPtr ed;
	Ed(RplFind, edRplFind);//-
	#endif

	ButtonPtr btn;
	BtnC("TrkView1", btnTrkView1);
	BtnC("TrkView2", btnTrkView2);
	imgTrkIco1 = fImg("TrkView2icons1");
	imgTrkIco2 = fImg("TrkView2icons2");
	
	li->removeAllColumns();  int c=0;
	li->addColumn("#E0FFE0"+TR("#{Name}"), colTrk[c++]);
	li->addColumn("#80FF80""N", colTrk[c++]);
	li->addColumn("#80FF80"+TR("#{Scenery}"), colTrk[c++]);
	li->addColumn("#80FF80""ver", colTrk[c++]);  // created- modified-

	li->addColumn("#C0D0FF""diff", colTrk[c++]);  // rateuser  drivenlaps ..
	li->addColumn("#C0E0FF""*", colTrk[c++]);   // rating
	li->addColumn("#FF80C0""o", colTrk[c++]);   // objects
	li->addColumn("#80C0FF""f", colTrk[c++]);   // fluids
	li->addColumn("#40FF00""B", colTrk[c++]);   // Bumps
	li->addColumn("#FFA030""J", colTrk[c++]);   // Jumps
	li->addColumn("#00FFFF""L", colTrk[c++]);   // Loops
	li->addColumn("#FFFF00""P", colTrk[c++]);   // Pipes
	li->addColumn("#C0C0C0""b", colTrk[c++]);   // banked
	li->addColumn("#C080FF""f", colTrk[c++]);   // frenzy
	li->addColumn("#FFA0A0""l", colTrk[c++]);	// longn
	li->addColumn(" ", colTrk[c++]);

	FillTrackLists();  //once

	trkList->mSortColumnIndex = pSet->tracks_sort;
	trkList->mSortUp = pSet->tracks_sortup;

    TrackListUpd(true);  //upd
	listTrackChng(trkList,0);

	ChangeTrackView();
}


//  done once to fill tracks list from dirs
//-----------------------------------------------------------------------------------------------------------
void CGuiCom::FillTrackLists()
{
	liTracks.clear();  liTracksUser.clear();
	#ifdef SR_EDITOR
	std::string chkfile = "/scene.xml";
	#else
	std::string chkfile = "/track.txt";
	#endif

	PATHMANAGER::DirList(pathTrk[0], liTracks);
	PATHMANAGER::DirList(pathTrk[1], liTracksUser);  //name duplicates
	if (liTracks.size() == 0)
		LogO("Error: no tracks !!! in data/tracks/ crashing.");

	//  original
	strlist::iterator i;
	i = liTracks.begin();
	while (i != liTracks.end())
	{
		std::string s = pathTrk[0] + *i + chkfile;
		if (!PATHMANAGER::FileExists(s))
			i = liTracks.erase(i);
		else  ++i;
	}
	//  user
	i = liTracksUser.begin();
	while (i != liTracksUser.end())
	{
		std::string s = pathTrk[1] + *i + chkfile;
		if (!PATHMANAGER::FileExists(s))
			i = liTracksUser.erase(i);
		else  ++i;
	}

	//  get info for track name, from data->tracks
	liTrk.clear();
	for (strlist::iterator i = liTracks.begin(); i != liTracks.end(); ++i)
	{
		TrkL trl;  trl.name = *i;  //trl.pA = this;
		trl.test = StringUtil::startsWith(trl.name,"test");

		int id = app->data->tracks->trkmap[*i];
		const TrackInfo* pTrk = id==0 ? 0 : &app->data->tracks->trks[id-1];
		trl.ti = pTrk;  // 0 if not in data->tracks
		liTrk.push_back(trl);
	}
}


///  . .  util tracks stats  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
//----------------------------------------------------------------------------------------------------------------

void CGuiCom::ReadTrkStats()
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
	SplineRoad rd(app);  rd.LoadFile(sRd,false);  // load
	UpdGuiRdStats(&rd,sc, sListTrack, 0.f);
#endif
	delete sc;
}

#ifndef SR_EDITOR  // game
void CGuiCom::ReadTrkStatsChamp(String track, bool reverse)
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


String CGuiCom::TrkDir() {
	int u = pSet->gui.track_user ? 1 : 0;		return pathTrk[u] + pSet->gui.track + "/";  }

String CGuiCom::PathListTrk(int user) {
	int u = user == -1 ? bListTrackU : user;	return pathTrk[u] + sListTrack;  }
	
String CGuiCom::PathListTrkPrv(int user, String track) {
	int u = user == -1 ? bListTrackU : user;	return pathTrk[u] + track + "/preview/";  }


bool CGuiCom::TrackExists(String name/*, bool user*/)
{
	// ignore letters case..
	for (strlist::const_iterator it = liTracks.begin(); it != liTracks.end(); ++it)
		if (*it == name)  return true;
	for (strlist::const_iterator it = liTracksUser.begin(); it != liTracksUser.end(); ++it)
		if (*it == name)  return true;
	return false;
}


void CGuiCom::UpdGuiRdStats(const SplineRoad* rd, const Scene* sc, const String& sTrack, float timeCur, bool champ)
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
					
	int id = app->data->tracks->trkmap[sTrack];
	for (int i=0; i < InfTrk; ++i)
		if (infTrk[ch][i])  infTrk[ch][i]->setCaption("");
	if (id > 0)
	{	const TrackInfo& ti = app->data->tracks->trks[id-1];
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
	float carMul = app->GetCarTimeMul(pSet->gui.car[0], pSet->gui.sim_mode);
	float timeTrk = app->data->tracks->times[sTrack];
	std::string speedTrk = fToStr(len / timeTrk * m, 0,3) + unit;
	float timeT = (/*place*/1 * data->cars->magic * timeTrk + timeTrk) / carMul;
	if (stTrk[ch][6])  stTrk[ch][6]->setCaption(CHud::StrTime(timeT));
	if (stTrk[ch][7])  stTrk[ch][7]->setCaption(timeT < 0.1f ? "--" : speedTrk);

	if (timeCur < 0.1f)
	{
		if (stTrk[ch][8])  stTrk[ch][8]->setCaption(CHud::StrTime(0.f));
		if (stTrk[ch][9])  stTrk[ch][9]->setCaption("--");
		if (stTrk[ch][10])  stTrk[ch][10]->setCaption("--");
	}else
	{	//  car record
		std::string speed = fToStr(len / timeCur * m, 0,3) + unit;
		if (stTrk[ch][8])  stTrk[ch][8]->setCaption(CHud::StrTime(timeCur));
		if (stTrk[ch][9])  stTrk[ch][9]->setCaption(speed);
		//  points
		float points = 0.f;
		app->GetRacePos(timeCur, timeTrk, carMul, false, &points);
		if (stTrk[ch][10])  stTrk[ch][10]->setCaption(fToStr(points ,1,3));
	}
#else
	if (app->gui->trkName)  //
		app->gui->trkName->setCaption(sTrack.c_str());
#endif
	if (trkDesc[ch])  // desc
		trkDesc[ch]->setCaption(rd->sTxtDesc.c_str());

	
	//  preview images
	//---------------------------------------------------------------------------
#ifndef SR_EDITOR
	if (pSet->dev_no_prvs)  return;
#endif
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
