#include "pch.h"
#include "../Defines.h"
#include "../../road/Road.h"
#include "../../vdrift/pathmanager.h"
#include "MaterialFactory.h"
#ifndef ROAD_EDITOR
	#include "../../vdrift/game.h"
	#include "../OgreGame.h"
	#include "../SplitScreen.h"
#else
	#include "../../editor/OgreApp.h"
#endif
#include <OgreRoot.h>
#include <OgreMaterialManager.h>
#include <OgreSceneManager.h>
#include <OgreTerrain.h>
#include <OgreRenderWindow.h>
#include "Gui_Def.h"
using namespace MyGUI;
using namespace Ogre;

// MyGUI 3.2 has no Align::Relative
#define ALIGN Align::Default

///  Gui Init  [Graphics]
//----------------------------------------------------------------------------------------------------------------

//  textures
void App::comboTexFilter(CMB)
{
	TextureFilterOptions tfo;							
	switch (val)  {
		case 0:	 tfo = TFO_BILINEAR;	break;
		case 1:	 tfo = TFO_TRILINEAR;	break;
		case 2:	 tfo = TFO_ANISOTROPIC;	break;	}
	MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);  //if (bGI)
}

void App::slAnisotropy(SL)
{	if (bGI)  {
	MaterialManager::getSingleton().setDefaultAnisotropy(val);	pSet->anisotropy = val;  }
	if (valAnisotropy)	valAnisotropy->setCaption(toStr(val));
}

//  view dist
void App::slViewDist(SL)
{
	Real v = 50.f + 19950.f * powf(val/res, 2.f);
	Vector3 sc = v*Vector3::UNIT_SCALE;

	SceneNode* nskb = mSceneMgr->getSkyBoxNode();
	if (nskb)  nskb->setScale(sc*0.58);
	else  if (ndSky)  ndSky->setScale(sc);

	if (bGI)  pSet->view_distance = v;
	if (valViewDist){	Fmt(s, "%4.1f km", v*0.001f);	valViewDist->setCaption(s);  }

	// Set new far clip distance for all cams
	#ifndef ROAD_EDITOR
	/*?if (bGI)*/  mSplitMgr->UpdateCamDist();
	#else
	mCamera->setFarClipDistance(pSet->view_distance*1.1f);
	#endif
}

//  ter detail
void App::slTerDetail(SL)
{
	Real v = 20.f * powf(val/res, 2.f);  if (bGI)  {  pSet->terdetail = v;
		if (mTerrainGlobals)
			mTerrainGlobals->setMaxPixelError(v);  }
	if (valTerDetail){	Fmt(s, "%4.1f %%", v);	valTerDetail->setCaption(s);  }
}

//  ter dist
void App::slTerDist(SL)
{
	Real v = 2000.f * powf(val/res, 2.f);  if (bGI)  {  pSet->terdist = v;
		if (mTerrainGlobals)
			mTerrainGlobals->setCompositeMapDistance(v);  }
	if (valTerDist){	Fmt(s, "%4.0f m", v);	valTerDist->setCaption(s);  }
}

//  road dist
void App::slRoadDist(SL)
{
	Real v = 4.f * powf(val/res, 2.f);  if (bGI)  pSet->road_dist = v;
	if (valRoadDist){	Fmt(s, "%5.2f", v);	valRoadDist->setCaption(s);  }
}


//  trees/grass
void App::slTrees(SL)
{
	Real v = 4.f * powf(val/res, 2.f);  if (bGI)  pSet->trees = v;
	if (valTrees){	Fmt(s, "%4.2f", v);	valTrees->setCaption(s);  }
}
void App::slGrass(SL)
{
	Real v = 4.f * powf(val/res, 2.f);  if (bGI)  pSet->grass = v;
	if (valGrass){	Fmt(s, "%4.2f", v);	valGrass->setCaption(s);  }
}

