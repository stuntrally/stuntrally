#include "pch.h"
#include "../common/Def_Str.h"
#include "GuiCom.h"
#include "../../road/Road.h"
#include "../../vdrift/pathmanager.h"
#include "../common/data/SceneXml.h"
#include "../common/data/TracksXml.h"
#include "../common/data/CData.h"
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
#include <MyGUI_Window.h>
#include <MyGUI_TabItem.h>
#include <MyGUI_TabControl.h>
#include <MyGUI_ImageBox.h>
#include <MyGUI_Gui.h>
#include <MyGUI_EditBox.h>
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
void CGuiCom::TrackListUpd(bool resetNotFound)
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

bool CGuiCom::needSort(Mli2 li)
{
	if (!li)  return false;
	if (li->mSortColumnIndex != li->mSortColumnIndexOld ||
		li->mSortUp != li->mSortUpOld)
	{
		li->mSortColumnIndexOld = li->mSortColumnIndex;
		li->mSortUpOld = li->mSortUp;
		return true;
	}
	return false;
}
void CGuiCom::SortTrkList()
{	
	if (needSort(trkList))
	{
		pSet->tracks_sort = trkList->mSortColumnIndex;
		pSet->tracks_sortup = trkList->mSortUp;
		TrackListUpd(false);
	}
}


//  events  . . . . . . . . . . . . . . . . . . . . . . . . . 
//-----------------------------------------------------------------------------------------------------------

//  list changed position
void CGuiCom::listTrackChng(MultiList2* li, size_t pos)
{
	if (!li)  return;
	size_t i = li->getIndexSelected();  if (i==ITEM_NONE)  return;

	const UString& sl = li->getItemNameAt(i);  String s = sl, s1 = s;
	s = StringUtil::replaceAll(s, "*", "");
	bListTrackU = s1 != s ? 1 : 0;
	if (s[0] == '#')  s = s.substr(7);
	sListTrack = s;

#ifndef SR_EDITOR
	app->gui->changeTrack();
#endif
#ifdef SR_EDITOR
	if (app->gui->iLoadNext==0)
#endif
		ReadTrkStats();
}

//  key util
#ifdef SR_EDITOR
void CGuiCom::trkListNext(int rel)
{
	bool b = app->bGuiFocus && (app->mWndTabsEdit->getIndexSelected() == 1)
		&& !pSet->isMain && pSet->inMenu == WND_Edit;
	if (!b)  return;
	
	size_t cnt = trkList->getItemCount();
	if (cnt == 0)  return;
	int i = std::max(0, std::min((int)cnt-1, (int)trkList->getIndexSelected()+rel ));
	trkList->setIndexSelected(i);
	trkList->beginToItemAt(std::max(0, i-11));  // center
	listTrackChng(trkList,i);
}
#endif

//  find edit changed text
void CGuiCom::editTrkFind(EditPtr ed)
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
void CGuiCom::btnTrkView1(WP wp)
{
	pSet->tracks_view = 0;  ChangeTrackView();
}
void CGuiCom::btnTrkView2(WP wp)
{
	pSet->tracks_view = 1;  ChangeTrackView();
}

void CGuiCom::ChangeTrackView()
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
void CGuiCom::updTrkListDim()
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
	bool hid = app->gui->panelNetTrack && app->gui->panelNetTrack->getVisible();
	if (!hid)
	#endif
		trkList->setVisible(true);

	//  car list  ----------
	#ifndef SR_EDITOR
	sum = 0;  sw = 0;  cnt = app->gui->carList->getColumnCount();
	for (c=0; c < cnt; ++c)  sum += app->gui->colCar[c];

	for (c=0; c < cnt; ++c)
	{
		int w = (c==cnt-1) ? 18 : (float(app->gui->colCar[c]) / sum * 0.21/*width*/ * wi.width * 0.97/*frame*/);
		app->gui->carList->setColumnWidthAt(c, w);
		sw += w;
	}

	xt = 0.018*wi.width;  yt = 0.024*wi.height, yico = yt - wico - 1;  //0.02*wi.height;
	app->gui->carList->setCoord(xt, yt, sw + 8/*frame*/, 0.41/*height*/*wi.height);
	#endif
	
	#ifndef SR_EDITOR
	if (app->gui->panelNetTrack)  {
		TabItem* trkTab = mGui->findWidget<TabItem>("TabTrack");
		const IntCoord& tc = trkTab->getCoord();
		app->gui->panelNetTrack->setCoord(0, 0.82f*tc.height, tc.width*0.64f, 0.162f*tc.height);  }
	#endif
}
