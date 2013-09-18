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

void CGui::comboTexFilter(CMB)
{
	TextureFilterOptions tfo;
	switch (val)  {
		case 0:	 tfo = TFO_BILINEAR;	break;
		case 1:	 tfo = TFO_TRILINEAR;	break;
		case 2:	 tfo = TFO_ANISOTROPIC;	break;	}
	MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
}

void CGui::slAnisotropy(SV*)
{
	MaterialManager::getSingleton().setDefaultAnisotropy(pSet->anisotropy);
}

void CGui::slViewDist(SV* sv)
{
	Vector3 sc = (*sv->pFloat) * Vector3::UNIT_SCALE;

	if (app->ndSky)  app->ndSky->setScale(sc);
	#ifndef SR_EDITOR
		app->mSplitMgr->UpdateCamDist();  // game, for all cams
	#else
		app->mCamera->setFarClipDistance(pSet->view_distance*1.1f);
	#endif
}

void CGui::slTerDetail(SV*)
{
	app->UpdTerErr();
}

void CGui::slTerDist(SV*)
{
	if (app->mTerrainGlobals)
		app->mTerrainGlobals->setCompositeMapDistance(pSet->terdist);
}

//  trees/grass
void CGui::btnTrGrReset(WP wp)
{
	svTrees.SetValueF(1.5f);
	svGrass.SetValueF(1.f);
	svTreesDist.SetValueF(1.f);
	svGrassDist.SetValueF(2.f);
}

void CGui::chkUseImposters(WP wp)
{
	ChkEv(use_imposters);
}
void CGui::chkImpostorsOnly(WP wp)
{
	ChkEv(imposters_only);
}


//  shadows
void CGui::btnShadows(WP){	app->changeShadows();	}
void CGui::btnShaders(WP){	app->changeShadows();	}




