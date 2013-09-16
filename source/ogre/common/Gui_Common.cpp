#include "pch.h"
#include "Defines.h"
#include "Gui_Def.h"
#include "../../road/Road.h"
#include "../../vdrift/pathmanager.h"
#ifndef SR_EDITOR
	#include "../../vdrift/game.h"
	#include "../CGame.h"
	#include "../CGui.h"
	#include "../SplitScreen.h"
#else
	#include "../../editor/CApp.h"
	#include "../../editor/CGui.h"
#endif
#include <OgreRoot.h>
#include <OgreMaterialManager.h>
#include <OgreSceneManager.h>
#include <OgreTerrain.h>
#include <OgreRenderWindow.h>
#include "Slider.h"
using namespace MyGUI;
using namespace Ogre;
using namespace std;


#include "../shiny/Main/Factory.hpp"
#include "../sdl4ogre/sdlinputwrapper.hpp"



///  Gui Init  [Graphics]
//----------------------------------------------------------------------------------------------------------------

//  textures
void CGui::comboTexFilter(CMB)
{
	TextureFilterOptions tfo;							
	switch (val)  {
		case 0:	 tfo = TFO_BILINEAR;	break;
		case 1:	 tfo = TFO_TRILINEAR;	break;
		case 2:	 tfo = TFO_ANISOTROPIC;	break;	}
	MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);  //if (bGI)
}

void CGui::slAnisotropy(SL)
{
	int v = val * 16.f +slHalf;  if (bGI)  {
		MaterialManager::getSingleton().setDefaultAnisotropy(v);	pSet->anisotropy = v;  }
	if (valAnisotropy)	valAnisotropy->setCaption(toStr(v));
}

//  view dist
void CGui::slViewDist(SL)
{
	Real v = 50.f + 19950.f * powf(val, 2.f);
	Vector3 sc = v*Vector3::UNIT_SCALE;

	/*SceneNode* nskb = ap->mSceneMgr->getSkyBoxNode();
	if (nskb)  nskb->setScale(sc*0.58f);
	else*/  if (app->ndSky)  app->ndSky->setScale(sc);

	if (bGI)  pSet->view_distance = v;
	if (valViewDist){	valViewDist->setCaption(fToStr(v*0.001f, 1,4)+" km");  }

	// Set new far clip distance for all cams
	#ifndef SR_EDITOR
	/*?if (bGI)*/  app->mSplitMgr->UpdateCamDist();
	#else
	app->mCamera->setFarClipDistance(pSet->view_distance*1.1f);
	#endif
}

//  ter detail
void CGui::slTerDetail(SL)
{
	Real v = 2.f * val;  if (bGI)  {  pSet->terdetail = v;  app->UpdTerErr();  }
	if (valTerDetail){	valTerDetail->setCaption(fToStr(v, 2,4));  }
}

//  ter dist
void CGui::slTerDist(SL)
{
	Real v = 2000.f * powf(val, 2.f);  if (bGI)  {  pSet->terdist = v;
		if (app->mTerrainGlobals)
			app->mTerrainGlobals->setCompositeMapDistance(v);  }
	if (valTerDist){	valTerDist->setCaption(fToStr(v,0,4)+" m");  }
}

//  road dist
void CGui::slRoadDist(SL)
{
	Real v = 4.f * powf(val, 2.f);  if (bGI)  pSet->road_dist = v;
	if (valRoadDist){	valRoadDist->setCaption(fToStr(v,2,5));  }
}


//  trees/grass
void CGui::slTrees(SL)
{
	Real v = 4.f * powf(val, 2.f);  if (bGI)  pSet->gui.trees = v;
	if (valTrees){	valTrees->setCaption(fToStr(v,2,4));  }
}
void CGui::slGrass(SL)
{
	Real v = 4.f * powf(val, 2.f);  if (bGI)  pSet->grass = v;
	if (valGrass){	valGrass->setCaption(fToStr(v,2,4));  }
}

