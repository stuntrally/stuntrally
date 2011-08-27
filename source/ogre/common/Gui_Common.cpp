#include "pch.h"
#include "../Defines.h"
//#include "../../road/Road.h"
#ifndef ROAD_EDITOR
	//#include "../../vdrift/pathmanager.h"
	#include "../../vdrift/game.h"
	#include "../OgreGame.h"
	#include "../SplitScreen.h"
#else
	#include "../../editor/OgreApp.h"
#endif
#include <OgreMaterialManager.h>
#include <OgreSceneManager.h>
#include <OgreTerrain.h>
#include <OgreRenderWindow.h>
#include "Gui_Def.h"
using namespace MyGUI;
using namespace Ogre;


///  Gui Init
//----------------------------------------------------------------------------------------------------------------
//  [Graphics]

//  textures
void App::comboTexFilter(SL)
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
	Real v = 50.f + 6950.f * powf(val/res, 2.f);
	Vector3 sc = v*Vector3::UNIT_SCALE;

	SceneNode* nskb = mSceneMgr->getSkyBoxNode();
	if (nskb)  nskb->setScale(sc*0.58);
	else  if (ndSky)  ndSky->setScale(sc);

	if (bGI)  pSet->view_distance = v;
	if (valViewDist){	Fmt(s, "%4.1f km", v*0.001f);	valViewDist->setCaption(s);  }

	// Set new far clip distance for all cams
	#ifndef ROAD_EDITOR
	/*?if (bGI)*/  mSplitMgr->UpdateCamDist();
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
	HScrollPtr sl;  size_t v;
	#define setSld(name)  sl##name(0,v);  \
		sl = (HScrollPtr)mWndOpts->findWidget(#name);  if (sl)  sl->setScrollPosition(v);
	v = res*powf(1.f /4.f, 0.5f);
	setSld(Trees);
	setSld(Grass);
	v = res*powf((1.f-0.5f) /6.5f, 0.5f);
	setSld(TreesDist);
	setSld(GrassDist);
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
}

void App::slTerMtr(SL)
{
	int v = val;  if (bGI)  pSet->ter_mtr = v;
	if (valTerMtr)
	{	if (v == 0)  valTerMtr->setCaption("Lowest");  else
		if (v == 1)  valTerMtr->setCaption("Low");  else
		if (v == 2)  valTerMtr->setCaption("Normal");  else
		if (v == 3)  valTerMtr->setCaption("Parallax");  }
	if (bGI)  changeShadows();
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
void App::GuiCenterMouse()
{
	int xm = mWindow->getWidth()/2, ym = mWindow->getHeight()/2;
	MyGUI::InputManager::getInstance().injectMouseMove(xm, ym, 0);
	OIS::MouseState &ms = const_cast<OIS::MouseState&>(mMouse->getMouseState());
	ms.X.abs = xm;  ms.Y.abs = ym;
}

//  tooltip
void App::GuiInitTooltip()
{
	mToolTip = Gui::getInstance().findWidget<Widget>("ToolTip");
	mToolTip->setVisible(false);
	mToolTipTxt = mToolTip->getChildAt(0)->castType<Edit>();
}

void App::btnQuit(WP)
{
	mShutDown = true;
}

//  Languages combo
void App::GuiInitLang()
{
	supportedLanguages["en"] = "English";
	supportedLanguages["de"] = "Deutsch";  //German
	supportedLanguages["fi"] = "Suomi";   //Finnish
	supportedLanguages["ro"] = "Romana";  //Romanian â?
	supportedLanguages["pl"] = "Polski";  //Polish
	
	//ComboBoxPtr combo = mGUI->findWidget<ComboBox>("Lang");
	ComboBoxPtr combo = (ComboBoxPtr)mWndOpts->findWidget("Lang");
	if (!combo)  return;
	combo->eventComboChangePosition = newDelegate(this, &App::comboLanguage);
	for (std::map<std::string, std::string>::const_iterator it = supportedLanguages.begin();
		it != supportedLanguages.end(); it++)
	{
		combo->addItem(it->second);
		if (it->first == pSet->language)
			combo->setIndexSelected(combo->getItemCount()-1);
	}
}

void App::comboLanguage(SL)
{
	if (val == MyGUI::ITEM_NONE)  return;
	MyGUI::ComboBoxPtr cmb = static_cast<MyGUI::ComboBoxPtr>(wp);
	std::string sel = cmb->getItemNameAt(val);
	
	for (std::map<std::string, std::string>::const_iterator it = supportedLanguages.begin();
		it != supportedLanguages.end(); it++)
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
