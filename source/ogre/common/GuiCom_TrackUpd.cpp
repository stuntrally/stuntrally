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
bool TrkSort(const TrkL& t1, const TrkL& t2)
{
	if (t1.ti && t2.ti)  // both are in tracks.ini
	{
		if (t1.ti->testC  != t2.ti->testC)   return t1.ti->testC;  // TestC first
		if (t1.ti->test   != t2.ti->test)    return t1.ti->test;   // Test after
		if (t1.ti->vdrift != t2.ti->vdrift)  return t1.ti->vdrift; // vdrift next
		
		if (TrkL::idSort > 1)
		{		 if (TrkL::idSort==2) {  if (t1.ti->scenery != t2.ti->scenery)  return t1.ti->scenery < t2.ti->scenery;  }
			else if (TrkL::idSort==3) {  if (t1.ti->crtver  != t2.ti->crtver)   return t1.ti->crtver  < t2.ti->crtver;   }
			else if (TrkL::idSort==4) {  if (t1.ti->diff    != t2.ti->diff)     return t1.ti->diff    < t2.ti->diff;     }
			else if (TrkL::idSort==5) {  if (t1.ti->rating  != t2.ti->rating)   return t1.ti->rating  < t2.ti->rating;   }
			else if (TrkL::idSort==6) {  if (t1.ti->objects != t2.ti->objects)  return t1.ti->objects < t2.ti->objects;  }
			else if (TrkL::idSort==7) {  if (t1.ti->fluids  != t2.ti->fluids)   return t1.ti->fluids  < t2.ti->fluids;   }
			else if (TrkL::idSort==8) {  if (t1.ti->bumps   != t2.ti->bumps)    return t1.ti->bumps   < t2.ti->bumps;    }
			else if (TrkL::idSort==9) {  if (t1.ti->jumps   != t2.ti->jumps)    return t1.ti->jumps   < t2.ti->jumps;    }
			else if (TrkL::idSort==10){  if (t1.ti->loops   != t2.ti->loops)    return t1.ti->loops   < t2.ti->loops;    }
			else if (TrkL::idSort==11){  if (t1.ti->pipes   != t2.ti->pipes)    return t1.ti->pipes   < t2.ti->pipes;    }
			else if (TrkL::idSort==12){  if (t1.ti->banked  != t2.ti->banked)   return t1.ti->banked  < t2.ti->banked;   }
			else if (TrkL::idSort==13){  if (t1.ti->frenzy  != t2.ti->frenzy)   return t1.ti->frenzy  < t2.ti->frenzy;   }
			else if (TrkL::idSort==14){  if (t1.ti->longn   != t2.ti->longn)    return t1.ti->longn   < t2.ti->longn;    }
		}
		else if (TrkL::idSort==1)	// 1 n
			if (t1.ti->n != t2.ti->n)  return t1.ti->n < t2.ti->n;

		//if (TrkL::idSort==0)		// 0 name, default
		if (t1.name[0] == t2.name[0] &&
			t1.ti->nn != t2.ti->nn)
			return t1.ti->nn < t2.ti->nn;  // using nn in name too

		return t1.ti->name < t2.ti->name;
	}
	//  user trks last
	if ( t1.ti &&!t2.ti)  return true;
	if (!t1.ti && t2.ti)  return false;

	return t1.name < t2.name;
}

//  done every list sort column change or find edit text change
//  fills gui track list
//-----------------------------------------------------------------------------------------------------------
void CGuiCom::TrackListUpd(bool resetNotFound)
{
	if (trkList)
	{	trkList->removeAllItems();
		int ii = 0, si = -1;  bool bFound = false;

		//  sort
		TrkL::idSort = min(14, (int)trkList->mSortColumnIndex);

		std::list<TrkL> liTrk2 = liTrk;  // copy
		liTrk2.sort(TrkSort);
		if (!trkList->mSortUp)  liTrk2.reverse();
		
		//  original
		for (std::list<TrkL>::iterator i = liTrk2.begin(); i != liTrk2.end(); ++i)
		{
			String name = (*i).name, nlow = name;  StringUtil::toLowerCase(nlow);
			if (sTrkFind == "" || strstr(nlow.c_str(), sTrkFind.c_str()) != 0)
			/// todo: filtering gui..
			//if ((*i).ti->rating > 2)
			//if ((*i).ti->diff > 3)
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
void CGuiCom::listTrackChng(Mli2 li, size_t pos)
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
	bool b = app->bGuiFocus && (app->mWndTabsTrack->getIndexSelected() == 1)
		&& !pSet->isMain && pSet->inMenu == WND_Track;
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
			float(colTrk[c]) / sum * 0.65/*width*/ * wi.width * 0.97/*frame*/ : 0);
		trkList->setColumnWidthAt(c, w);
		sw += w;
		if (c == 4)  wico = w;
		if (c < 4)  xico1 += w;
		if (c < 8)  xico2 += w;
	}

	int xt = 0.018*wi.width, yt = 0.06*wi.height, yico = yt - wico - 1;  //0.02*wi.height;
	trkList->setCoord(xt, yt, sw + 8/*frame*/, 0.70/*height*/*wi.height);
	imgTrkIco1->setCoord(xt + xico1+2, yico, 4*wico, wico);
	imgTrkIco2->setCoord(xt + xico2+2, yico, 8*wico, wico);
	#ifndef SR_EDITOR
	bool hid = app->gui->panNetTrack && app->gui->panNetTrack->getVisible();
	if (!hid)
	#endif
		trkList->setVisible(true);

	//  car list  ----------
	#ifndef SR_EDITOR
	sum = 0;  sw = 0;  cnt = app->gui->carList->getColumnCount();
	for (c=0; c < cnt; ++c)  sum += app->gui->colCar[c];

	for (c=0; c < cnt; ++c)
	{
		int w = (c==cnt-1) ? 18 : (float(app->gui->colCar[c]) / sum * 0.22/*width*/ * wi.width * 0.97/*frame*/);
		app->gui->carList->setColumnWidthAt(c, w);
		sw += w;
	}

	xt = 0.018*wi.width;  yt = 0.024*wi.height, yico = yt - wico - 1;  //0.02*wi.height;
	app->gui->carList->setCoord(xt, yt, sw + 8/*frame*/, 0.41/*height*/*wi.height);
	#endif
	
	#ifndef SR_EDITOR
	if (app->gui->panNetTrack)  {
		Tbi trkTab = fTbi("TabTrack");
		const IntCoord& tc = trkTab->getCoord();
		app->gui->panNetTrack->setCoord(0, 0.82f*tc.height, tc.width*0.64f, 0.162f*tc.height);  }
	#endif

	#ifdef SR_EDITOR
	const IntCoord& wp = app->mWndPick->getCoord();
	//IntCoord ic(0.01*wp.width, 0.04*wp.height, 0.38*wp.width, 0.93*wp.height);
	IntCoord ic(0.01*wp.width, 0.055*wp.height, 0.38*wp.width, 0.89*wp.height);
	ic.width = app->gui->liPickW[CGui::P_Sky];  app->gui->liSky->setCoord(ic);  ///pick dim
	ic.width = app->gui->liPickW[CGui::P_Tex];  app->gui->liTex->setCoord(ic);
	ic.width = app->gui->liPickW[CGui::P_Grs];  app->gui->liGrs->setCoord(ic);
	ic.width = app->gui->liPickW[CGui::P_Veg];  app->gui->liVeg->setCoord(ic);
	ic.width = app->gui->liPickW[CGui::P_Rd ];  app->gui->liRd->setCoord(ic);
	#endif
}
