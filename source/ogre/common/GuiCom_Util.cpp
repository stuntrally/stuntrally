#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "CScene.h"
#include "../../vdrift/pathmanager.h"
#ifndef SR_EDITOR
	#include "../CGame.h"
	#include "../CGui.h"
	#include "../../vdrift/settings.h"
#else
	#include "../../editor/CApp.h"
	#include "../../editor/CGui.h"
	#include "../../editor/settings.h"
#endif
#include "../../sdl4ogre/sdlinputwrapper.hpp"
#include <MyGUI_InputManager.h>
#include <MyGUI_Widget.h>
#include <MyGUI_EditBox.h>
#include <MyGUI_ComboBox.h>
#include <MyGUI_Gui.h>
#include <MyGUI_Window.h>
#include <MyGUI_TabControl.h>
using namespace MyGUI;
using namespace Ogre;


CGuiCom::CGuiCom(App* app1)
	:app(app1), pSet(0), sc(0),	mGui(0)
	,bGuiReinit(0)
	,mToolTip(0), mToolTipTxt(0)
	,bnQuit(0)
	,trkList(0), imgTrkIco1(0),imgTrkIco2(0)
	, bListTrackU(0)
	,edTrkFind(0), resList(0)
{
	pSet = app1->pSet;
	sc = app1->scn->sc;
	//mGui = app1->mGui;  set in GuiInit

	pathTrk[0] = PATHMANAGER::Tracks() + "/";
	pathTrk[1] = PATHMANAGER::TracksUser() + "/";
	
	int t,i;
	for (t=0; t<2; ++t)
	{	
		trkDesc[t] = 0;
		imgPrv[t] = 0; imgMini[t] = 0; imgTer[t] = 0;

		for (i=0; i < StTrk; ++i)  stTrk[t][i] = 0;
		for (i=0; i < InfTrk; ++i)  infTrk[t][i] = 0;
	}
	
	//  short scenery names
	scnN["AP"]= "Asphalt";
	scnN["J"] = "Jungle";        scnN["JD"]= "JungleDark";
	scnN["D"] = "Desert";        scnN["DM"]= "DesertMud";
	scnN["S"] = "Savanna";       scnN["SD"]= "SavannaDry";
	scnN["W"] = "Winter";        scnN["WW"]= "WinterWet";
	scnN["F"] = "Forest";        scnN["FM"]= "ForestMntn";
	scnN["FY"]= "ForestYellow";  scnN["E"] = "Finland";
	scnN["I"] = "Island";        scnN["ID"]= "IslandDark";
	scnN["M"] = "Mud";           scnN["A"] = "Australia";
	scnN["G"] = "Greece";        scnN["C"] = "Canyon";
	scnN["T"] = "Autumn";        scnN["TD"]= "AutumnDark";
	scnN["O"] = "Moss";          
	scnN["V"] = "Volcanic";      scnN["VD"]= "VolcanicDark";
	scnN["X"] = "Unidentified";  scnN["R"] = "Mars";
	//  scenery colors for track names
	scnClr["Asphalt"]      = "#B0E0E0";
	scnClr["Jungle"]       = "#50FF50";  scnClr["JungleDark"]   = "#40C040";
	scnClr["Desert"]       = "#F0F000";  scnClr["DesertMud"]    = "#B0B000";
	scnClr["Savanna"]      = "#C0F080";  scnClr["SavannaDry"]   = "#C0D090";
	scnClr["Winter"]       = "#D0D8D8";  scnClr["WinterWet"]    = "#90D898";
	scnClr["Forest"]       = "#A0C000";  scnClr["ForestMntn"]   = "#A0C080";
	scnClr["ForestYellow"] = "#C0C000";  scnClr["Finland"]      = "#A0E080";
	scnClr["Island"]       = "#FFFF80";  scnClr["IslandDark"]   = "#909080";
	scnClr["Mud"]          = "#A0A000";  scnClr["Australia"]    = "#FFA080";
	scnClr["Greece"]       = "#B0FF00";  scnClr["Canyon"]       = "#E0B090";
	scnClr["Autumn"]       = "#FFA020";  scnClr["AutumnDark"]   = "#908070";
	scnClr["Moss"]         = "#70F0B0";
	scnClr["Volcanic"]     = "#908030";  scnClr["VolcanicDark"] = "#706030";
	scnClr["Unidentified"] = "#8080D0";  scnClr["Mars"]         = "#A04840";
}

int TrkL::idSort = 0;

TrkL::TrkL() :
	ti(0)
{	}



//  Util
//----------------------------------------------------------------------------------------------------------------
void CGuiCom::GuiCenterMouse()
{	
	int xm = app->mWindow->getWidth()/2, ym = app->mWindow->getHeight()/2;

	app->mInputWrapper->warpMouse(xm, ym);
	MyGUI::InputManager::getInstance().injectMouseMove(xm, ym, 0);
}

