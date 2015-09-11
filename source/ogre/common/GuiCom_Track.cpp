#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "../../road/Road.h"
#include "../../vdrift/pathmanager.h"
#include "data/SceneXml.h"
#include "data/TracksXml.h"
#include "data/CData.h"
#include "CScene.h"
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
//-----------------------------------------------------------------------------------------------------------

//  track difficulties colors from value
const String CGuiCom::clrsDiff[9] =  // difficulty
	{"#60C0FF", "#00FF00", "#60FF00", "#C0FF00", "#FFFF00", "#FFC000", "#FF6000", "#FF4040", "#FF7090"};
const String CGuiCom::clrsRating[6] =  // rating
	{"#808080", "#606060", "#7090A0", "#60C8D8", "#A0D0F0", "#E0F0FF"};
const String CGuiCom::clrsLong[10] =  // long
	{"#E0D0D0", "#E8C0C0", "#F0B0B0", "#F8A0A0", "#FF9090", "#FF8080", "#F07070", "#F06060", "#E04040", "#D03030"};


//  * * * *  CONST  * * * *
//  column widths in MultiList2,  track detailed
const int wi = 17;            // id name nm   N  scn ver
const int CGuiCom::colTrk[32] = {40, 90, 80, 25, 70, 25, wi, wi, wi, wi, wi, wi, wi, wi, wi, wi, wi, wi, 24};
#ifndef SR_EDITOR
const int CGui::colCar[16] = {34, 27, 37, 52, 24};  // car
const int CGui::colCh [16] = {16, 200, 120, 50, 80, 80, 60, 40};  // champs
const int CGui::colChL[16] = {16, 180, 90, 100, 50, 60, 60, 60, 50};  // challs
const int CGui::colSt [16] = {30, 170, 100, 90, 50, 80, 70};  // stages
#endif

//  get scenery color string from track name
String CGuiCom::GetSceneryColor(String name, String* sc)
{
	if (name.length() < 3)  return "#707070";

	int id = app->scn->data->tracks->trkmap[name];
	const TrackInfo* pTrk = id==0 ? 0 : &app->scn->data->tracks->trks[id-1];

	char ch = name.c_str()[0];  // vdr
	if (ch == 'T' && name.c_str()[1] == 'e')  // Test,TestC
		return (name.length() > 5 && name.c_str()[4] == 'C') ? "#A0C0D0" : "#A0A0A0";
	else
	if (pTrk)
	{	//if (sc)  *sc = pTrk->scenery;
		return scnClr[pTrk->scenery];
	}
	else  // user *
	{
		if (name.c_str()[0]=='*')
			name = name.substr(1);
		String ss;
		size_t p = name.find_first_of("-0123456789");
		if (p != string::npos)
		{	ss = name.substr(0, p);  //LogO(ss);
			String s1 = scnN[ss];  if (sc)  *sc = s1;
			return scnClr[s1];
		}else
		{	ss += name.c_str()[0];
			String s1 = scnN[ss];  if (sc)  *sc = s1;
			return scnClr[s1];
	}	}
}
//-----------------------------------------------------------------------------------------------------------