void CGui::slTreesDist(SL)
{
	Real v = 0.5f + 6.5f * powf(val, 2.f);  if (bGI)  pSet->trees_dist = v;
	if (valTreesDist){	valTreesDist->setCaption(fToStr(v,2,4));  }
}
void CGui::slGrassDist(SL)
{
	Real v = 0.5f + 6.5f * powf(val, 2.f);  if (bGI)  pSet->grass_dist = v;
	if (valGrassDist){	valGrassDist->setCaption(fToStr(v,2,4));  }
}

void CGui::btnTrGrReset(WP wp)
{
	Slider* sl;  float v;
	#define setSld(name)  sl##name(0,v);  \
		sl = (Slider*)app->mWndOpts->findWidget(#name);  if (sl)  sl->setValue(v);
	v = powf(1.5f /4.f, 0.5f);
	setSld(Trees);
	v = powf(1.f /4.f, 0.5f);
	setSld(Grass);
	v = powf((1.f-0.5f) /6.5f, 0.5f);
	setSld(TreesDist);
	v = powf((2.f-0.5f) /6.5f, 0.5f);
	setSld(GrassDist);
}

void CGui::chkUseImposters(WP wp)
{
	ChkEv(use_imposters);
}
void CGui::chkImpostorsOnly(WP wp)
{
	ChkEv(imposters_only);
}

void CGui::slShaders(SL)
{
	float v = val;  if (bGI)  pSet->shaders = v;
	if (valShaders)
	{	valShaders->setCaption(TR("#{GraphicsAll_Lowest}"));
		if (v > 0.2)  valShaders->setCaption(TR("#{GraphicsAll_Low}"));
		if (v > 0.4)  valShaders->setCaption(TR("#{GraphicsAll_Medium}"));
		if (v > 0.6)  valShaders->setCaption(TR("#{GraphicsAll_High}"));
		if (v > 0.8)  valShaders->setCaption(TR("#{GraphicsAll_VeryHigh}"));
	}
	//if (materialFactory) materialFactory->setShaderQuality(v);
}

void CGui::slTexSize(SL)
{
	int v = val*1.f +slHalf;  if (bGI)  pSet->tex_size = v;
	if (valTexSize)
	{	if (v == 0)  valTexSize->setCaption("Small");  else
		if (v == 1)  valTexSize->setCaption("Big");  }
}

void CGui::slTerMtr(SL)
{
	int v = val*3.f +slHalf;  if (bGI)  pSet->ter_mtr = v;
	if (valTerMtr)
	{	if (v == 0)  valTerMtr->setCaption(TR("#{GraphicsAll_Lowest}"));  else
		if (v == 1)  valTerMtr->setCaption(TR("#{GraphicsAll_Medium}"));  else
		if (v == 2)  valTerMtr->setCaption(TR("#{GraphicsAll_High}"));  else
		if (v == 3)  valTerMtr->setCaption("Parallax");  }
	//if (bGI)  changeShadows();
}

void CGui::slTerTripl(SL)
{
	int v = val*2.f +slHalf;  if (bGI)  pSet->ter_tripl = v;
	if (valTerTripl)
	{	if (v == 0)  valTerTripl->setCaption("Off");  else
		if (v == 1)  valTerTripl->setCaption("One");  else
		if (v == 2)  valTerTripl->setCaption("Full");  }
	//if (bGI)  changeShadows();
}



//  shadows
void CGui::btnShadows(WP){	app->changeShadows();	}
void CGui::btnShaders(WP){	app->changeShadows();	}

void CGui::slShadowType(SL)
{
	int v = val*2.f +slHalf;	if (bGI)  pSet->shadow_type = eShadowType(v);
	if (valShadowType)
	{	if (v == 0)  valShadowType->setCaption("None");  else
		if (v == 1)  valShadowType->setCaption("Simple");  else
		if (v == 2)  valShadowType->setCaption("Depth");  else
		if (v == 3)  valShadowType->setCaption("Soft");  }
}

void CGui::slShadowCount(SL)
{
	int v = 1 + 2.f * val +slHalf;	if (bGI)  pSet->shadow_count = v;
	if (valShadowCount)  valShadowCount->setCaption(toStr(v));
}

void CGui::slShadowSize(SL)
{
	int v = max( 0.0f, min((float) ciShadowNumSizes-1, ciShadowNumSizes * val +slHalf));
	if (bGI)  pSet->shadow_size = v;
	if (valShadowSize)  valShadowSize->setCaption(toStr(ciShadowSizesA[v]));
}

void CGui::slShadowDist(SL)
{
	Real v = 20.f + 4780.f * powf(val, 3.f);	if (bGI)  pSet->shadow_dist = v;
	if (valShadowDist){  valShadowDist->setCaption(fToStr(v,0,2)+" m");  }
}

/*void CGui::slShadowFilter(SL)
{
	int v = 1 + 3 * val +slHalf;  if (bGI)  pSet->shadow_filter = v;
	//if (materialFactory)  materialFactory->setShadowsFilterSize(v);  //TODO..
	if (valShadowFilter)  valShadowFilter->setCaption(toStr(v));
}*/

//  water
void CGui::slWaterSize(SL)
{
	int v = 2.f * val +slHalf;	if (bGI)  pSet->water_rttsize = v;
	if (valWaterSize)  valWaterSize->setCaption(toStr(ciShadowSizesA[v]));
}

void CGui::chkWaterReflect(WP wp)
{
	ChkEv(water_reflect);
	app->mWaterRTT.setReflect(pSet->water_reflect);
	app->changeShadows();
	app->mWaterRTT.recreate();
}

void CGui::chkWaterRefract(WP wp)
{
	ChkEv(water_refract);
	app->mWaterRTT.setRefract(pSet->water_refract);
	app->changeShadows();
	app->mWaterRTT.recreate();
}


//  init  common
//----------------------------------------------------------------------------------------------------------------
void CGui::GuiInitGraphics()
{
	ButtonPtr btn, bchk;  ComboBoxPtr combo;
	Slider* sl;  size_t v;

	//  detail
	Slv(TerDetail,	pSet->terdetail /2.f);
	Slv(TerDist,	powf(pSet->terdist /2000.f, 0.5f));
	Slv(ViewDist,	powf((pSet->view_distance -50.f)/19950.f, 0.5f));
	Slv(RoadDist,	powf(pSet->road_dist /4.f, 0.5f));

	//  textures
	Cmb(combo, "TexFiltering", comboTexFilter);
	Slv(Anisotropy,	pSet->anisotropy /16.f);
	Slv(Shaders,	pSet->shaders);
	Slv(TexSize,	pSet->tex_size /1.f);
	Slv(TerMtr,		pSet->ter_mtr /3.f);
	Slv(TerTripl,	pSet->ter_tripl /2.f);

	//  trees/grass
	Slv(Trees,		powf(pSet->gui.trees /4.f, 0.5f));
	Slv(Grass,		powf(pSet->grass /4.f, 0.5f));
	Slv(TreesDist,	powf((pSet->trees_dist-0.5f) /6.5f, 0.5f));
	Slv(GrassDist,	powf((pSet->grass_dist-0.5f) /6.5f, 0.5f));
	Btn("TrGrReset", btnTrGrReset);
	Chk("UseImposters", chkUseImposters, pSet->use_imposters);
	Chk("ImpostorsOnly", chkImpostorsOnly, pSet->imposters_only);

	//  screen
	Cmb(combo, "CmbAntiAliasing", cmbAntiAliasing);
	int si=0;
	if (combo)
	{	combo->removeAllItems();
		int a[6] = {0,1,2,4,8,16};
		for (int i=0; i < 6; ++i)
		{	int v = a[i];
			combo->addItem(toStr(v));
			if (pSet->fsaa >= v)
				si = i;
		}
		combo->setIndexSelected(si);
	}

	//  shadows
	Slv(ShadowType,	pSet->shadow_type /2.f);
	Slv(ShadowCount,(pSet->shadow_count-1) /2.f);
	//Slv(ShadowFilter, (pSet->shadow_filter-1) /3.f);
	Slv(ShadowSize,	pSet->shadow_size /float(ciShadowNumSizes));
	Slv(ShadowDist,	powf((pSet->shadow_dist -20.f)/4780.f, 1.f/3.f));
	Btn("Apply", btnShadows);
	
	Btn("ApplyShaders", btnShaders);
	Btn("ApplyShadersWater", btnShaders);
	
	//  water
	Chk("WaterReflection", chkWaterReflect, pSet->water_reflect);
	Chk("WaterRefraction", chkWaterRefract, pSet->water_refract);
	Slv(WaterSize, pSet->water_rttsize /2.f);
	
	Cmb(combo, "CmbGraphicsAll", comboGraphicsAll);
	if (combo)
	{	combo->removeAllItems();
		combo->addItem(TR("#{GraphicsAll_Lowest}"));
		combo->addItem(TR("#{GraphicsAll_Low}"));
		combo->addItem(TR("#{GraphicsAll_Medium}"));
		combo->addItem(TR("#{GraphicsAll_High}"));
		combo->addItem(TR("#{GraphicsAll_Higher}"));
		combo->addItem(TR("#{GraphicsAll_VeryHigh}"));
		combo->addItem(TR("#{GraphicsAll_Ultra}"));
		combo->addItem(TR("#{GraphicsAll_Impossible}"));
	}
	
	//  render systems
	Cmb(combo, "CmbRendSys", comboRenderSystem);
	if (combo)
	{	combo->removeAllItems();

		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		const int nRS = 3;  //4
		const String rs[nRS] = {"Default", "OpenGL Rendering Subsystem",
			"Direct3D9 Rendering Subsystem"/*, "Direct3D11 Rendering Subsystem"*/};
		#else
		const int nRS = 2;
		const String rs[nRS] = {"Default", "OpenGL Rendering Subsystem"};
		#endif
			
		for (int i=0; i < nRS; ++i)
		{
			combo->addItem(rs[i]);
			if (pSet->rendersystem == rs[i])
				combo->setIndexSelected(combo->findItemIndexWith(rs[i]));
		}
		//const RenderSystemList& rsl = Ogre::Root::getSingleton().getAvailableRenderers();
		//for (int i=0; i < rsl.size(); ++i)
		//	combo->addItem(rsl[i]->getName());
	}
}


//  util
//----------------------------------------------------------------------------------------------------------------
void CGui::GuiCenterMouse()
{	
	int xm = app->mWindow->getWidth()/2, ym = app->mWindow->getHeight()/2;

	app->mInputWrapper->warpMouse(xm, ym);
	MyGUI::InputManager::getInstance().injectMouseMove(xm, ym, 0);
}

void CGui::btnQuit(WP)
{
	app->mShutDown = true;
}

//  unfocus lists (would have double up/dn key input)
void CGui::UnfocusLists()
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

void CGui::SizeGUI()
{
	#ifndef SR_EDITOR
	app->baseSizeGui();
	#endif
	
	//  call recursive method for all root widgets
	for (VectorWidgetPtr::iterator it = app->vwGui.begin(); it != app->vwGui.end(); ++it)
		doSizeGUI((*it)->getEnumerator());
}

void CGui::doSizeGUI(EnumeratorWidgetPtr widgets)
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
				WidgetPtr window = app->mGUI->findWidget<Widget>(relativeTo);
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
void CGui::GuiInitTooltip()
{
	mToolTip = Gui::getInstance().findWidget<Widget>("ToolTip");
	mToolTip->setVisible(false);
	mToolTipTxt = mToolTip->getChildAt(0)->castType<Edit>();
}

void CGui::setToolTips(EnumeratorWidgetPtr widgets)
{
	while (widgets.next())
	{
		WidgetPtr wp = widgets.current();
		wp->setAlign(Align::Default);
		
		///
		/*if (wp->getTypeName() == "TabControl") //&& wp->getSkinWidgetsByNameskin="TabControlIcon
		{
			ISubWidgetRect* r = wp->getSubWidgetMain();
			if (r)
			r->setCoord(IntCoord(100,100,200,200));
			if (r)
			LogO("r: "+r->getTypeName()+" "+r->getClassTypeName());

			//ap->mGUI->mSubWidgetManager
			//wp->getSubWidgetMain
			LogO(wp->getName());
			size_t ch = wp->getChildCount();
			for (size_t c=0; c < ch; ++c)
			{
				Widget* w = wp->getChildAt(c);
				LogO("  "+w->getName()+" "+w->getTypeName());

				size_t ch1 = w->getChildCount();
				for (size_t c1=0; c1 < ch1; ++c1)
				{
					Widget* w1 = w->getChildAt(c1);
					LogO("    "+w1->getName()+" "+w1->getTypeName());
		}	}	}
		/**/
		
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
			wp->eventToolTip += newDelegate(this, &CGui::notifyToolTip);
		}
		//LogO(wp->getName() + (tip ? "  *" : ""));
		setToolTips(wp->getEnumerator());
	}
}

