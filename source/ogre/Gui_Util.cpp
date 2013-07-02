#include "pch.h"
#include "common/Defines.h"
#include "../vdrift/pathmanager.h"
#include "../vdrift/settings.h"
#include "OgreGame.h"
#include "common/Gui_Def.h"
#include "common/TracksXml.h"
#include "common/MultiList2.h"
#include "common/Slider.h"
//#include "../network/masterclient.hpp"
#include "../network/gameclient.hpp"
#include <boost/filesystem.hpp>
using namespace std;
using namespace Ogre;
using namespace MyGUI;


///  Car list

//  sort	 . . . . . . . . . . . . . . . . . . . . . . 
//-----------------------------------------------------------------------------------------------------------
/*  common sort code,  no info only by name  */
#define sArg  const CarL& c2, const CarL& c1
#define sortDef  bool t = false;  if (!c1.ci || !c2.ci)  return c1.name > c2.name || t;
const int allSortFunc = 5;

//  sorting functions for columns
/* 0  name    */  bool Sort0 (sArg){  sortDef  return c1.name  < c2.name   || t;  }
/* 1  speed   */  bool Sort1 (sArg){  sortDef  return c1.ci->speed < c2.ci->speed || t;  }
/* 2  year    */  bool Sort2 (sArg){  sortDef  return c1.ci->year < c2.ci->year  || t;  }
/* 3  type    */  bool Sort3 (sArg){  sortDef  return c1.ci->type < c2.ci->type || t;  }
/* 4  rate    */  bool Sort4 (sArg){  sortDef  return c1.ci->rating < c2.ci->rating || t;  }

//  sorting functions array (to access by column index)
bool (*CarSort[allSortFunc])(const CarL& c1, const CarL& c2) = {
	Sort0, Sort1, Sort2, Sort3, Sort4 };


//  done every list sort column change or find edit text change
//  fills gui track list
//-----------------------------------------------------------------------------------------------------------
void App::CarListUpd(bool resetNotFound)
{
	if (carList)
	{	carList->removeAllItems();
		int ii = 0, si = -1;  bool bFound = false;

		//  sort
		int numFunc = min(allSortFunc-1, (int)carList->mSortColumnIndex);
		std::list<CarL> liCar2 = liCar;  // copy
		liCar2.sort(CarSort[numFunc]);
		if (carList->mSortUp)  liCar2.reverse();
		
		//  original
		for (std::list<CarL>::iterator i = liCar2.begin(); i != liCar2.end(); ++i)
		{
			String name = (*i).name, nlow = name;  StringUtil::toLowerCase(nlow);
			//if (sTrkFind == "" || strstr(nlow.c_str(), sTrkFind.c_str()) != 0)
			{
				AddCarL(name, (*i).ci);
				if (name == pSet->gui.car[0])  {  si = ii;
					carList->setIndexSelected(si);
					bFound = true;  }
				ii++;
		}	}

		//  not found last track, set 1st  .. only 
		if (resetNotFound && !bFound && !liTracks.empty())
		{	pSet->gui.track = *liTracks.begin();  pSet->gui.track_user = 0;
		}
		if (si > -1)  // center
			carList->beginToItemAt(max(0, si-5));
	}
}

void App::AddCarL(std::string name, const CarInfo* ci)
{
	MultiList2* li = carList;
	CarInfo cci;
	if (!ci)  ci = &cci;  //  details
	String clr = carsXml.colormap[ci->type];  if (clr.length() != 7)  clr = "#C0D0E0";
	
	li->addItem(clr+ name);  int l = li->getItemCount()-1, y = ci->year%100;
	li->setSubItemNameAt(1,l, clrsDiff[std::min(7, (int)(ci->speed*0.9f))]+ toStr(ci->speed));
	li->setSubItemNameAt(2,l, clr+ "\'"+toStr(y/10)+toStr(y%10));
	li->setSubItemNameAt(3,l, clr+ TR("#{CarType_"+ci->type+"}"));
}