//  Add tracks list item
void CGuiCom::AddTrkL(std::string name, int user, const TrackInfo* ti)
{
	String sc, c = GetSceneryColor(name, &sc);
	Mli2 li = trkList;

	//  split -
	string pre, shrt;
	size_t p = name.find("-");  // Test
	if (p != string::npos /*&& !(name[0]=='T' && name[1]=='e')*/)
	{
		pre = name.substr(0,p);
		shrt = name.substr(p+1);  // short name
	}else
	{	pre = name;  shrt = name;  }
	
	//  add  name = prefix-short
	li->addItem(c+ pre, 0);
	int l = li->getItemCount()-1;
	li->setSubItemNameAt(1,l, c+ name);
	li->setSubItemNameAt(2,l, c+ shrt);

	if (!ti)  //  user (or new) trks
	{	if (!sc.empty())
			li->setSubItemNameAt(4,l, c+ TR("#{SC_"+sc+"}"));
		return;
	}
	//  details
	li->setSubItemNameAt(3,l, c+ toStr(ti->n/10)+toStr(ti->n%10));
	li->setSubItemNameAt(4,l, c+ TR("#{SC_"+ti->scenery+"}"));
	li->setSubItemNameAt(5,l, c+ fToStr(ti->crtver,1,3));

	//list->setSubItemNameAt(4,l, ti->created);  list->setSubItemNameAt(5,l, ti->modified);
	#define toS(clr,v)  (v > 0) ? (String(clr)+"  "+toStr(v)) : " "
	li->setSubItemNameAt(6,l, toS(clrsDiff[ti->diff], ti->diff));
	li->setSubItemNameAt(7,l, toS(clrsRating[ti->rating], ti->rating));
	
	//todo: rateuser drivenlaps
	li->setSubItemNameAt(8,l, toS("#D070A0",ti->objects));
	li->setSubItemNameAt(9,l, toS("#C09060",ti->obstacles));
	li->setSubItemNameAt(10,l,toS("#80C0FF",ti->fluids));
	li->setSubItemNameAt(11,l,toS("#40FF00",ti->bumps));
	li->setSubItemNameAt(12,l,toS("#FFA030",ti->jumps));
	li->setSubItemNameAt(13,l,toS("#00FFFF",ti->loops));
	li->setSubItemNameAt(14,l,toS("#FFFF00",ti->pipes));
	li->setSubItemNameAt(15,l,toS("#C0C0C0",ti->banked));
	li->setSubItemNameAt(16,l,toS("#C080FF",ti->frenzy));
	li->setSubItemNameAt(17,l,toS(clrsLong[ti->longn], ti->longn));
	//li->setSubItemNameAt(18,l,clrsDiff[std::min(8, 5*ti->sum/10)]+" "+toStr(ti->sum));
}


void CGuiCom::initMiniPos(int i)
{
	imgMiniPos[i] = fImg(i==0 ? "TrackPos" : "TrackPos2");
	imgMiniRot[i] = imgMiniPos[i]->getSubWidgetMain()->castType<RotatingSkin>();
	IntSize si = imgMiniPos[i]->getSize();
	imgMiniRot[i]->setCenter(IntPoint(si.width*0.7f, si.height*0.7f));  //0.5?
}


