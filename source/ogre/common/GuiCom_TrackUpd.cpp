#include "pch.h"
#include "../common/Def_Str.h"
#include "../../road/Road.h"
#include "../../vdrift/pathmanager.h"
#include "../common/SceneXml.h"
#include "../common/TracksXml.h"
#include "../common/CData.h"
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
void CGui::TrackListUpd(bool resetNotFound)
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
			app->UpdWndTitle();
			#endif
		}
		if (si > -1)  // center
			trkList->beginToItemAt(max(0, si-11));
	}
}


//  events  . . . . . . . . . . . . . . . . . . . . . . . . . 
//-----------------------------------------------------------------------------------------------------------

//  list changed position
void CGui::listTrackChng(MultiList2* li, size_t pos)
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
void CGui::edTrkFind(EditPtr ed)
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
void CGui::edRplFind(EditPtr ed)
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
void CGui::btnTrkView1(WP wp)
{
	pSet->tracks_view = 0;  ChangeTrackView();
}
void CGui::btnTrkView2(WP wp)
{
	pSet->tracks_view = 1;  ChangeTrackView();
}

void CGui::ChangeTrackView()
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
void CGui::updTrkListDim()
{
	//  tracks list  ----------
	if (!trkList)  return;
	bool full = pSet->tracks_view;

	int c, sum = 0, cnt = trkList->getColumnCount();
	for (c=0; c < cnt; ++c)  sum += colTrk[c];

	const IntCoord& wi = app->mWndOpts->getCoord();
	int sw = 0, xico1 = 0, xico2 = 0, wico = 0;

	for (c=0; c < cnt; ++c)
	{
		int w = c==cnt-1 ? 18 : (full || c==0 || c==cnt-1 ?
			float(colTrk[c]) / sum * 0.63/*width*/ * wi.width * 0.97/*frame*/ : 0);
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
	#ifndef SR_EDITOR
	bool hid = panelNetTrack && panelNetTrack->getVisible();
	if (!hid)
	#endif
		trkList->setVisible(true);

	//  car list  ----------
	#ifndef SR_EDITOR
	sum = 0;  sw = 0;  cnt = carList->getColumnCount();
	for (c=0; c < cnt; ++c)  sum += colCar[c];

	for (c=0; c < cnt; ++c)
	{
		int w = (c==cnt-1) ? 18 : (float(colCar[c]) / sum * 0.21/*width*/ * wi.width * 0.97/*frame*/);
		carList->setColumnWidthAt(c, w);
		sw += w;
	}

	xt = 0.018*wi.width;  yt = 0.024*wi.height, yico = yt - wico - 1;  //0.02*wi.height;
	carList->setCoord(xt, yt, sw + 8/*frame*/, 0.41/*height*/*wi.height);
	#endif
	
	#ifndef SR_EDITOR
	if (panelNetTrack)  {
		TabItem* trkTab = app->mGui->findWidget<TabItem>("TabTrack");
		const IntCoord& tc = trkTab->getCoord();
		panelNetTrack->setCoord(0, 0.82f*tc.height, tc.width*0.64f, 0.162f*tc.height);  }
	#endif
}

#ifndef SR_EDITOR
///  champ,chall,stages lists  ----------
void CGui::updChampListDim()
{
	const IntCoord& wi = app->mWndGame->getCoord();

	//  Champs  -----
	if (!liChamps)  return;

	int sum = 0, cnt = liChamps->getColumnCount(), sw = 0;
	for (int c=0; c < cnt; ++c)  sum += colCh[c];
	for (int c=0; c < cnt; ++c)
	{
		int w = c==cnt-1 ? 18 : float(colCh[c]) / sum * 0.72/*width*/ * wi.width * 0.97/*frame*/;
		liChamps->setColumnWidthAt(c, w);  sw += w;
	}
	int xt = 0.038*wi.width, yt = 0.10*wi.height;  // pos
	liChamps->setCoord(xt, yt, sw + 8/*frame*/, 0.32/*height*/*wi.height);
	liChamps->setVisible(!isChallGui());

	//  Stages  -----
	if (!liStages)  return;

	sum = 0;  cnt = liStages->getColumnCount();  sw = 0;
	for (int c=0; c < cnt; ++c)  sum += colSt[c];  sum += 43;//-
	for (int c=0; c < cnt; ++c)
	{
		int w = c==cnt-1 ? 18 : float(colSt[c]) / sum * 0.58/*width*/ * wi.width * 0.97/**/;
		liStages->setColumnWidthAt(c, w);  sw += w;
	}
	liStages->setCoord(xt, yt, sw + 8/**/, 0.50/*height*/*wi.height);
	liStages->setVisible(true);

	//  Challs  -----
	if (!liChalls)  return;

	sum = 0;  cnt = liChalls->getColumnCount();  sw = 0;
	for (int c=0; c < cnt; ++c)  sum += colChL[c];
	for (int c=0; c < cnt; ++c)
	{
		int w = c==cnt-1 ? 18 : float(colChL[c]) / sum * 0.71/*width*/ * wi.width * 0.97/**/;
		liChalls->setColumnWidthAt(c, w);  sw += w;
	}
	xt = 0.038*wi.width, yt = 0.10*wi.height;  // pos
	liChalls->setCoord(xt, yt, sw + 8/**/, 0.32/*height*/*wi.height);
	liChalls->setVisible(isChallGui());
}
#endif