void App::slTreesDist(SL)
{
	Real v = 0.5f + 6.5f * powf(val/res, 2.f);  if (bGI)  pSet->trees_dist = v;
	if (valTreesDist){	Fmt(s, "%4.2f", v);	valTreesDist->setCaption(s);  }
}
void App::slGrassDist(SL)
{
	Real v = 0.5f + 6.5f * powf(val/res, 2.f);  if (bGI)  pSet->grass_dist = v;
	if (valGrassDist){	Fmt(s, "%4.2f", v);	valGrassDist->setCaption(s);  }
}

void App::btnTrGrReset(WP wp)
{
	ScrollBar* sl;  size_t v;
	#define setSld(name)  sl##name(0,v);  \
		sl = (ScrollBar*)mWndOpts->findWidget(#name);  if (sl)  sl->setScrollPosition(v);
	v = res*powf(1.f /4.f, 0.5f);
	setSld(Trees);
	setSld(Grass);
	v = res*powf((1.f-0.5f) /6.5f, 0.5f);
	setSld(TreesDist);
	setSld(GrassDist);
}

void App::chkUseImposters(WP wp)
{
	ChkEv(use_imposters);
}
void App::slShaders(SL)
{
	int v = val;  if (bGI)  pSet->shaders = v;
	if (valShaders)
	{	if (v == 0)  valShaders->setCaption("Vertex");  else
		if (v == 1)  valShaders->setCaption("Pixel");  else
		if (v == 2)  valShaders->setCaption("Metal");  }
}

void App::slTexSize(SL)
{
	int v = val;  if (bGI)  pSet->tex_size = v;
	if (valTexSize)
	{	if (v == 0)  valTexSize->setCaption("Small");  else
		if (v == 1)  valTexSize->setCaption("Big");  }
	
	if (!materialFactory) return;
	if (v == 0)
		materialFactory->setTexSize(0); // lowest
	else if (v == 1)
		materialFactory->setTexSize(4096); // highest
}

void App::slTerMtr(SL)
{
	int v = val;  if (bGI)  pSet->ter_mtr = v;
	if (valTerMtr)
	{	if (v == 0)  valTerMtr->setCaption("Lowest");  else
		if (v == 1)  valTerMtr->setCaption("Low");  else
		if (v == 2)  valTerMtr->setCaption("Normal");  else
		if (v == 3)  valTerMtr->setCaption("Parallax");  }
	//if (bGI)  changeShadows();
}


//  shadows
void App::btnShadows(WP){	changeShadows();	}

void App::slShadowType(SL)
{
	int v = val;	if (bGI)  pSet->shadow_type = v;
	if (valShadowType)
	{	if (v == 0)  valShadowType->setCaption("None");  else
		if (v == 1)  valShadowType->setCaption("Old");  else
		if (v == 2)  valShadowType->setCaption("Normal");  else
		if (v == 3)  valShadowType->setCaption("Depth-");  }
}

void App::slShadowCount(SL)
{
	int v = 2 + 2.f * val/res;	if (bGI)  pSet->shadow_count = v;
	if (valShadowCount)  valShadowCount->setCaption(toStr(v));
}

void App::slShadowSize(SL)
{
	int v = std::max( 0.0f, std::min((float) ciShadowNumSizes-1, ciShadowNumSizes * val/res));
	if (bGI)  pSet->shadow_size = v;
	if (valShadowSize)  valShadowSize->setCaption(toStr(ciShadowSizesA[v]));
}

void App::slShadowDist(SL)
{
	Real v = 50.f + 4750.f * powf(val/res, 2.f);	if (bGI)  pSet->shadow_dist = v;
	if (valShadowDist){  Fmt(s, "%4.1f km", v*0.001f);	valShadowDist->setCaption(s);  }
}


//  init  common
//----------------------------------------------------------------------------------------------------------------
void App::GuiInitGraphics()
{
	ButtonPtr btn, bchk;  ComboBoxPtr combo;
	ScrollBar* sl;  size_t v;

	//  detail
	Slv(TerDetail,	powf(pSet->terdetail /20.f, 0.5f));
	Slv(TerDist,	powf(pSet->terdist /2000.f, 0.5f));
	Slv(ViewDist,	powf((pSet->view_distance -50.f)/19950.f, 0.5f));
	Slv(RoadDist,	powf(pSet->road_dist /4.f, 0.5f));

	//  textures
	Cmb(combo, "TexFiltering", comboTexFilter);
	Slv(Anisotropy,	pSet->anisotropy /res);
	Slv(Shaders,	pSet->shaders /res);
	Slv(TexSize,	pSet->tex_size /res);
	Slv(TerMtr,		pSet->ter_mtr /res);

	//  trees/grass
	Slv(Trees,		powf(pSet->trees /4.f, 0.5f));
	Slv(Grass,		powf(pSet->grass /4.f, 0.5f));
	Slv(TreesDist,	powf((pSet->trees_dist-0.5f) /6.5f, 0.5f));
	Slv(GrassDist,	powf((pSet->grass_dist-0.5f) /6.5f, 0.5f));
	Btn("TrGrReset", btnTrGrReset);
	Chk("UseImposters", chkUseImposters, pSet->use_imposters);

	// screen
	// find max. fsaa
	int fsaa = 0; int newfsaa;
	Ogre::ConfigOptionMap& configOptions = Ogre::Root::getSingleton().getRenderSystem()->getConfigOptions();
	Ogre::ConfigOptionMap::iterator result = configOptions.find("FSAA");
	if ( result != configOptions.end() )
	{
		Ogre::ConfigOption& FSAAOption = result->second;
		for( Ogre::StringVector::iterator i( FSAAOption.possibleValues.begin() ), iEnd( FSAAOption.possibleValues.end() ); i != iEnd; ++i )
		{
			newfsaa = strtol( (*i).c_str(), 0, 10  );
			if (newfsaa > fsaa) fsaa = newfsaa;
		}
	}
	Slv(AntiAliasing, float(pSet->fsaa)/float(fsaa));

	//  shadows
	Slv(ShadowType,	pSet->shadow_type /res);
	Slv(ShadowCount,(pSet->shadow_count-2) /2.f);
	Slv(ShadowSize,	pSet->shadow_size /float(ciShadowNumSizes));
	Slv(ShadowDist,	powf((pSet->shadow_dist -50.f)/4750.f, 0.5f));
	Btn("Apply", btnShadows);
	
	Cmb(combo, "CmbGraphicsAll", comboGraphicsAll);
	if (combo)  {
		combo->removeAllItems();
		combo->addItem(TR("#{GraphicsAll_Lowest}"));
		combo->addItem(TR("#{GraphicsAll_Low}"));
		combo->addItem(TR("#{GraphicsAll_Medium}"));
		combo->addItem(TR("#{GraphicsAll_High}"));
		combo->addItem(TR("#{GraphicsAll_Ultra}"));
    }
    
    //  render systems
	Cmb(combo, "CmbRendSys", comboRenderSystem);
	if (combo)  {
		combo->removeAllItems();

		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		const int nRS = 4;
		const String rs[nRS] = {"Default", "OpenGL Rendering Subsystem",
			"Direct3D9 Rendering Subsystem", "Direct3D11 Rendering Subsystem"};
		#else
		const int nRS = 2;
		const String rs[nRS] = {"Default", "OpenGL Rendering Subsystem"};
		#endif
			
		for (int i=0; i < nRS; ++i)
		{
			combo->addItem(rs[i]);
			if (pSet->rendersystem == rs[i])
				combo->setIndexSelected(combo->getItemCount()-1);
		}
		//const RenderSystemList& rsl = Ogre::Root::getSingleton().getAvailableRenderers();
		//for (int i=0; i < rsl.size(); ++i)
		//	combo->addItem(rsl[i]->getName());
	}
}


//  util
//----------------------------------------------------------------------------------------------------------------
void App::GuiCenterMouse()
{
	// mouse center causes problems on x11 with mouse capture=off
	#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	if (pSet->x11_capture_mouse == false) return;
	#endif
	
	int xm = mWindow->getWidth()/2, ym = mWindow->getHeight()/2;
	MyGUI::InputManager::getInstance().injectMouseMove(xm, ym, 0);
	OIS::MouseState &ms = const_cast<OIS::MouseState&>(mMouse->getMouseState());
	ms.X.abs = xm;  ms.Y.abs = ym;
}

void App::btnQuit(WP)
{
	mShutDown = true;
}

//  begin MyGUI HACKS
//-----------------------------------------------------------------------------------

void App::SizeGUI()
{		
	// call recursive method for all root widgets
	for (VectorWidgetPtr::iterator it = vwGui.begin(); it != vwGui.end(); ++it)
	{
		doSizeGUI((*it)->getEnumerator());
	}
}

void App::doSizeGUI(MyGUI::EnumeratorWidgetPtr widgets)
{
	while (widgets.next())
	{
        WidgetPtr wp = widgets.current();

		std::string relativeTo = wp->getUserString("RelativeTo");
		
		if (relativeTo != "")
		{
			// position & size relative to the widget specified in "RelativeTo" property (or full screen)
			MyGUI::IntSize relativeSize;
			if (relativeTo == "Screen")
				relativeSize = MyGUI::IntSize(mWindow->getWidth(), mWindow->getHeight());
			else
			{
				WidgetPtr window = mGUI->findWidget<Widget>(relativeTo);
				relativeSize = window->getSize();
			}
			
			// retrieve original size & pos
			MyGUI::IntPoint origPos;
			MyGUI::IntSize origSize;
			origPos.left = s2i( wp->getUserString("origPosX") );
			origPos.top = s2i( wp->getUserString("origPosY") );
			origSize.width = s2i( wp->getUserString("origSizeX") );
			origSize.height = s2i( wp->getUserString("origSizeY") );
			
			// calc new size & pos
			const MyGUI::IntPoint& newPos = MyGUI::IntPoint(
				int(origPos.left * (float(relativeSize.width) / 800)),
				int(origPos.top * (float(relativeSize.height) / 600))
			);
			
			const MyGUI::IntSize& newScale = MyGUI::IntSize(
				int(origSize.width * (float(relativeSize.width) / 800)),
				int(origSize.height * (float(relativeSize.height) / 600))
			);
			
			// apply
			wp->setPosition(newPos);
			wp->setSize(newScale);
		}
		
		doSizeGUI(wp->getEnumerator());
	}
}

//-----------------------------------------------------------------------------------



///  Tooltips
//----------------------------------------------------------------------------------------------------------------
void App::GuiInitTooltip()
{
	mToolTip = Gui::getInstance().findWidget<Widget>("ToolTip");
	mToolTip->setVisible(false);
	mToolTipTxt = mToolTip->getChildAt(0)->castType<Edit>();
}

void App::setToolTips(EnumeratorWidgetPtr widgets)
{
    while (widgets.next())
    {
        WidgetPtr wp = widgets.current();
		wp->setAlign(ALIGN);
		
		MyGUI::IntPoint origPos = wp->getPosition();
		MyGUI::IntSize origSize = wp->getSize();
		
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
			wp->eventToolTip += newDelegate(this, &App::notifyToolTip);
		}
		//LogO(wp->getName() + (tip ? "  *" : ""));
        setToolTips(wp->getEnumerator());
    }
}

