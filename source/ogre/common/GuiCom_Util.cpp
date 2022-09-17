#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "CScene.h"
#include "../../vdrift/pathmanager.h"
#ifndef SR_EDITOR
	#include "../CGame.h"
	#include "../CGui.h"
	#include "../settings.h"
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
	,bListTrackU(0)
	,edTrkFind(0), resList(0)
	,txtTracksFAll(0), txtTracksFCur(0)
{
	pSet = app1->pSet;
	sc = app1->scn->sc;
	//mGui = app1->mGui;  set in GuiInit

	pathTrk[0] = PATHMANAGER::Tracks() + "/";
	pathTrk[1] = PATHMANAGER::TracksUser() + "/";
	
	int t,i;
	for (t=0; t < 3; ++t)
	{	
		imgPrv[t] = 0;  imgMini[t] = 0;  imgTer[t] = 0;
		imgMiniPos[t] = 0;  imgMiniRot[t] = 0;
	}
	for (t=0; t < 2; ++t)
	{	
		trkDesc[t] = 0;  trkAdvice[t] = 0;

		for (i=0; i < StTrk; ++i)     stTrk[t][i] = 0;
		for (i=0; i < ImStTrk; ++i) imStTrk[t][i] = 0;
		for (i=0; i < InfTrk; ++i){  infTrk[t][i] = 0;  imInfTrk[t][i] = 0;  }
	}
	
	//  short scenery names  for user tracks
	//  and presets.xml  colors from  sc=".."
	scnN["Jng"] = "Jungle";        scnN["JngD"]= "JungleDark";      scnN["Mos"] = "Moss";          
	scnN["For"] = "Forest";        scnN["ForM"]= "ForestMntn";      scnN["ForY"]= "ForestYellow";
	scnN["Fin"] = "Finland";       scnN["Mud"] = "Mud";             scnN["DesM"]= "DesertMud";
	scnN["Sav"] = "Savanna";       scnN["SavD"]= "SavannaDry";      scnN["Stn"] = "Stone";
	scnN["Grc"] = "Greece";        scnN["GrcW"]= "GreeceWhite";     scnN["GrcR"] = "GreeceRocky";  

	scnN["Atm"] = "Autumn";        scnN["AtmD"]= "AutumnDark";
	scnN["Wnt"] = "Winter";        scnN["Wet"] = "WinterWet";

    scnN["Des"] = "Desert";        scnN["Isl"] = "Island";          scnN["IslD"]= "IslandDark";
	scnN["Aus"] = "Australia";     scnN["Can"] = "Canyon";        
	scnN["Vlc"] = "Volcanic";      scnN["VlcD"]= "VolcanicDark";

	scnN["Tox"] = "Toxic";         scnN["Aln"] = "Alien";
	scnN["Uni"] = "Unidentified";  scnN["Mrs"] = "Mars";            scnN["Cry"] = "Crystals";
	scnN["Sur"] = "Surreal";       scnN["Spc"] = "Space";           scnN["SuSp"] = "SurrealSpace";
	scnN["Oth"] = "Other";         scnN["Blk"] = "BlackDesert";     scnN["Asp"] = "Asphalt";

	//  scenery colors  for track names  *  *  * * * * ** ** ** *** *** ****
	scnClr["Jungle"]       = "#50FF50";  scnClr["JungleDark"]   = "#40C040";  scnClr["Moss"]         = "#70F0B0";
	scnClr["Forest"]       = "#A0C000";  scnClr["ForestMntn"]   = "#A0C080";  scnClr["ForestYellow"] = "#C0C000";
	scnClr["Finland"]      = "#A0E080";  scnClr["Mud"]          = "#A0A000";  scnClr["DesertMud"]    = "#B0B000";
	scnClr["Savanna"]      = "#C0F080";  scnClr["SavannaDry"]   = "#C0D090";  scnClr["Stone"]        = "#A0A0A0";
	scnClr["Greece"]       = "#B0FF00";  scnClr["GreeceWhite"]  = "#C0C0A0";  scnClr["GreeceRocky"]  = "#B0A8A0";

	scnClr["Autumn"]       = "#FFA020";  scnClr["AutumnDark"]   = "#908070";
	scnClr["Winter"]       = "#D0D8D8";  scnClr["WinterWet"]    = "#90D898";

	scnClr["Desert"]       = "#F0F000";  scnClr["Island"]       = "#FFFF80";  scnClr["IslandDark"]   = "#909080";
	scnClr["Australia"]    = "#FF9070";  scnClr["Canyon"]       = "#E0B090";
	scnClr["Volcanic"]     = "#908030";  scnClr["VolcanicDark"] = "#706030";

	scnClr["Toxic"]        = "#60A030";  scnClr["Alien"]        = "#D0FFA0";
	scnClr["Unidentified"] = "#8080D0";  scnClr["Mars"]         = "#A04840";  scnClr["Crystals"]     = "#4090F0";
	scnClr["Surreal"]      = "#F080B0";  scnClr["Space"]        = "#A0B8D0";  scnClr["SurrealSpace"] = "#80B0FF";
	scnClr["Other"]        = "#C0D0E0";  scnClr["BlackDesert"]  = "#202020";  scnClr["Asphalt"]      = "#B0E0E0";
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
				relSize = window->getSize();
			}
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
	{	//  add new in core_language.xml too
		auto AddLang = [&](string s){  languages[s] = TR("#{LANG_"+s+"}");  };
		AddLang("en");
		AddLang("de");
		AddLang("fr");
		AddLang("pl");
		AddLang("ru");
		AddLang("fi");
		AddLang("pt_BR");
		AddLang("it");
		AddLang("ro");
		AddLang("sk");
		AddLang("cs");
		AddLang("es_AR");
		AddLang("nl_NL");
		AddLang("hu_HU");
	}
	ComboBoxPtr combo = fCmb("Lang");
	if (!combo)  return;
	combo->eventComboChangePosition += newDelegate(this, &CGuiCom::comboLanguage);
	for (auto it = languages.cbegin(); it != languages.end(); ++it)
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
	
	for (auto it = languages.cbegin(); it != languages.end(); ++it)
	{
		if (it->second == sel)
			pSet->language = it->first;
	}
	LanguageManager::getInstance().setCurrentLanguage(pSet->language);

	// todo: fix, without restart
	//  reinit gui
	#ifndef SR_EDITOR
	//bGuiReinit = true;
	#endif
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
	float sizes[cnt] = {28.f, 33.f, 38.f};  // par
	
	String inf;
	for (int i=0; i < cnt; ++i)
	{
		//  del old
		const string name = names[i];
		if (mgr.isExist(name))
			mgr.removeByName(name);

		//  setup font				   // par
		float size = sizes[i];  // less for low screen res
		size *= max(0.55f, min(1.2f, (pSet->windowy - 600.f) / 600.f));
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
		{	const auto& vv = bfont->getCodePointRanges();
			for (auto it = vv.cbegin(); it != vv.cend(); ++it)
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
		{	const auto& vv = bfont->getCodePointRanges();
			for (auto it = vv.cbegin(); it != vv.cend(); ++it)
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