void CGui::notifyToolTip(Widget *sender, const ToolTipInfo &info)
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
void CGui::boundedMove(Widget* moving, const IntPoint& point)
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
void CGui::GuiInitLang()
{
	languages["en"] = TR("#{LANG_EN}");  //English
	languages["de"] = TR("#{LANG_DE}");  //German
	languages["fi"] = TR("#{LANG_FI}");  //Finnish
	languages["pl"] = TR("#{LANG_PL}");  //Polish
	languages["ro"] = TR("#{LANG_RO}");  //Romanian
	languages["fr"] = TR("#{LANG_FR}");  //French
	languages["ru"] = TR("#{LANG_RU}");  //Russian
	languages["pt"] = TR("#{LANG_PT}");  //Portuguese

	ComboBoxPtr combo = app->mGUI->findWidget<ComboBox>("Lang");
	if (!combo)  return;
	combo->eventComboChangePosition += newDelegate(this, &CGui::comboLanguage);
	for (std::map<std::string, UString>::const_iterator it = languages.begin();
		it != languages.end(); ++it)
	{
		combo->addItem(it->second);
		if (it->first == pSet->language)
			combo->setIndexSelected(combo->getItemCount()-1);
	}
}

void CGui::comboLanguage(MyGUI::ComboBox* wp, size_t val)
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
	
	#ifndef SR_EDITOR
	app->setTranslations();
	#endif
}