void App::notifyToolTip(Widget *sender, const ToolTipInfo &info)
{
	if (!mToolTip)  return;

	#ifndef ROAD_EDITOR
	if (!isFocGui)
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
void App::boundedMove(Widget* moving, const IntPoint& point)
{
	const IntPoint offset(20, 20);  // mouse cursor
	IntPoint p = point + offset;

	const IntSize& size = moving->getSize();
	
	int vpw = mWindow->getWidth();
	int vph = mWindow->getHeight();
	
	if (p.left + size.width > vpw)
		p.left = vpw - size.width;
	if (p.top + size.height > vph)
		p.top = vph - size.height;
			
	moving->setPosition(p);
}


//  Languages combo
//----------------------------------------------------------------------------------------------------------------
void App::GuiInitLang()
{
	languages["en"] = "English";
	languages["de"] = "Deutsch";  //German
	languages["fi"] = "Suomi";   //Finnish
	languages["ro"] = "Romana";  //Romanian â?
	languages["pl"] = "Polski";  //Polish
	
	ComboBoxPtr combo = mGUI->findWidget<ComboBox>("Lang");
	if (!combo)  return;
	combo->eventComboChangePosition += newDelegate(this, &App::comboLanguage);
	for (std::map<std::string, std::string>::const_iterator it = languages.begin();
		it != languages.end(); it++)
	{
		combo->addItem(it->second);
		if (it->first == pSet->language)
			combo->setIndexSelected(combo->getItemCount()-1);
	}
}

void App::comboLanguage(MyGUI::ComboBox* wp, size_t val)
{
	if (val == MyGUI::ITEM_NONE)  return;
	MyGUI::ComboBoxPtr cmb = static_cast<MyGUI::ComboBoxPtr>(wp);
	std::string sel = cmb->getItemNameAt(val);
	
	for (std::map<std::string, std::string>::const_iterator it = languages.begin();
		it != languages.end(); it++)
	{
		if (it->second == sel)
			pSet->language = it->first;
	}
	MyGUI::LanguageManager::getInstance().setCurrentLanguage(pSet->language);

	//  reinit gui
	bGuiReinit = true;
	
	#ifndef ROAD_EDITOR
	setTranslations();
	#endif
}

//  [Screen] 
//-----------------------------------------------------------------------------------------------------------

void App::slAntiAliasing(SL)
{
	// get allowed values for FSAA
	std::vector<int> fsaaValues;
	try
	{
		int fsaa = 0;
		Ogre::ConfigOptionMap& configOptions = Ogre::Root::getSingleton().getRenderSystem()->getConfigOptions();
		Ogre::ConfigOptionMap::iterator result = configOptions.find("FSAA");
		if ( result != configOptions.end() )
		{
			Ogre::ConfigOption& FSAAOption = result->second;
			for( Ogre::StringVector::iterator i( FSAAOption.possibleValues.begin() ), iEnd( FSAAOption.possibleValues.end() ); i != iEnd; ++i )
			{
				fsaa = strtol( (*i).c_str(), 0, 10  );
				fsaaValues.push_back(fsaa);
			}
		}
	}
	catch (Ogre::Exception&) { return; }
	
	float v = fsaaValues.back() * val/res;
	
	if (fsaaValues.size() < 1) return;
	
	for (int i=1; i<fsaaValues.size(); i++)
	{
		if (v >= fsaaValues[i]) continue;
		int smaller = fsaaValues[i] - v;
		int bigger = v - fsaaValues[i-1];
		if (bigger > smaller) v = fsaaValues[i];
		else v = fsaaValues[i-1];
		break;
	}
	if (bGI)  pSet->fsaa = v;
	
	if (valAntiAliasing){  Fmt(s, "%4.0f", v); valAntiAliasing->setCaption(s);  }
}

///  resolutions
//  change
void App::btnResChng(WP)
{
	if (!resList)  return;
	if (resList->getIndexSelected() == MyGUI::ITEM_NONE) return;
	String mode = resList->getItemNameAt(resList->getIndexSelected());

	pSet->windowx = StringConverter::parseInt(StringUtil::split(mode, "x")[0]);
	pSet->windowy = StringConverter::parseInt(StringUtil::split(mode, "x")[1]);
	
	mWindow->resize(pSet->windowx, pSet->windowy);
	
	if (pSet->fullscreen)
		mWindow->setFullscreen(true, pSet->windowx, pSet->windowy);
	else
	{
	#ifdef _WIN32
		int sx = GetSystemMetrics(SM_CXSCREEN), sy = GetSystemMetrics(SM_CYSCREEN);
		int cx = std::max(0,(sx - pSet->windowx) / 2), cy = std::max(0,(sy - pSet->windowy) / 2);
		mWindow->reposition(cx,cy);
	#else
		//mWindow->reposition(0,0);  //TODO: linux window size,center ?..
	#endif
	}
	bWindowResized = true;
}


//  get screen resolutions
struct ScrRes {  int w,h;  String mode;  };

bool ResSort(const ScrRes& r1, const ScrRes& r2)
{
	return (r1.w <= r2.w) && (r1.h <= r2.h);
}

void App::InitGuiScrenRes()
{
	ButtonPtr bchk;
	Chk("FullScreen", chkVidFullscr, pSet->fullscreen);
	Chk("VSync", chkVidVSync, pSet->vsync);

	//  video resolutions combobox
	resList = mGUI->findWidget<List>("ResList");
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
				int ww = w - mWindow->getWidth(), hh = h - mWindow->getHeight();
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
	ButtonPtr btnRes = mGUI->findWidget<Button>("ResChange");
	if (btnRes)  {  btnRes->eventMouseButtonClick += newDelegate(this, &App::btnResChng);  }
}


//  resize Options wnd
void App::ResizeOptWnd()
{
	if (!mWndOpts)  return;

	const int wx = pSet->windowx, wy = pSet->windowy;
	const int yN = 7;
	const Real yw[yN] = {400, 600, 720, 768, 960, 1024, 1200};
	const Real yf[yN] = {0.0, 0.0, 0.05, 0.1, 0.2, 0.3,  0.3};

	Real xm = 0.f, ym = 0.f;  // margin
	for (int i=0; i < yN; ++i)
		if (wy >= yw[i]-10)  ym = yf[i];

	Real yo = (1.f - ym)*wy, xo = 4.f/3.f * yo;  // opt wnd size in pix
	ym = (wy - yo)*0.5f;  xm = (wx - xo)*0.5f;

	mWndOpts->setCoord(xm, ym, xo, yo);
	if (bnQuit)  //  reposition Quit btn
		bnQuit->setCoord(wx - 0.09*wx, 0, 0.09*wx, 0.03*wy);

	updTrkListDim();
}

void App::chkVidFullscr(WP wp)
{
	ChkEv(fullscreen);
	//TODO: got broken, crashes on win,  need to change res 1st then fullscr..
	mWindow->setFullscreen(pSet->fullscreen, pSet->windowx, pSet->windowy);
	mWindow->resize(pSet->windowx, pSet->windowy);
}

void App::chkVidVSync(WP wp)
{		
	ChkEv(vsync); 
	Ogre::Root::getSingleton().getRenderSystem()->setWaitForVerticalBlank(pSet->vsync);
}


///  change all Graphics settings
///..............................................................................................................................
void App::comboGraphicsAll(ComboBoxPtr cmb, size_t val)
{
	//"TexFiltering", comboTexFilter ?
	//fsaa = 0;  vsync = false;  //?  rpl?
	//  sim  - other combobox, not recommended_
	//game_fq = 100.f;  blt_fq = 60.f;  blt_iter = 7;  mult_thr = 0;
	//veget_collis = true;  car_collis = false;

	SETTINGS& s = *pSet;
	switch (val)        ///  common
	{
	case 0:  // Lowest  -------------
		s.anisotropy = 0;  s.view_distance = 1000;  s.terdetail = 2.0f;  s.terdist = 0.f;  s.road_dist = 2.0;
		s.tex_size = 0;  s.ter_mtr = 0;  s.shaders = 0;
		s.shadow_type = 0;/*0*/  s.shadow_size = 0;  s.shadow_count = 3;  s.shadow_dist = 1000;
		s.trees = 0.f;  s.grass = 0.f;  s.trees_dist = 1.f;  s.grass_dist = 1.f;	break;

	case 1:  // Low  -------------
		s.anisotropy = 0;  s.view_distance = 1500;  s.terdetail = 1.7f;  s.terdist = 40.f;  s.road_dist = 1.8;
		s.tex_size = 0;  s.ter_mtr = 1;  s.shaders = 0;
		s.shadow_type = 0;/*1*/  s.shadow_size = 0;  s.shadow_count = 3;  s.shadow_dist = 1000;
		s.trees = 0.f;  s.grass = 0.f;  s.trees_dist = 1.f;  s.grass_dist = 1.f;	break;

	case 2:  // Medium  -------------
		s.anisotropy = 4;  s.view_distance = 2500;  s.terdetail = 1.5f;  s.terdist = 80.f;  s.road_dist = 1.6;
		s.tex_size = 1;  s.ter_mtr = 1;  s.shaders = 1;
		s.shadow_type = 2;/*1*/  s.shadow_size = 1;  s.shadow_count = 3;  s.shadow_dist = 3000;
		s.trees = 0.5f;  s.grass = 0.f;  s.trees_dist = 1.f;  s.grass_dist = 1.f;	break;

	case 3:  // High  -------------
		s.anisotropy = 8;  s.view_distance = 6000;  s.terdetail = 1.3f;  s.terdist = 200.f;  s.road_dist = 1.4;
		s.tex_size = 1;  s.ter_mtr = 2;  s.shaders = 1;
		s.shadow_type = 2;/*2*/  s.shadow_size = 2;  s.shadow_count = 3;  s.shadow_dist = 3000;
		s.trees = 1.f;  s.grass = 1.f;  s.trees_dist = 1.f;  s.grass_dist = 1.f;	break;

	case 4:  // Ultra  -------------
		s.anisotropy = 16;  s.view_distance = 20000;  s.terdetail = 1.0f;  s.terdist = 1000.f;  s.road_dist = 1.2;
		s.tex_size = 1;  s.ter_mtr = 2;  s.shaders = 1;
		s.shadow_type = 3;/*3*/  s.shadow_size = 3;  s.shadow_count = 3;  s.shadow_dist = 3000;
		s.trees = 2.f;  s.grass = 2.f;  s.trees_dist = 2.f;  s.grass_dist = 2.f;	break;
	}
#ifndef ROAD_EDITOR  /// game only
	switch (val)
	{
	case 0:  // Lowest  -------------
		s.particles = false;  s.trails = false;  s.particles_len = 1.f;  s.trails_len = 1.f;
		s.refl_mode = "static";  s.refl_skip = 500;  s.refl_faces = 1;  s.refl_size = 0;  s.refl_dist = 100.f;
		s.all_effects = false;  s.bloom = false;  s.hdr = false;  s.motionblur = false;
		s.rpl_rec = 0;  s.rpl_ghost = 0;  s.rpl_alpha = 1;	break;

	case 1:  // Low  -------------
		s.particles = true;  s.trails = true;  s.particles_len = 1.f;  s.trails_len = 1.f;
		s.refl_mode = "static";  s.refl_skip = 300;  s.refl_faces = 1;  s.refl_size = 0;  s.refl_dist = 200.f;
		s.all_effects = false;  s.bloom = false;  s.hdr = false;  s.motionblur = false;
		s.rpl_rec = 1;  s.rpl_ghost = 1;  s.rpl_alpha = 1;  break;

	case 2:  // Medium  -------------
		s.particles = true;  s.trails = true;  s.particles_len = 1.f;  s.trails_len = 1.5f;
		s.refl_mode = "single";  s.refl_skip = 200;  s.refl_faces = 1;  s.refl_size = 0;  s.refl_dist = 500.f;
		s.all_effects = false;  s.bloom = false;  s.hdr = false;  s.motionblur = false;
		s.rpl_rec = 1;  s.rpl_ghost = 1;  s.rpl_alpha = 1;	break;

	case 3:  // High  -------------
		s.particles = true;  s.trails = true;  s.particles_len = 1.2f;  s.trails_len = 2.f;
		s.refl_mode = "full";    s.refl_skip = 50;  s.refl_faces = 1;  s.refl_size = 0;  s.refl_dist = 1000.f;
		s.all_effects = true;  s.bloom = true;  s.hdr = false;  s.motionblur = false;
		s.rpl_rec = 1;  s.rpl_ghost = 1;  s.rpl_alpha = 0;	break;

	case 4:  // Ultra  -------------
		s.particles = true;  s.trails = true;  s.particles_len = 1.5f;  s.trails_len = 4.f;
		s.refl_mode = "full";    s.refl_skip = 10;  s.refl_faces = 1;  s.refl_size = 1;  s.refl_dist = 1500.f;
		s.all_effects = true;  s.bloom = true;  s.hdr = false;  s.motionblur = false;  //true;
		s.rpl_rec = 1;  s.rpl_ghost = 1;  s.rpl_alpha = 0;	break;
	}
#endif

#ifdef ROAD_EDITOR  /// editor only
	switch (val)
	{
	case 0:  // Lowest  -------------
		s.trackmap = 0;  s.brush_prv = 0;	s.ter_skip = 20;  s.mini_skip = 20;  break;

	case 1:  // Low  -------------
		s.trackmap = 1;  s.brush_prv = 0;	s.ter_skip = 10;  s.mini_skip = 20;  break;

	case 2:  // Medium  -------------
		s.trackmap = 1;  s.brush_prv = 1;	s.ter_skip = 3;  s.mini_skip = 6;  break;

	case 3:  // High  -------------
		s.trackmap = 1;  s.brush_prv = 1;	s.ter_skip = 2;  s.mini_skip = 4;  break;

	case 4:  // Ultra  -------------
		s.trackmap = 1;  s.brush_prv = 1;	s.ter_skip = 1;  s.mini_skip = 1;  break;
	}
#endif

	//  update gui  sld,val,chk  ...
	GuiInitGraphics();  // += newDelegate..?
	changeShadows(); // apply shadow

	ButtonPtr btn, bchk;  ScrollBar* sl;  size_t v;
#ifndef ROAD_EDITOR  /// game only
	// duplicated code..
	Chk("ParticlesOn", chkParticles, pSet->particles);	Chk("TrailsOn", chkTrails, pSet->trails);
	Slv(Particles,	powf(pSet->particles_len /4.f, 0.5f));
	Slv(Trails,		powf(pSet->trails_len /4.f, 0.5f));

	Slv(ReflSkip,	powf(pSet->refl_skip /1000.f, 0.5f));
	Slv(ReflSize,	pSet->refl_size /res);
	Slv(ReflFaces,	pSet->refl_faces /res);
	Slv(ReflDist,	powf((pSet->refl_dist -20.f)/1480.f, 0.5f));
	int value=0;  if (pSet->refl_mode == "static")  value = 0;
	else if (pSet->refl_mode == "single")  value = 1;
	else if (pSet->refl_mode == "full")  value = 2;
	Slv(ReflMode,   value /res);

	Chk("Bloom", chkVidBloom, pSet->bloom);
	Chk("HDR", chkVidHDR, pSet->hdr);
	Chk("MotionBlur", chkVidBlur, pSet->motionblur);
	Chk("ssao", chkVidSSAO, pSet->ssao);

	Chk("RplChkAutoRec", chkRplAutoRec, pSet->rpl_rec);
	Chk("RplChkGhost", chkRplChkGhost, pSet->rpl_ghost);
	Chk("RplChkAlpha", chkRplChkAlpha, pSet->rpl_alpha);
#endif

#ifdef ROAD_EDITOR  /// editor only
	Chk("Minimap", chkMinimap, pSet->trackmap);
	Slv(TerUpd, pSet->ter_skip /res);
	Slv(MiniUpd, pSet->mini_skip /res);
#endif
}

void App::comboRenderSystem(ComboBoxPtr wp, size_t val)
{
	pSet->rendersystem = wp->getItemNameAt(val);
}
