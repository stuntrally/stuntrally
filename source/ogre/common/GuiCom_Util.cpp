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
#include <MyGUI_FactoryManager.h>
#include <MyGUI_ResourceTrueTypeFont.h>
using namespace MyGUI;
using namespace Ogre;
using namespace std;


CGuiCom::CGuiCom(App* app1)
	:app(app1), pSet(0), sc(0),	mGui(0)
	,bGuiReinit(0)
	,mToolTip(0), mToolTipTxt(0)
	,bnQuit(0)
	,trkList(0), imgTrkIco1(0),imgTrkIco2(0)
	, bListTrackU(0)
	,edTrkFind(0), resList(0)
	,txtTracksFAll(0), txtTracksFCur(0)
{
	pSet = app1->pSet;
	sc = app1->scn->sc;
	//mGui = app1->mGui;  set in GuiInit

	pathTrk[0] = PATHMANAGER::Tracks() + "/";
	pathTrk[1] = PATHMANAGER::TracksUser() + "/";
	
	int t,i;
	for (t=0; t < 2; ++t)
	{	
		trkDesc[t] = 0;
		imgPrv[t] = 0;  imgMini[t] = 0;  imgTer[t] = 0;
		imgMiniPos[t] = 0;  imgMiniRot[t] = 0;

		for (i=0; i < StTrk; ++i)  stTrk[t][i] = 0;
		for (i=0; i < 4; ++i)  imStTrk[t][i] = 0;
		for (i=0; i < InfTrk; ++i){  infTrk[t][i] = 0;  imInfTrk[t][i] = 0;  }
	}
	
	//  short scenery names  for sc clr in presets.xml
	scnN["AP"]= "Asphalt";
	scnN["J"] = "Jungle";        scnN["JD"]= "JungleDark";
	scnN["D"] = "Desert";        scnN["DM"]= "DesertMud";
	scnN["S"] = "Savanna";       scnN["SD"]= "SavannaDry";
	scnN["W"] = "Winter";        scnN["WW"]= "WinterWet";
	scnN["F"] = "Forest";        scnN["FM"]= "ForestMntn";
	scnN["E"] = "Finland";       scnN["FY"]= "ForestYellow";
	scnN["I"] = "Island";        scnN["ID"]= "IslandDark";
	scnN["M"] = "Mud";           scnN["A"] = "Australia";
	scnN["G"] = "Greece";        scnN["C"] = "Canyon";
	scnN["T"] = "Autumn";        scnN["TD"]= "AutumnDark";
	scnN["O"] = "Moss";          
	scnN["V"] = "Volcanic";      scnN["VD"]= "VolcanicDark";
	scnN["X"] = "Unidentified";  scnN["R"] = "Mars";
	scnN["Y"] = "Crystals";      scnN["GW"]= "GreeceWhite";
	scnN["L"] = "Alien";         scnN["N"] = "Other";
	scnN["P"] = "Stone";         scnN["H"] = "Space";

	//  short scenery names  for user tracks, ^-
	scnN["Jng"] = "Jungle";        scnN["JngD"]= "JungleDark";
	scnN["Des"] = "Desert";        scnN["DesM"]= "DesertMud";
	scnN["Sav"] = "Savanna";       scnN["SavD"]= "SavannaDry";
	scnN["Wnt"] = "Winter";        scnN["Wet"]= "WinterWet";
	scnN["For"] = "Forest";        scnN["ForM"]= "ForestMntn";
	scnN["ForY"]= "ForestYellow";  scnN["Fin"] = "Finland";
	scnN["Isl"] = "Island";        scnN["IslD"]= "IslandDark";
	scnN["Mud"] = "Mud";           scnN["Aus"] = "Australia";
	scnN["Grc"] = "Greece";        scnN["Can"] = "Canyon";
	scnN["Atm"] = "Autumn";        scnN["AtmD"]= "AutumnDark";
	scnN["Mos"] = "Moss";          
	scnN["Vlc"] = "Volcanic";      scnN["VlcD"]= "VolcanicDark";
	scnN["Uni"] = "Unidentified";  scnN["Mrs"] = "Mars";
	scnN["Cry"] = "Crystals";      scnN["GrcW"]= "GreeceWhite";
	scnN["Tox"] = "Toxic";         scnN["Sur"] = "Surreal";
	scnN["Aln"] = "Alien";         scnN["Oth"] = "Other";
	scnN["Stn"] = "Stone";         scnN["Spc"] = "Space";
	scnN["Blk"] = "BlackDesert";

	//  scenery colors  for track names
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
	scnClr["Crystals"]     = "#4090F0";  scnClr["GreeceWhite"]  = "#C0C0A0";
	scnClr["Toxic"]        = "#60A030";  scnClr["Surreal"]      = "#F0B0D0";
	scnClr["Alien"]        = "#D0FFA0";  scnClr["Other"]        = "#C0D0E0";
	scnClr["Stone"]        = "#A0A0A0";  scnClr["Space"]        = "#A0B8D0";
	scnClr["BlackDesert"]  = "#202020";
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
	InputManager::getInstance().injectMouseMove(xm, ym, 0);
}