void App::FillCarList()
{
	liCar.clear();
	strlist li;
	PATHMANAGER::GetFolderIndex(PATHMANAGER::Cars(), li);
	for (strlist::iterator i = li.begin(); i != li.end(); ++i)
	{
		if (boost::filesystem::exists(PATHMANAGER::Cars() + "/" + *i + "/about.txt"))
		{	String s = *i;
			CarL c;  c.name = *i;  c.pA = this;
			int id = carsXml.carmap[*i];
			c.ci = id==0 ? 0 : &carsXml.cars[id-1];
			liCar.push_back(c);
	}	}
}
//-----------------------------------------------------------------------------------------------------------


//  [Game] 	. . . . . . . . . . . . . . . . . . . .    --- lists ----    . . . . . . . . . . . . . . . . . . 

//  car
void App::listCarChng(MultiList2* li, size_t pos)
{
	size_t i = li->getIndexSelected();  if (i==ITEM_NONE)  return;
	const UString& sl = li->getItemNameAt(i).substr(7);  sListCar = sl;

	if (imgCar)  imgCar->setImageTexture(sListCar+".jpg");
	if (mClient) mClient->updatePlayerInfo(pSet->nickname, sListCar);
	
	//  car desc load
	if (carDesc)
	{
		string path = PATHMANAGER::Cars()+"/"+sListCar+"/description.txt";
		ifstream fi(path.c_str());

		string sdesc = "", s;  bool f1 = true;
		while (getline(fi, s))
		{
			if (f1) {  f1 = false;
				if (txCarAuthor)  txCarAuthor->setCaption(s);  }
			else
				sdesc += s + "\n";
		}
		fi.close();

		carDesc->setCaption(sdesc);
	}
	//  car info
	int id = carsXml.carmap[sl];
	if (id > 0 && txCarSpeed && txCarType)
	{	const CarInfo& ci = carsXml.cars[id-1];
		txCarSpeed->setCaption(clrsDiff[std::min(7, (int)(ci.speed*0.9f))]+ toStr(ci.speed));
		txCarType->setCaption(carsXml.colormap[ci.type]+ TR("#{CarType_"+ci.type+"}"));
	}
	
	changeCar();
	UpdCarStatsTxt();
}	
void App::changeCar()
{
	pSet->gui.car[iCurCar] = sListCar;
}

///  car stats txt
void App::UpdCarStatsTxt()
{
	std::string path;
	//GetCarPath(&path, 0, 0, sListCar, true);
	//path = path.substr(0, path.length()-4) + "_stats.txt";
	path = PATHMANAGER::CarSim() + "/" + pSet->gui.sim_mode + "/cars/" + sListCar + "_stats.txt";

	string txt(""), vals(""), s;
	ifstream fi(path.c_str());
	if (fi.good())
	{
		int i = 0;
		while (getline(fi, s))
		{
			if (i % 2 == 0)
				txt += s + "\n";
			else
				vals += s + "\n";
			++i;
		}
		fi.close();
	}
	txCarStatsTxt->setCaption(txt);
	txCarStatsVals->setCaption(vals);
}


//  track
void App::changeTrack()
{
	pSet->gui.track = sListTrack;
	pSet->gui.track_user = bListTrackU;

	if (mMasterClient)
	{	uploadGameInfo();
		updateGameInfoGUI();  }
}

//  new game
void App::btnNewGame(WP)
{
	if (mWndGame->getVisible() && mWndTabsGame->getIndexSelected() < 6  || mClient)
		pSet->gui.champ_num = -1;  /// champ, back to single race
	
	NewGame();  isFocGui = false;  // off gui
	if (mWndOpts)  mWndOpts->setVisible(isFocGui);
	if (mWndRpl)  mWndRpl->setVisible(false);//
	if (bnQuit)  bnQuit->setVisible(isFocGui);
	
	updMouse();
	
	mToolTip->setVisible(false);
}
void App::btnNewGameStart(WP wp)
{
	changeTrack();
	btnNewGame(wp);
}


//  Menu
//-----------------------------------------------------------------------------------------------------------