void CGuiCom::btnQuit(WP)
{
	app->mShutDown = true;
}

//  unfocus lists (would have double up/dn key input)
void CGuiCom::UnfocusLists()
{
	WP w = MyGUI::InputManager::getInstance().getKeyFocusWidget();
	while (w)
	{
		//LogO(wg->getTypeName() +" "+ wg->getName());
		w = w->getParent();

		#ifdef SR_EDITOR
		if (w == (WP)trkList)
		#else
		if (w == (WP)trkList  || (app && app->gui && (
			w == (WP)app->gui->carList  || w == (WP)app->gui->liChalls ||
			w == (WP)app->gui->liChamps || w == (WP)app->gui->liStages || w == (WP)app->gui->rplList)) )
		#endif
		{
			MyGUI::InputManager::getInstance().resetKeyFocusWidget();
			return;
	}	}
}


//  Resize MyGUI
//-----------------------------------------------------------------------------------

void CGuiCom::SizeGUI()
{
	#ifndef SR_EDITOR
	app->baseSizeGui();
	#endif
	
	//  call recursive method for all root widgets
	for (VectorWidgetPtr::iterator it = app->vwGui.begin(); it != app->vwGui.end(); ++it)
		doSizeGUI((*it)->getEnumerator());
}

void CGuiCom::doSizeGUI(EnumeratorWidgetPtr widgets)
{
	while (widgets.next())
	{
		WP wp = widgets.current();
		std::string relativeTo = wp->getUserString("RelativeTo");

		if (relativeTo != "")
		{
			//  position & size relative to the widget specified in "RelativeTo" property (or full screen)
			IntSize relSize;
			if (relativeTo == "Screen")
				relSize = IntSize(app->mWindow->getWidth(), app->mWindow->getHeight());
			else
			{	WP window = fWP(relativeTo);
				relSize = window->getSize();  }
			
			//  retrieve original size & pos
			IntPoint origPos;  IntSize origSize;
			origPos.left = s2i(wp->getUserString("origPosX"));
			origPos.top  = s2i(wp->getUserString("origPosY"));
			origSize.width  = s2i(wp->getUserString("origSizeX"));
			origSize.height = s2i(wp->getUserString("origSizeY"));
			
			//  calc & apply new size & pos
			float sx = relSize.width / 800.f, sy = relSize.height / 600.f;
			wp->setPosition(IntPoint( int(origPos.left * sx), int(origPos.top * sy) ));
			wp->setSize(IntSize(    int(origSize.width * sx), int(origSize.height * sy) ));
		}
		
		doSizeGUI(wp->getEnumerator());
	}
}

void CGuiCom::setOrigPos(WP wp, const char* relToWnd)
{
	if (!wp)  return;
	wp->setUserString("origPosX", toStr(wp->getPosition().left));
	wp->setUserString("origPosY", toStr(wp->getPosition().top));
	wp->setUserString("origSizeX", toStr(wp->getSize().width));
	wp->setUserString("origSizeY", toStr(wp->getSize().height));
	wp->setUserString("RelativeTo", relToWnd);
}


///  Tooltips
//----------------------------------------------------------------------------------------------------------------
void CGuiCom::GuiInitTooltip()
{
	mToolTip = fWP("ToolTip");
	mToolTip->setVisible(false);
	mToolTipTxt = mToolTip->getChildAt(0)->castType<Edit>();

	for (VectorWidgetPtr::iterator it = app->vwGui.begin(); it != app->vwGui.end(); ++it)
		setToolTips((*it)->getEnumerator());
}

void CGuiCom::setToolTips(EnumeratorWidgetPtr widgets)
{
	while (widgets.next())
	{
		WP wp = widgets.current();
		wp->setAlign(Align::Default);
				
		IntPoint origPos = wp->getPosition();
		IntSize origSize = wp->getSize();
		
		wp->setUserString("origPosX", toStr(origPos.left));
		wp->setUserString("origPosY", toStr(origPos.top));
		wp->setUserString("origSizeX", toStr(origSize.width));
		wp->setUserString("origSizeY", toStr(origSize.height));

		//  find parent window
		WP p = wp->getParent();
		while (p)
		{
			if (p->getTypeName() == "Window")
			{
				if (p->getUserString("NotSized").empty())
					wp->setUserString("RelativeTo", p->getName());
				break;
			}
			p = p->getParent();
		}
		
		bool tip = wp->isUserString("tip");
		if (tip)  // if has tooltip string
		{
			// needed for translation
			wp->setUserString("tip", LanguageManager::getInstance().replaceTags(wp->getUserString("tip")));
			wp->setNeedToolTip(true);
			wp->eventToolTip += newDelegate(this, &CGuiCom::notifyToolTip);
		}
		//LogO(wp->getName() + (tip ? "  *" : ""));
		setToolTips(wp->getEnumerator());
	}
}