void CGuiCom::btnQuit(WP)
{
	app->mShutDown = true;
}

//  unfocus lists (would have double up/dn key input)
void CGuiCom::UnfocusLists()
{
	WP w = InputManager::getInstance().getKeyFocusWidget();
	while (w)
	{
		//LogO(w->getTypeName() +" "+ w->getName());

		#ifdef SR_EDITOR
		if (w == (WP)trkList  || w == (WP)app->gui->liSky || w == (WP)app->gui->liTex ||
			w == (WP)app->gui->liGrs || w == (WP)app->gui->liVeg || w == (WP)app->gui->liRd)
		#else
		if (w == (WP)trkList  || (app && app->gui && (
			w == (WP)app->gui->carList  || w == (WP)app->gui->liChalls ||
			w == (WP)app->gui->liChamps || w == (WP)app->gui->liStages || w == (WP)app->gui->rplList)) )
		#endif
		{
			InputManager::getInstance().resetKeyFocusWidget();
			return;
		}
		w = w->getParent();
	}
}

TabPtr CGuiCom::FindSubTab(WP tab)
{
	TabPtr  sub = 0;  // 0 for not found
	size_t s = tab->getChildCount();
	for (size_t n = 0; n < s; ++n)
	{
		WP wp = tab->getChildAt(n);
		if (StringUtil::startsWith(wp->getName(), "SubTab"))
			sub = (TabPtr)wp;
	}
	return sub;
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
		string relativeTo = wp->getUserString("RelativeTo");

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

void CGuiCom::notifyToolTip(WP wp, const ToolTipInfo &info)
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
	{
		mToolTip->setSize(320, 128);  // start size for wrap
		mToolTipTxt->setSize(320, 128);

		String s = TR(wp->getUserString("tip"));
		mToolTipTxt->setCaption(s);
		const IntSize &si = mToolTipTxt->getTextSize();

		mToolTip->setSize(si.width +8, si.height +8);
		mToolTipTxt->setSize(si.width, si.height);
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
	if (languages.empty())
	{	languages["en"] = TR("#{LANG_EN}");  languages["de"] = TR("#{LANG_DE}");
		languages["fr"] = TR("#{LANG_FR}");  languages["pl"] = TR("#{LANG_PL}");
		languages["ru"] = TR("#{LANG_RU}");  languages["fi"] = TR("#{LANG_FI}");
		languages["pt"] = TR("#{LANG_PT}");  languages["ro"] = TR("#{LANG_RO}");
		languages["it"] = TR("#{LANG_IT}");  languages["sk"] = TR("#{LANG_SK}");
		languages["es"] = TR("#{LANG_ES}");  languages["cs"] = TR("#{LANG_CS}");
	}
	ComboBoxPtr combo = fCmb("Lang");
	if (!combo)  return;
	combo->eventComboChangePosition += newDelegate(this, &CGuiCom::comboLanguage);
	for (std::map<string, UString>::const_iterator it = languages.begin();
		it != languages.end(); ++it)
	{
		combo->addItem(it->second);
		if (it->first == pSet->language)
			combo->setIndexSelected(combo->getItemCount()-1);
	}
}

void CGuiCom::comboLanguage(ComboBox* wp, size_t val)
{
	if (val == ITEM_NONE)  return;
	UString sel = wp->getItemNameAt(val);
	
	for (std::map<string, UString>::const_iterator it = languages.begin();
		it != languages.end(); ++it)
	{
		if (it->second == sel)
			pSet->language = it->first;
	}
	LanguageManager::getInstance().setCurrentLanguage(pSet->language);

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
	#ifndef SR_EDITOR  ///_  game tab change
	if (tab == app->mWndTabsGame)
	{
		if (id == TAB_Car)
			app->gui->CarListUpd();  //  off filtering by chall
	
		app->mWndTrkFilt->setVisible(false);  //

		if (id == TAB_Multi)
		{	//  back to mplr tab, upload game info
									//_ only for host..
			if (app->mMasterClient && app->gui->valNetPassword->getVisible())
			{	app->gui->uploadGameInfo();
				app->gui->updateGameInfoGUI();
			}
			//- app->gui->evBtnNetRefresh(0);  // upd games list (don't, breaks game start)
		}
	}
	#endif

	if (id != 0)  return;  // <back
	tab->setIndexSelected(1);  // dont switch to 0
	pSet->isMain = true;
	app->gui->toggleGui(false);  // back to main
}


///  create fonts
//----------------------------------------------------------------------------------------------------------------
void CGuiCom::CreateFonts()
{
	MyGUI::ResourceManager& mgr = MyGUI::ResourceManager::getInstance();
	IResource* resource = mgr.findByName("hud.text");  // based on this font
	ResourceTrueTypeFont* bfont = resource != nullptr ? resource->castType<ResourceTrueTypeFont>(false) : 0;
	if (!bfont)  LogO("!!Error: Can't find font: hud.text");

	const int cnt = 3;
	string names[cnt] = {"font.small","font.normal","font.big"};
	float sizes[cnt] = {26.f, 30.f, 34.f};  // par
	
	String inf;
	for (int i=0; i < cnt; ++i)
	{
		//  del old
		const string name = names[i];
		if (mgr.isExist(name))
			mgr.removeByName(name);

		//  setup font				   // par
		float size = sizes[i] * (1.f - 1.5f * (GetGuiMargin(2000) - GetGuiMargin(pSet->windowy)));
		inf += name+"  "+fToStr(size,1,3)+"  ";

		//  create
	#if 0  //  mygui from svn
		string cat = mgr.getCategoryName();   // createObject("Resource", "ResourceTrueTypeFont"));
		ResourceTrueTypeFont* font = FactoryManager::getInstance().createObject<ResourceTrueTypeFont>(cat);
		font->setResourceName(name);

		font->setSource("DejaVuLGCSans.ttf");
		font->setSize(size);  font->setResolution(50);  font->setAntialias(false);  //font->setHinting("");
		font->setTabWidth(8);  font->setDistance(4);  font->setOffsetHeight(0);
		//font->setSubstituteCode(_data->getPropertyValue<int>("SubstituteCode"));

		//  char ranges
		if (bfont)
		{	const std::vector<pair<Char, Char> >& vv = bfont->getCodePointRanges();
			for (std::vector<pair<Char, Char> >::const_iterator it = vv.begin(); it != vv.end(); ++it)
			if ((*it).first > 10 && (*it).first < 10000)
			{	//LogO("aa "+toStr((*it).first)+" "+toStr((*it).second));
				font->addCodePointRange((*it).first, (*it).second);  }
		}else
			font->addCodePointRange(33,255);

		font->initialise();
	#else
		ResourceTrueTypeFont* font = (ResourceTrueTypeFont*)FactoryManager::getInstance().createObject("Resource", "ResourceTrueTypeFont");

		//  Loading from XML, data members are private in MyGUI 3.2.0
		xml::Document doc;
		xml::ElementPtr root = doc.createRoot("ResourceTrueTypeFont"), e;
		root->addAttribute("name", name);

		#define AddE(key, val)  e = root->createChild("Property");  e->addAttribute("key", key);  e->addAttribute("value", val)
		AddE("Source", "DejaVuLGCSans.ttf");
		AddE("Size", toStr(size));  AddE("Resolution", "50");  AddE("Antialias", "false");
		AddE("TabWidth", "8");  AddE("Distance", "4");  AddE("OffsetHeight", "0");

		xml::ElementPtr codes = root->createChild("Codes"), c;
		//  char ranges
		if (bfont)
		{	const std::vector<pair<Char, Char> >& vv = bfont->getCodePointRanges();
			for (std::vector<pair<Char, Char> >::const_iterator it = vv.begin(); it != vv.end(); ++it)
			if ((*it).first > 10 && (*it).first < 10000)
			{
				c = codes->createChild("Code");
				c->addAttribute("range", toStr((*it).first)+" "+toStr((*it).second));
			}
		}else
		{	c = codes->createChild("Code");
			c->addAttribute("range", "33 255");
		}
		//doc.save(string("aaa.txt"));
		font->deserialization(root, Version(3,2,0));
	#endif
		//  add
		mgr.addResource(font);
	}

	LogO("-- Font sizes:  "+inf);
}