void App::toggleGui(bool toggle)
{
	if (toggle)
		isFocGui = !isFocGui;

	bool notMain = isFocGui && !pSet->isMain;
	if (mWndMain)	mWndMain->setVisible(isFocGui && pSet->isMain);
	if (mWndReplays) mWndReplays->setVisible(notMain && pSet->inMenu == WND_Replays);
	if (mWndHelp)	mWndHelp->setVisible(notMain && pSet->inMenu == WND_Help);
	if (mWndOpts)	mWndOpts->setVisible(notMain && pSet->inMenu == WND_Options);
	
	//  fill Readme editbox from file
	if (mWndHelp && mWndHelp->getVisible() && loadReadme)
	{
		loadReadme = false;
		EditBox* edit = mGUI->findWidget<EditBox>("Readme",false);
		if (edit)
		{	std::string path = PATHMANAGER::Data()+"/../Readme.txt";
			std::ifstream fi(path.c_str());
			if (fi.good())
			{	String text = "", s;
				while (getline(fi,s))
					text += s + "\n";

				text = StringUtil::replaceAll(text, "#", "##");
				edit->setCaption(UString(text));
				edit->setVScrollPosition(0);
	}	}	}

	///  update track tab, for champs wnd
	bool game = pSet->inMenu == WND_Game, champ = pSet->inMenu == WND_Champ, gc = game || champ;
	if (mWndGame)
	{	mWndGame->setVisible(notMain  && gc);
		if (mWndGame->getVisible())
			mWndGame->setCaption(champ ? TR("#{Championship}") : TR("#{SingleRace}"));
	}
	if (notMain && gc)  // show hide champs,stages
	{
		size_t id = mWndTabsGame->getIndexSelected();
		mWndTabsGame->setButtonWidthAt(1,champ ? 1 :-1);  if (id == 1 && champ)  mWndTabsGame->setIndexSelected(6);
		mWndTabsGame->setButtonWidthAt(5,champ ? 1 :-1);  if (id == 5 && champ)  mWndTabsGame->setIndexSelected(6);
		mWndTabsGame->setButtonWidthAt(6,champ ?-1 : 1);  if (id == 6 && !champ)  mWndTabsGame->setIndexSelected(1);
		mWndTabsGame->setButtonWidthAt(7,champ ?-1 : 1);  if (id == 7 && !champ)  mWndTabsGame->setIndexSelected(1);
		mWndTabsGame->setButtonWidthAt(8,champ ?-1 : 1);  if (id == 8 && !champ)  mWndTabsGame->setIndexSelected(1);
	}

	if (bnQuit)  bnQuit->setVisible(isFocGui);
	updMouse();
	if (!isFocGui)  mToolTip->setVisible(false);

	for (int i=0; i < WND_ALL; ++i)
		mWndMainPanels[i]->setVisible(pSet->inMenu == i);
}

void App::MainMenuBtn(MyGUI::WidgetPtr wp)
{
	for (int i=0; i < WND_ALL; ++i)
		if (wp == mWndMainBtns[i])
		{
			pSet->isMain = false;
			pSet->inMenu = i;
			toggleGui(false);
			return;
		}
}

void App::MenuTabChg(MyGUI::TabPtr tab, size_t id)
{
	if (id != 0)  return;
	tab->setIndexSelected(1);  // dont switch to 0
	pSet->isMain = true;
	toggleGui(false);  // back to main
}

void App::GuiShortcut(WND_Types wnd, int tab, int subtab)
{
	if (subtab == -1 && (!isFocGui || pSet->inMenu != wnd))  subtab = -2;  // cancel subtab cycling

	isFocGui = true;
	pSet->isMain = false;  pSet->inMenu = wnd;
	
	MyGUI::TabPtr mWndTabs = 0;
	std::vector<MyGUI::TabControl*>* subt = 0;
	
	switch (wnd)
	{	case WND_Champ:
		case WND_Game:		mWndTabs = mWndTabsGame;  subt = &vSubTabsGame;  break;
		case WND_Replays:	mWndTabs = mWndTabsRpl;  break;
		case WND_Help:		mWndTabs = mWndTabsHelp;  break;
		case WND_Options:	mWndTabs = mWndTabsOpts;  subt = &vSubTabsOpts;  break;
	}
	toggleGui(false);


	size_t t = mWndTabs->getIndexSelected();
	mWndTabs->setIndexSelected(tab);

	if (!subt)  return;
	MyGUI::TabControl* tc = (*subt)[tab];  if (!tc)  return;
	int  cnt = tc->getItemCount();

	if (t == tab && subtab == -1)  // cycle subpages if same tab
	{	if (shift)
			tc->setIndexSelected( (tc->getIndexSelected()-1+cnt) % cnt );
		else
			tc->setIndexSelected( (tc->getIndexSelected()+1) % cnt );
	}
	if (subtab > -1)
		tc->setIndexSelected( std::min(cnt-1, subtab) );
}