//  [Screen] 
//-----------------------------------------------------------------------------------------------------------

void CGui::cmbAntiAliasing(MyGUI::ComboBox* wp, size_t val)
{
	int v = s2i(wp->getItemNameAt(val));
	if (bGI)  pSet->fsaa = v;
}

///  resolutions
//  change
void CGui::btnResChng(WP)
{
	if (!resList)  return;
	if (resList->getIndexSelected() == MyGUI::ITEM_NONE)  return;
	String mode = resList->getItemNameAt(resList->getIndexSelected());

	pSet->windowx = StringConverter::parseInt(StringUtil::split(mode, "x")[0]);
	pSet->windowy = StringConverter::parseInt(StringUtil::split(mode, "x")[1]);

	Uint32 flags = SDL_GetWindowFlags(app->mSDLWindow);
	if (flags & SDL_WINDOW_MAXIMIZED) // Can't change size of a maximized window
		SDL_RestoreWindow(app->mSDLWindow);

	if (pSet->fullscreen)
	{
		SDL_DisplayMode mode;
		SDL_GetWindowDisplayMode(app->mSDLWindow, &mode);
		mode.w = pSet->windowx;
		mode.h = pSet->windowy;
		SDL_SetWindowDisplayMode(app->mSDLWindow, &mode);
		SDL_SetWindowFullscreen(app->mSDLWindow, 0);
		SDL_SetWindowFullscreen(app->mSDLWindow, SDL_WINDOW_FULLSCREEN);
	}else
		SDL_SetWindowSize(app->mSDLWindow, pSet->windowx, pSet->windowy);
}


