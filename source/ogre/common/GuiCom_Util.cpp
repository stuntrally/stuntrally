#include "pch.h"
#include "Def_Str.h"
#include "Gui_Def.h"
#include "GuiCom.h"
#include "../../vdrift/pathmanager.h"
#ifndef SR_EDITOR
	#include "../CGame.h"
	//#include "../CGui.h"
	#include "../../vdrift/settings.h"
#else
	#include "../../editor/CApp.h"
	//#include "../../editor/CGui.h"
	#include "../../editor/settings.h"
#endif
#include "../../sdl4ogre/sdlinputwrapper.hpp"
#include <MyGUI_InputManager.h>
#include <MyGUI_Widget.h>
#include <MyGUI_EditBox.h>
#include <MyGUI_ComboBox.h>
#include <MyGUI_Gui.h>
using namespace MyGUI;
using namespace Ogre;


CGuiCom::CGuiCom(App* app1)
	:app(app1)
	,bGuiReinit(0)
	,mToolTip(0), mToolTipTxt(0)
	,bnQuit(0)
	,trkList(0), imgTrkIco1(0),imgTrkIco2(0)
	, bListTrackU(0)
	,edTrkFind(0), resList(0)
{
	pSet = app1->pSet;
	sc = app1->sc;
	mGui = app1->mGui;

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
}


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
	Widget* w = MyGUI::InputManager::getInstance().getKeyFocusWidget();
	while (w)
	{
		//LogO(wg->getTypeName() +" "+ wg->getName());
		w = w->getParent();

		#ifdef SR_EDITOR
		if (w == (Widget*)trkList)
		#else
		if (w == (Widget*)trkList  || w == (Widget*)carList  || w == (Widget*)liChalls ||
			w == (Widget*)liChamps || w == (Widget*)liStages || w == (Widget*)rplList)
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
		WidgetPtr wp = widgets.current();
		std::string relativeTo = wp->getUserString("RelativeTo");

		if (relativeTo != "")
		{
			// position & size relative to the widget specified in "RelativeTo" property (or full screen)
			IntSize relativeSize;
			if (relativeTo == "Screen")
				relativeSize = IntSize(app->mWindow->getWidth(), app->mWindow->getHeight());
			else
			{
				WidgetPtr window = app->mGui->findWidget<Widget>(relativeTo);
				relativeSize = window->getSize();
			}
			
			// retrieve original size & pos
			IntPoint origPos;  IntSize origSize;
			origPos.left = s2i( wp->getUserString("origPosX") );
			origPos.top = s2i( wp->getUserString("origPosY") );
			origSize.width = s2i( wp->getUserString("origSizeX") );
			origSize.height = s2i( wp->getUserString("origSizeY") );
			
			// calc new size & pos
			const IntPoint& newPos = IntPoint(
				int(origPos.left * (float(relativeSize.width) / 800)),
				int(origPos.top * (float(relativeSize.height) / 600)) );
			
			const IntSize& newScale = IntSize(
				int(origSize.width * (float(relativeSize.width) / 800)),
				int(origSize.height * (float(relativeSize.height) / 600)) );
			
			// apply
			wp->setPosition(newPos);
			wp->setSize(newScale);
		}
		
		doSizeGUI(wp->getEnumerator());
	}
}


///  Tooltips
//----------------------------------------------------------------------------------------------------------------
void CGuiCom::GuiInitTooltip()
{
	mToolTip = mGui->findWidget<Widget>("ToolTip");
	mToolTip->setVisible(false);
	mToolTipTxt = mToolTip->getChildAt(0)->castType<Edit>();
}

void CGuiCom::setToolTips(EnumeratorWidgetPtr widgets)
{
	while (widgets.next())
	{
		WidgetPtr wp = widgets.current();
		wp->setAlign(Align::Default);
				
		IntPoint origPos = wp->getPosition();
		IntSize origSize = wp->getSize();
		
		wp->setUserString("origPosX", toStr(origPos.left));
		wp->setUserString("origPosY", toStr(origPos.top));
		wp->setUserString("origSizeX", toStr(origSize.width));
		wp->setUserString("origSizeY", toStr(origSize.height));
		
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
	{	mToolTip->setVisible(false);
		return;  }
	#endif

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

	ComboBoxPtr combo = app->mGui->findWidget<ComboBox>("Lang");
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
	bGuiReinit = true;
}