//  Gui Init [Track] once
//-----------------------------------------------------------------------------------------------------------
void CGuiCom::GuiInitTrack()
{
	Tbi trktab = fTbi("TabTrack");
	Mli2 li = trktab->createWidget<MultiList2>("MultiListBox",0,0,500,300, Align::Left | Align::VStretch);
	li->setColour(Colour(0.8,0.9,0.8));  li->setInheritsAlpha(false);
	trkList = li;
   	li->eventListChangePosition += newDelegate(this, &CGuiCom::listTrackChng);
   	li->setVisible(false);
	
	//  preview images
	imgPrv[0] = fImg("TrackImg");   imgPrv[0]->setImageTexture("PrvView");
	imgTer[0] = fImg("TrkTerImg");  imgTer[0]->setImageTexture("PrvTer");
	imgMini[0] = fImg("TrackMap");  imgMini[0]->setImageTexture("PrvRoad");
	initMiniPos(0);

	//  stats text
	int i, st;
	#ifdef SR_EDITOR
	st = 9;
	#else
	st = StTrk;
	#endif
	for (i=0; i < st; ++i)   stTrk[0][i] = fTxt("st"+toStr(i));
	for (i=0; i < 4; ++i)  imStTrk[0][i] = fImg("ist"+toStr(i));
	for (i=0; i < InfTrk; ++i){  infTrk[0][i] = fTxt("ti"+toStr(i));  imInfTrk[0][i] =  fImg("iti"+toStr(i));  }
		
	EdC(edTrkFind, "TrkFind", editTrkFind);

	ButtonPtr btn;
	BtnC("TrkView1", btnTrkView1);  imgTrkIco1 = fImg("TrkView2icons1");
	BtnC("TrkView2", btnTrkView2);  imgTrkIco2 = fImg("TrkView2icons2");
	BtnC("TrkFilter", btnTrkFilter);
	SV* sv;  Ck* ck;
	ck= &ckTrkFilter;  ck->Init("TracksFilter", &pSet->tracks_filter);  CevC(TrkFilter);
	txtTracksFAll = fTxt("TracksFAll");
	txtTracksFCur = fTxt("TracksFCur");
	
	//  columns  ----
	li->removeAllColumns();  int c=0;
	li->addColumn("#C0E0C0""id", colTrk[c++]);  // prefix
	li->addColumn("#D0FFD0"+TR("#{Name}"), colTrk[c++]);  // full
	li->addColumn("#E0FFE0"+TR("#{Name}"), colTrk[c++]);  // short

	li->addColumn("#80FF80""N", colTrk[c++]);
	li->addColumn("#80FFC0"+TR("#{Scenery}"), colTrk[c++]);
	li->addColumn("#80FF80""ver", colTrk[c++]);  // created- modified-

	li->addColumn("#C0D0FF""diff", colTrk[c++]);  //todo: rateuser, drivenlaps ..
	li->addColumn("#C0E0FF""*", colTrk[c++]);   // rating

	li->addColumn("#FF80C0""o", colTrk[c++]);   // objects
	li->addColumn("#C09060""c", colTrk[c++]);   // obstacles
	li->addColumn("#80C0FF""f", colTrk[c++]);   //  fluids
	li->addColumn("#40FF00""B", colTrk[c++]);   //  Bumps
	li->addColumn("#FFA030""J", colTrk[c++]);   // Jumps
	li->addColumn("#00FFFF""L", colTrk[c++]);   // Loops
	li->addColumn("#FFFF00""P", colTrk[c++]);   // Pipes
	li->addColumn("#C0C0C0""b", colTrk[c++]);   //  banked
	li->addColumn("#C080FF""f", colTrk[c++]);   //  frenzy
	li->addColumn("#FFA0A0""l", colTrk[c++]);	// longn
	//li->addColumn("#9060FF""E", colTrk[c++]);	// sigma
	li->addColumn(" ", colTrk[c++]);
	
	//  columns, filters  ---
	for (i=0; i < COL_VIS; ++i)
	{
		ck= &ckTrkColVis[i];  ck->Init("col"+toStr(i), &pSet->col_vis[pSet->tracks_view][i]);  CevC(TrkColVis);
	}
	//ChkUpd_Col();
	for (i=0; i < COL_FIL; ++i)
	{	string si = toStr(i);
		int a = pSet->colFil[0][i], b = pSet->colFil[1][i];
		sv= &svTrkFilMin[i];  sv->Init("min"+si, &pSet->col_fil[0][i], a,b);  sv->DefaultI(a);  SevC(TrkFil);
		sv= &svTrkFilMax[i];  sv->Init("max"+si, &pSet->col_fil[1][i], a,b);  sv->DefaultI(b);  SevC(TrkFil);
	}

	FillTrackLists();  //once

	li->mSortColumnIndex = pSet->tracks_sort;
	li->mSortUp = pSet->tracks_sortup;

    TrackListUpd(true);  //upd
	listTrackChng(trkList,0);  //-

	ChangeTrackView();
}

void CGuiCom::ChkUpd_Col()
{
	for (int i=0; i < COL_VIS; ++i)
		ckTrkColVis[i].Upd(&pSet->col_vis[pSet->tracks_view][i]);
}


//  done once to fill tracks list from dirs
//-----------------------------------------------------------------------------------------------------------
void CGuiCom::FillTrackLists()
{
	liTracks.clear();  liTracksUser.clear();
	std::string chkfile = "/scene.xml";

	PATHMANAGER::DirList(pathTrk[0], liTracks);
	PATHMANAGER::DirList(pathTrk[1], liTracksUser);  //name duplicates
	if (liTracks.size() == 0)
		LogO("Error: NO tracks !!!  in data/tracks/  crashing.");

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
		int id = app->scn->data->tracks->trkmap[*i];
		const TrackInfo* pTrk = id==0 ? 0 : &app->scn->data->tracks->trks[id-1];
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
	SplineRoad rd(app->pGame);  rd.LoadFile(sRd,false);  // load

	TIMER tim;  tim.Load(PATHMANAGER::Records()+"/"+ pSet->gui.sim_mode+"/"+ sListTrack+".txt", 0.f);
	tim.AddCar(app->gui->sListCar);

	bool reverse = sc->denyReversed ? false : pSet->gui.trackreverse;
	app->gui->ckReverse.setVisible(!sc->denyReversed);  //
	UpdGuiRdStats(&rd,sc, sListTrack, tim.GetBestLap(0, reverse), reverse, 0);
#else
	SplineRoad rd(app);  rd.LoadFile(sRd,false);  // load
	UpdGuiRdStats(&rd,sc, sListTrack, 0.f, false, 0);
#endif
	delete sc;
}