//  get screen resolutions
struct ScrRes {  int w,h;  String mode;  };

bool ResSort(const ScrRes& r1, const ScrRes& r2)
{
	return (r1.w <= r2.w) && (r1.h <= r2.h);
}

void CGui::InitGuiScreenRes()
{
	ButtonPtr bchk;
	Chk("FullScreen", chkVidFullscr, pSet->fullscreen);
	Chk("VSync", chkVidVSync, pSet->vsync);

	//  video resolutions combobox
	resList = app->mGUI->findWidget<List>("ResList");
	if (resList)
	{
		//  get resolutions
		const StringVector& videoModes = Root::getSingleton().getRenderSystem()->getConfigOptions()["Video Mode"].possibleValues;
		String modeSel = "";
		std::vector<ScrRes> vRes;
		for (int i=0; i < videoModes.size(); i++)
		{
			String mode = videoModes[i];
			StringUtil::trim(mode);
			if (StringUtil::match(mode, "*16-bit*"))  continue;  //skip ?DX

			StringVector vmopts = StringUtil::split(mode, " x");  // only resolution
			int w = StringConverter::parseUnsignedInt(vmopts[0]);
			int h = StringConverter::parseUnsignedInt(vmopts[1]);
			
			if (w >= 800 && h >= 600)  // min res
			{
				mode = toStr(w) + " x " + toStr(h);
				ScrRes res;  res.w = w;  res.h = h;  res.mode = mode;
				vRes.push_back(res);
				int ww = w - app->mWindow->getWidth(), hh = h - app->mWindow->getHeight();
				if (abs(ww) < 30 && abs(hh) < 50)  // window difference
					modeSel = mode;
			}
		}
		//  sort and add
		std::sort(vRes.begin(), vRes.end(), ResSort);
		for (int r=0; r < vRes.size(); ++r)
			resList->addItem(vRes[r].mode);

		//  sel current mode
		if (modeSel != "")
			resList->setIndexSelected(resList->findItemIndexWith(modeSel));
	}
	ButtonPtr btnRes = app->mGUI->findWidget<Button>("ResChange");
	if (btnRes)  {  btnRes->eventMouseButtonClick += newDelegate(this, &CGui::btnResChng);  }
}