void CGuiCom::notifyToolTip(Widget *sender, const ToolTipInfo &info)
{
	if (!mToolTip)  return;

	#ifndef SR_EDITOR
	if (!app->isFocGui)
	#else
	if (!app->bGuiFocus)
	#endif
	{	mToolTip->setVisible(false);
		return;
	}
	if (info.type == ToolTipInfo::Show)
	{	// TODO: Tooltip isn't resizing properly ..
		mToolTip->setSize(320, 96);  // start size for wrap
		String s = TR(sender->getUserString("tip"));
		mToolTipTxt->setCaption(s);
		const IntSize &textsize = mToolTipTxt->getTextSize();
		mToolTip->setSize(textsize.width*1.5, textsize.height*1.5);
		mToolTip->setVisible(true);
		boundedMove(mToolTip, info.point);
	}
	else if (info.type == ToolTipInfo::Hide)
		mToolTip->setVisible(false);
}

//  Move a widget to a point while making it stay in the viewport.
void CGuiCom::boundedMove(Widget* moving, const IntPoint& point)
{
	const IntPoint offset(20, 20);  // mouse cursor
	IntPoint p = point + offset;

	const IntSize& size = moving->getSize();
	
	int w = app->mWindow->getWidth();
	int h = app->mWindow->getHeight();
	
	if (p.left + size.width > w)  p.left = w - size.width;
	if (p.top + size.height > h)  p.top = h - size.height;
			
	moving->setPosition(p);
}


//  Languages combo
//----------------------------------------------------------------------------------------------------------------
void CGuiCom::GuiInitLang()
{
	languages["en"] = TR("#{LANG_EN}");  languages["de"] = TR("#{LANG_DE}");
	languages["fr"] = TR("#{LANG_FR}");  languages["pl"] = TR("#{LANG_PL}");
	languages["ru"] = TR("#{LANG_RU}");  languages["fi"] = TR("#{LANG_FI}");
	languages["pt"] = TR("#{LANG_PT}");  languages["ro"] = TR("#{LANG_RO}");
	languages["it"] = TR("#{LANG_IT}");

	ComboBoxPtr combo = fCmb("Lang");
	if (!combo)  return;
	combo->eventComboChangePosition += newDelegate(this, &CGuiCom::comboLanguage);
	for (std::map<std::string, UString>::const_iterator it = languages.begin();
		it != languages.end(); ++it)
	{
		combo->addItem(it->second);
		if (it->first == pSet->language)
			combo->setIndexSelected(combo->getItemCount()-1);
	}
}

void CGuiCom::comboLanguage(MyGUI::ComboBox* wp, size_t val)
{
	if (val == MyGUI::ITEM_NONE)  return;
	MyGUI::UString sel = wp->getItemNameAt(val);
	
	for (std::map<std::string, MyGUI::UString>::const_iterator it = languages.begin();
		it != languages.end(); ++it)
	{
		if (it->second == sel)
			pSet->language = it->first;
	}
	MyGUI::LanguageManager::getInstance().setCurrentLanguage(pSet->language);

	//  reinit gui
	#ifndef SR_EDITOR  //todo: fix in ed
	bGuiReinit = true;
	#endif
}


//  Main menu
//----------------------------------------------------------------------------------------------------------------
#ifdef SR_EDITOR
	#define cntMain WND_ALL
#else
	#define cntMain ciMainBtns
#endif

void CGuiCom::InitMainMenu()
{
	Btn btn;
	for (int i=0; i < cntMain; ++i)
	{
		const String s = toStr(i);
		app->mWndMainPanels[i] = fWP("PanMenu"+s);
		BtnC("BtnMenu"+s, btnMainMenu);  app->mWndMainBtns[i] = btn;
	}

	//  center
	int wx = app->mWindow->getWidth(), wy = app->mWindow->getHeight();
	Wnd wnd = app->mWndMain;

	IntSize w = wnd->getSize();
	wnd->setPosition((wx-w.width)*0.5f, (wy-w.height)*0.5f);
}

void CGuiCom::btnMainMenu(WP wp)
{
	for (int i=0; i < cntMain; ++i)
		if (wp == app->mWndMainBtns[i])
		{
			pSet->isMain = false;
			pSet->inMenu = i;
			app->gui->toggleGui(false);
			return;
		}
}

void CGuiCom::tabMainMenu(Tab tab, size_t id)
{
	#ifndef SR_EDITOR
	if (tab == app->mWndTabsGame && id == TAB_Car)
		app->gui->CarListUpd();  // off filtering
	#endif

	if (id != 0)  return;  // <back
	tab->setIndexSelected(1);  // dont switch to 0
	pSet->isMain = true;
	app->gui->toggleGui(false);  // back to main
}