//  water
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
void CGui::GuiInitGraphics()  // also called on preset change with bGI true
{
	ButtonPtr btn, bchk;  ComboBoxPtr cmb;
	Slider* sl;  size_t v;
	SliderValue::pGUI = app->mGUI;
	SliderValue::bGI = &bGI;

	SliderValue* sv;
	#define Sev(ev)  if (sv->event.empty())  sv->event += newDelegate(this, &CGui::##ev)
	//Check* ck;
	//#define Cev(ev)  if (sv->event.empty())  ck->event += newDelegate(this, &CGui::##ev)
	
	//  detail
	sv= &svTerDetail;	sv->Init("TerDetail",	&pSet->terdetail,	0.f,2.f);  Sev(slTerDetail);
	sv= &svTerDist;		sv->Init("TerDist",		&pSet->terdist, 0.f,2000.f, 2.f, 0,3, 1.f," m");  Sev(slTerDist);
	sv= &svRoadDist;	sv->Init("RoadDist",	&pSet->road_dist,	0.f,4.f, 2.f, 2,5);

	//  textures
	Cmb(cmb, "TexFiltering", comboTexFilter);

	sv= &svViewDist;	sv->Init("ViewDist",	&pSet->view_distance, 50.f,20000.f, 2.f, 1,4, 0.001f," km");  Sev(slViewDist);
	sv= &svAnisotropy;	sv->Init("Anisotropy",	&pSet->anisotropy,	0.f,16.f);  Sev(slAnisotropy);
	sv= &svTexSize;
		sv->strMap[0] = "Small";  sv->strMap[1] = "Big";
						sv->Init("TexSize",		&pSet->tex_size,	0.f,1.f);
	sv= &svTerMtr;
		sv->strMap[0] = TR("#{GraphicsAll_Lowest}");	sv->strMap[1] = TR("#{GraphicsAll_Medium}");
		sv->strMap[2] = TR("#{GraphicsAll_High}");		sv->strMap[3] = "Parallax";
						sv->Init("TerMtr",		&pSet->ter_mtr,		0.f,3.f);
	sv= &svTerTripl;
		sv->strMap[0] = "Off";  sv->strMap[1] = "One";  sv->strMap[2] = "Full";
						sv->Init("TerTripl",	&pSet->ter_tripl,	0.f,2.f);

	//  trees/grass
	sv= &svTrees;		sv->Init("Trees",		&pSet->gui.trees,	0.f,4.f, 2.f);
	sv= &svGrass;		sv->Init("Grass",		&pSet->grass,		0.f,4.f, 2.f);
	sv= &svTreesDist;	sv->Init("TreesDist",   &pSet->trees_dist,	0.5f,7.f, 2.f);
	sv= &svGrassDist;	sv->Init("GrassDist",   &pSet->grass_dist,	0.5f,7.f, 2.f);
	Btn("TrGrReset",  btnTrGrReset);

	//ck = ckUseImposters; ck->Init("UseImposters", pSet->use_imposters);  Cev(chkUseImposters);
	Chk("UseImposters",  chkUseImposters,  pSet->use_imposters);
	Chk("ImpostorsOnly", chkImpostorsOnly, pSet->imposters_only);

	//  shadows
	sv= &svShadowType;
		sv->strMap[0] = "None";		sv->strMap[1] = "Depth";	sv->strMap[2] = "Soft-";
						sv->Init("ShadowType",	&pSet->shadow_type, 0.f,1.f); //2.f);
	sv= &svShadowCount;	sv->Init("ShadowCount",	&pSet->shadow_count, 1.f,3.f);
	sv= &svShadowSize;
		for (int i=0; i < ciShadowNumSizes; ++i)  sv->strMap[i] = toStr(ciShadowSizesA[i]);
						sv->Init("ShadowSize",	&pSet->shadow_size, 0.f,ciShadowNumSizes-1);
	sv= &svShadowDist;	sv->Init("ShadowDist",	&pSet->shadow_dist, 20.f,5000.f, 3.f, 0,3, 1.f," m");
	Btn("Apply", btnShadows);
	
	Btn("ApplyShaders", btnShaders);
	Btn("ApplyShadersWater", btnShaders);
	
	//  water
	Chk("WaterReflection", chkWaterReflect, pSet->water_reflect);
	Chk("WaterRefraction", chkWaterRefract, pSet->water_refract);
	sv= &svWaterSize;
		for (int i=0; i <= 2; ++i)  sv->strMap[i] = toStr(ciShadowSizesA[i]);
						sv->Init("WaterSize",	&pSet->water_rttsize, 0.f,2.f);
	
	//  presets
	Cmb(cmb, "CmbGraphicsAll", comboGraphicsAll);
	if (cmb)
	{	cmb->removeAllItems();
		cmb->addItem(TR("#{GraphicsAll_Lowest}"));  cmb->addItem(TR("#{GraphicsAll_Low}"));
		cmb->addItem(TR("#{GraphicsAll_Medium}"));  cmb->addItem(TR("#{GraphicsAll_High}"));
		cmb->addItem(TR("#{GraphicsAll_Higher}"));  cmb->addItem(TR("#{GraphicsAll_VeryHigh}"));
		cmb->addItem(TR("#{GraphicsAll_Ultra}"));   cmb->addItem(TR("#{GraphicsAll_Impossible}"));
		cmb->setIndexSelected(pSet->preset);
	}
	
	//  screen
	Cmb(cmb, "CmbAntiAliasing", cmbAntiAliasing);
	int si=0;
	if (cmb)
	{	cmb->removeAllItems();
		int a[6] = {0,1,2,4,8,16};
		for (int i=0; i < 6; ++i)
		{	int v = a[i];
			cmb->addItem(toStr(v));
			if (pSet->fsaa >= v)
				si = i;
		}
		cmb->setIndexSelected(si);
	}

	//  render systems
	Cmb(cmb, "CmbRendSys", comboRenderSystem);
	if (cmb)
	{	cmb->removeAllItems();

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
			cmb->addItem(rs[i]);
			if (pSet->rendersystem == rs[i])
				cmb->setIndexSelected(cmb->findItemIndexWith(rs[i]));
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
	languages["en"] = TR("#{LANG_EN}");  languages["de"] = TR("#{LANG_DE}");
	languages["fr"] = TR("#{LANG_FR}");  languages["pl"] = TR("#{LANG_PL}");
	languages["ru"] = TR("#{LANG_RU}");  languages["fi"] = TR("#{LANG_FI}");
	languages["pt"] = TR("#{LANG_PT}");  languages["ro"] = TR("#{LANG_RO}");

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
	#ifndef SR_EDITOR
	app->bSizeHUD = true;
	#endif
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