//  resize Options wnd
void CGui::ResizeOptWnd()
{
	if (!app->mWndOpts)  return;

	const int wx = pSet->windowx, wy = pSet->windowy;
	const int yN = 7;
	const Real yw[yN] = {400.f, 600.f, 720.f, 768.f, 960.f, 1024.f, 1200.f};
	const Real yf[yN] = {0.0f,  0.0f,  0.05f, 0.1f,  0.2f,  0.3f,   0.3f};

	Real xm = 0.f, ym = 0.f;  // margin
	for (int i=0; i < yN; ++i)
		if (wy >= yw[i]-10)  ym = yf[i];

	Real yo = (1.f - ym)*wy, xo = 4.f/3.f * yo;  // opt wnd size in pix
	ym = (wy - yo)*0.5f;  xm = (wx - xo)*0.5f;

	#ifndef SR_EDITOR  // game
	app->mWndGame->setCoord(xm, ym, xo, yo);
	app->mWndReplays->setCoord(xm, ym, xo, yo);
	//ap->mWndTweak->setCoord(0, 6, xo/3, yo-ym);
	#else  // editor
	app->mWndEdit->setCoord(xm, ym, xo, yo);
	#endif  // both
	app->mWndHelp->setCoord(xm, ym, xo, yo);
	app->mWndOpts->setCoord(xm, ym, xo, yo);

	if (bnQuit)  //  reposition Quit btn
		bnQuit->setCoord(wx - 0.09*wx, 0, 0.09*wx, 0.03*wy);

	updTrkListDim();
	#ifndef SR_EDITOR
	updChampListDim();  // resize lists
	#endif
}

void CGui::chkVidFullscr(WP wp)
{
	ChkEv(fullscreen);
	SDL_SetWindowFullscreen(app->mSDLWindow,  wp->castType<MyGUI::Button>()->getStateSelected()? SDL_WINDOW_FULLSCREEN : 0);
}

void CGui::chkVidVSync(WP wp)
{		
	ChkEv(vsync); 
	Ogre::Root::getSingleton().getRenderSystem()->setWaitForVerticalBlank(pSet->vsync);
}

void CGui::comboRenderSystem(ComboBoxPtr wp, size_t val)
{
	pSet->rendersystem = wp->getItemNameAt(val);
}