//  close netw end
void App::btnNetEndClose(WP)
{
	mWndNetEnd->setVisible(false);
	isFocGui = true;  // show back gui
	toggleGui(false);
}


//  utility
//---------------------------------------------------------------------------------------------------------------------

void App::UpdCarClrSld(bool upd)
{
	Slider* sl;
	Slv(CarClrH, pSet->gui.car_hue[iCurCar]);
	Slv(CarClrS, pSet->gui.car_sat[iCurCar]);
	Slv(CarClrV, pSet->gui.car_val[iCurCar]);
	Slv(CarClrGloss, powf(pSet->gui.car_gloss[iCurCar], 1.f/ 1.6f));
	Slv(CarClrRefl, pSet->gui.car_refl[iCurCar] /1.4f);
	pSet->game.car_hue[iCurCar] = pSet->gui.car_hue[iCurCar];  // copy to apply
	pSet->game.car_sat[iCurCar] = pSet->gui.car_sat[iCurCar];
	pSet->game.car_val[iCurCar] = pSet->gui.car_val[iCurCar];
	pSet->game.car_gloss[iCurCar] = pSet->gui.car_gloss[iCurCar];
	pSet->game.car_refl[iCurCar] = pSet->gui.car_refl[iCurCar];
	bUpdCarClr = true;
}


//  next/prev in list by key
int App::LNext(MyGUI::MultiList2* lp, int rel, int ofs)
{
	size_t cnt = lp->getItemCount();
	if (cnt==0)  return 0;
	int i = std::max(0, std::min((int)cnt-1, (int)lp->getIndexSelected()+rel ));
	lp->setIndexSelected(i);
	lp->beginToItemAt(std::max(0, i-ofs));  // center
	return i;
}
int App::LNext(MyGUI::MultiList* lp, int rel)
{
	size_t cnt = lp->getItemCount();
	if (cnt==0)  return 0;
	int i = std::max(0, std::min((int)cnt-1, (int)lp->getIndexSelected()+rel ));
	lp->setIndexSelected(i);
	return i;
}
int App::LNext(MyGUI::ListPtr lp, int rel, int ofs)
{
	size_t cnt = lp->getItemCount();
	if (cnt==0)  return 0;
	int i = std::max(0, std::min((int)cnt-1, (int)lp->getIndexSelected()+rel ));
	lp->setIndexSelected(i);
	lp->beginToItemAt(std::max(0, i-ofs));  // center
	return i;
}

void App::LNext(int rel)
{
	//if (!isFocGui || pSet->isMain)  return;
	switch (pSet->inMenu)
	{
	case WND_Game: case WND_Champ:
		switch (mWndTabsGame->getIndexSelected())
		{	case 1:  listTrackChng(trkList,  LNext(trkList, rel, 11));  return;
			case 2:	 listCarChng(carList,    LNext(carList, rel, 5));  return;
			case 4:	 if (rel > 0)  radSimNorm(0);  else  radSimEasy(0);  return;
			case 6:  listChampChng(liChamps, LNext(liChamps, rel, 8));  return;
			case 7:	 listStageChng(liStages, LNext(liStages, rel, 8));  return;
			case 8:	 if (rel > 0)  btnStageNext(0);  else  btnStagePrev(0);  return;
		}	break;
	case WND_Replays:
		listRplChng(rplList,  LNext(rplList, rel, 11));
		break;
	}
}