#ifndef SR_EDITOR  // game
void CGui::ReadTrkStatsChamp(String track, bool reverse)
{
	String sRd = gcom->pathTrk[0] + track + "/road.xml";
	String sSc = gcom->pathTrk[0] + track + "/scene.xml";

	Scene* sc = new Scene();  sc->LoadXml(sSc);  // fails to defaults
	SplineRoad rd(pGame);  rd.LoadFile(sRd,false);  // load

	TIMER tim;  tim.Load(PATHMANAGER::Records()+"/"+ pSet->gui.sim_mode+"/"+ track+".txt", 0.f);
	tim.AddCar(sListCar);

	gcom->UpdGuiRdStats(&rd,sc, track, tim.GetBestLap(0, reverse), reverse, 1);
}
#endif


#ifndef SR_EDITOR  // game
String CGuiCom::TrkDir() {
	int u = pSet->game.track_user ? 1 : 0;		return pathTrk[u] + pSet->game.track + "/";  }
#else
String CGuiCom::TrkDir() {
	int u = pSet->gui.track_user ? 1 : 0;		return pathTrk[u] + pSet->gui.track + "/";  }
#endif

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


void CGuiCom::UpdGuiRdStats(const SplineRoad* rd, const Scene* sc, const String& sTrack,
	float timeCur, bool reverse, int ch)
{
	#ifndef SR_EDITOR  // game
	bool mph = pSet->show_mph;
	#else
	bool mph = false;
	#endif
	float m = mph ? 0.621371f : 1.f;
	String km = mph ? TR(" #{UnitMi}") : TR(" #{UnitKm}");
	String sm = TR(" #{UnitM}");
	
	//  road stats
	//---------------------------------------------------------------------------
	stTrk[ch][1]->setCaption(fToStr(sc->td.fTerWorldSize*0.001f*m ,1,3) + km);
	if (!rd)  return;
	float len = rd->st.Length;					//3,5
	stTrk[ch][0]->setCaption(fToStr(len*0.001f*m ,1,3) + km);

	stTrk[ch][2]->setCaption(fToStr(rd->st.WidthAvg ,1,3) + sm);
	stTrk[ch][3]->setCaption(fToStr(rd->st.HeightDiff ,0,2) + sm);

	bool h = rd->st.Pipes > 99.f && rd->st.OnTer > 99.f;  // hide bridge 100% when pipe is 100%
	float a;
	stTrk[ch][4]->setCaption(fToStr(rd->st.OnTer ,0,1)/*+"%"*/);
	a = h || rd->st.OnTer < 1.f ? 0.f :  (0.5f + 0.5f * rd->st.OnTer / 100.f);
	stTrk[ch][4]->setAlpha(a);  imStTrk[ch][0]->setAlpha(a);
	
	stTrk[ch][5]->setCaption(fToStr(rd->st.Pipes ,0,1)/*+"%"*/);
	a =      rd->st.Pipes < 1.f ? 0.f :  (0.4f + 0.4f * rd->st.Pipes / 100.f);
	stTrk[ch][5]->setAlpha(a);  imStTrk[ch][1]->setAlpha(a);

	stTrk[ch][6]->setCaption(fToStr(rd->st.bankAvg,0,1)+"\'");
	stTrk[ch][7]->setCaption(fToStr(rd->st.bankMax,0,1)+"\'");
	a = std::min(1.f,  0.3f + 0.7f * rd->st.bankMax / 80.f);  //rd->st.bankAvg / 30.f);
	stTrk[ch][6]->setAlpha(a);  stTrk[ch][7]->setAlpha(a);  imStTrk[ch][2]->setAlpha(a);

	stTrk[ch][8]->setCaption(fToStr(rd->st.OnPipe,0,1)/*+"%"*/);
	a = rd->st.OnPipe < 0.1f ? 0.f : (0.5f + 0.5f * rd->st.OnPipe / 100.f);
	stTrk[ch][8]->setAlpha(a);  imStTrk[ch][3]->setAlpha(a);

	#ifndef SR_EDITOR
	if (app->gui->txTrackAuthor)
		app->gui->txTrackAuthor->setCaption("");  // user trks
	#endif
	
	int id = app->scn->data->tracks->trkmap[sTrack];
	for (int i=0; i < InfTrk; ++i)
		if (infTrk[ch][i])  infTrk[ch][i]->setCaption("");
	if (id > 0)
	{	const TrackInfo& ti = app->scn->data->tracks->trks[id-1];

		#define str0(v)  ((v)==0 ? "" : toStr(v))
		#define inf(i,t,m)  infTrk[ch][i]->setCaption(str0(t));\
			imInfTrk[ch][i]->setAlpha(t==0 ? 0.f : std::min(1.f, 0.2f + 0.8f * float(t)/m))

		inf(0, ti.fluids,5);  inf(1, ti.bumps, 4);
		inf(2, ti.jumps, 3);  inf(3, ti.loops, 4);  inf(4, ti.pipes, 5);
		inf(5, ti.banked,4);  inf(6, ti.frenzy,4);
		inf(7, ti.obstacles,3);  inf(8, ti.objects,3);

		infTrk[ch][11]->setCaption(clrsLong[ti.longn] + str0(ti.longn));
		a = ti.longn / 9.f;
		imInfTrk[ch][11]->setAlpha(a);  infTrk[ch][11]->setAlpha(0.5f + 0.5f * a);
		  infTrk[ch][9]->setCaption(ti.diff==0   ? "" : (clrsDiff[ti.diff] + toStr(ti.diff)));
		imInfTrk[ch][9]->setAlpha(0.2f + 0.8f * ti.diff / 6.f);
		  infTrk[ch][10]->setCaption(ti.rating==0 ? "" : (clrsRating[ti.rating] + toStr(ti.rating)));
		imInfTrk[ch][10]->setAlpha(0.2f + 0.8f * ti.rating / 5.f);

		#ifndef SR_EDITOR
		if (app->gui->txTrackAuthor)
			app->gui->txTrackAuthor->setCaption(ti.author=="CH" ? "CryHam" : ti.author);
		#endif
	}

#ifndef SR_EDITOR  // game
	//  best time, avg vel
	std::string unit = mph ? TR(" #{UnitMph}") : TR(" #{UnitKmh}");
	m = pSet->show_mph ? 2.23693629f : 3.6f;

	//  track time
	float carMul = app->GetCarTimeMul(pSet->gui.car[0], pSet->gui.sim_mode);
	float timeTrk = app->scn->data->tracks->times[sTrack];
	bool noTrk = timeTrk < 2.f;
	std::string speedTrk = fToStr(len / timeTrk * m, 0,3) + unit;
	float timeT = (/*place*/1 * app->scn->data->cars->magic * timeTrk + timeTrk) / carMul;
	bool no = timeCur < 0.1f || !rd;
	if (ch==1)  no = false;  // show track's not current

	stTrk[ch][9]->setCaption(StrTime(noTrk ? 0.f : timeT));
	stTrk[ch][10]->setCaption(noTrk ? "--" : speedTrk);

	if (ch==0)
	if (no)
	{	stTrk[ch][11]->setCaption(StrTime(0.f));
		stTrk[ch][12]->setCaption("--");
		stTrk[ch][13]->setCaption("--");
	}else
	{	//  car record
		std::string speed = fToStr(len / timeCur * m, 0,3) + unit;
		stTrk[ch][11]->setCaption(StrTime(timeCur));
		stTrk[ch][12]->setCaption(speed);
		//  points
		float points = 0.f;
		app->GetRacePos(timeCur, timeTrk, carMul, false, &points);
		stTrk[ch][13]->setCaption(fToStr(points ,1,3));
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
	bool any = pSet->inMenu == MNU_Tutorial || pSet->inMenu == MNU_Champ || pSet->inMenu == MNU_Challenge;
#else
	bool any = false;
#endif
	String path = PathListTrkPrv(any ? 0 : -1, sTrack);

	app->prvView.Load(path+"view.jpg");
	app->prvRoad.Load(path+"road.png");
	app->prvTer.Load(path+"terrain.jpg");


	//  start pos on minimap
	//---------------------------------------------------------------------------
	float t = sc->td.fTerWorldSize,
		xp = sc->startPos[1]/t, yp = sc->startPos[0]/t;
	const IntSize& si = imgTer[ch]->getSize(), st = imgMiniPos[ch]->getSize();
	int x = (xp + 0.5f) * si.width  - st.width *0.5f,
		y = (yp + 0.5f) * si.height - st.height*0.5f;
	imgMiniPos[ch]->setPosition(IntPoint(x,y));

	//  rot
	const float* rot = &sc->startRot[0];
	Quaternion q(rot[0],rot[1],rot[2],rot[3]);
	a = q.getPitch().valueRadians();
	if (reverse)  a += PI_d;
	//static float a=0.f; a+=0.1f;  //test center
	imgMiniRot[ch]->setAngle(a);
}
