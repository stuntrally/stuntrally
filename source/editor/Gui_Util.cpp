#include "pch.h"
#include "enums.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/GuiCom.h"
#include "../ogre/common/CScene.h"
#include "../vdrift/pathmanager.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include "../road/PaceNotes.h"
#include "../ogre/common/MultiList2.h"
#include "../sdl4ogre/sdlcursormanager.hpp"
#include "../sdl4ogre/sdlinputwrapper.hpp"
#include <OgreTerrain.h>
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <OgreCamera.h>
#include <OgreSceneNode.h>
#include <OgreViewport.h>
#include <MyGUI.h>
using namespace MyGUI;
using namespace Ogre;


///  Update all Gui controls
///  (after track Load, or Copy)
///  basing on values in scene and road
//..........................................................................................................
void CGui::SetGuiFromXmls()
{
	if (!app->mWndEdit)  return;
	bGI = false;
	
	Vector3 c;
	#define _Ed(name, val)  ed##name->setCaption(toStr(val))
	#define _Clr(name, val)  c = val.GetRGB1();  clr##name->setColour(Colour(c.x,c.y,c.z))
	#define _Cmb(cmb, str)  cmb->setIndexSelected( cmb->findItemIndexWith(str) )
	

	//  [Sky]
	//-----------------------------------------------
	btnSky->setCaption(sc->skyMtr);
	String s = sc->skyMtr;  s = s.substr(4, s.length());  // sel on pick list
	for (size_t i=0; i < liSky->getItemCount(); ++i)
		if (liSky->getSubItemNameAt(1, i).substr(7) == s)
			liSky->setIndexSelected(i);

	svSkyYaw.Upd();  svSunPitch.Upd();  svSunYaw.Upd();
	_Clr(Amb, sc->lAmb);  _Clr(Diff, sc->lDiff);  _Clr(Spec, sc->lSpec);
	_Clr(Fog, sc->fogClr);  _Clr(Fog2, sc->fogClr2);  _Clr(FogH, sc->fogClrH);
	svFogStart.Upd();	svFogEnd.Upd();
	svFogHStart.Upd();	svFogHEnd.Upd();
	svFogHeight.Upd();	svFogHDensity.Upd();  svFogHDmg.Upd();
	svRain1Rate.Upd();	svRain2Rate.Upd();	
	_Cmb(cmbRain1, sc->rainName);
	_Cmb(cmbRain2, sc->rain2Name);
	_Cmb(cmbReverbs, sc->sReverbs);  UpdRevDescr();
	
	//  [Terrain]
	//-----------------------------------------------
	updTabHmap();
	svTerTriSize.Upd();
	svTerErrorNorm.Upd();  svTerNormScale.Upd();
	svTerSpecPow.Upd();  svTerSpecPowEm.Upd();
	
	tabTerLayer(tabsTerLayers, idTerLay);
	_Cmb(cmbParDust, sc->sParDust);	_Cmb(cmbParMud,  sc->sParMud);
	_Cmb(cmbParSmoke,sc->sParSmoke);

	//  [Vegetation]
	//-----------------------------------------------
	svGrassDens.Upd();  svTreesDens.Upd();
	_Ed(TrPage, sc->trPage);  _Ed(TrDist, sc->trDist);
	_Ed(GrPage, sc->grPage);  _Ed(GrDist, sc->grDist);  _Ed(TrImpDist, sc->trDistImp);
	svTrRdDist.Upd();  svGrDensSmooth.Upd();

	tabGrLayers(tabsGrLayers, idGrLay);
	tabPgLayers(tabsPgLayers, idPgLay);

	SetGuiRoadFromXml();

	//  [Game]
	//-----------------------------------------------
	ckDenyReversed.Upd(&sc->denyReversed);
	ckTiresAsphalt.Upd(&sc->asphalt);
	ckTerrainEmissive.Upd(&sc->td.emissive);
	ckNoWrongChks.Upd(&sc->noWrongChks);
	SldUpd_Game();
	UpdEdInfo();
	
	//  [Surface]
	//-----------------------------------------------
	UpdSurfList();
	listSurf(surfList, idSurf);
	
	bGI = true;
}

void CGui::SetGuiRoadFromXml()
{
	//  [Road]
	//-----------------------------------------------
	SplineRoad* rd = scn->road;
	for (int i=0; i < 4/*MTRs*/; ++i)
	{	btnRoad[i]->setCaption(rd->sMtrRoad[i]);
		btnPipe[i]->setCaption(rd->sMtrPipe[i]);
	}
	btnRoadW->setCaption(rd->sMtrWall);
	btnPipeW->setCaption(rd->sMtrWallPipe);
	btnRoadCol->setCaption(rd->sMtrCol);

	_Ed(RdHeightOfs, rd->g_Height);
	_Ed(RdSkirtLen, rd->g_SkirtLen);  _Ed(RdSkirtH, rd->g_SkirtH);
	SldUpd_Road();
	ckRoad1Mtr.Upd();
}


void CGui::btnNewGame(WP)
{
	if (trkName)  trkName->setCaption(gcom->sListTrack);
	pSet->gui.track = gcom->sListTrack;
	pSet->gui.track_user = gcom->bListTrackU;  //UpdWndTitle();//? load
	app->LoadTrack();
}

void CGui::btnEdTut(WP)
{
	PATHMANAGER::OpenUrl("https://stuntrally.tuxfamily.org/wiki/doku.php?id=editor");
}


//  Update  input, info
//-------------------------------------------------------------------------------
//  tool wnds show/hide
void App::UpdEditWnds()
{
	if (mWndBrush)
	{	if (edMode == ED_Deform)
		{	mWndBrush->setCaption(TR("D - #{TerDeform}"));
			mWndBrush->setColour(Colour(0.5f, 0.9f, 0.3f));
			mWndBrush->setVisible(true);
		}
		else if (edMode == ED_Filter)
		{	mWndBrush->setCaption(TR("F - #{TerFilter}"));
			mWndBrush->setColour(Colour(0.5f, 0.75f, 1.0f));
			mWndBrush->setVisible(true);  
		}
		else if (edMode == ED_Smooth)
		{	mWndBrush->setCaption(TR("S - #{TerSmooth}"));
			mWndBrush->setColour(Colour(0.3f, 0.8f, 0.8f));
			mWndBrush->setVisible(true);
		}
		else if (edMode == ED_Height)
		{	mWndBrush->setCaption(TR("E - #{TerHeight}"));
			mWndBrush->setColour(Colour(0.7f, 1.0f, 0.7f));
			mWndBrush->setVisible(true);
		}else
			mWndBrush->setVisible(false);
	}
	if (mWndRoadCur)  mWndRoadCur->setVisible(edMode == ED_Road);
	if (mWndCam)      mWndCam->setVisible(edMode == ED_PrvCam);
	
	if (mWndStart)    mWndStart->setVisible(edMode == ED_Start);

	if (mWndFluids)   mWndFluids->setVisible(edMode == ED_Fluids);
	UpdFluidBox();

	if (mWndObjects)   mWndObjects->setVisible(edMode == ED_Objects);

	if (mWndParticles) mWndParticles->setVisible(edMode == ED_Particles);
	UpdEmtBox();

	UpdStartPos();  // StBox visible
	UpdVisGui();  //br prv..

	UpdMtrWaterDepth();
}


//  change editor mode
//-----------------------------------------------
void App::SetEdMode(ED_MODE newMode)
{
	static bool first = true;
	if (newMode == ED_Objects && first)
	{
		SetObjNewType(iObjTNew);
		first = false;
	}
	if (ndObjBox)  ndObjBox->setVisible(newMode == ED_Objects && bEdit());
	if (ndEmtBox)  ndEmtBox->setVisible(newMode == ED_Particles && bEdit());

	//if (pSet->autoWaterDepth)  //..?
	if (edMode == ED_Fluids && newMode != ED_Fluids)
		SaveWaterDepth();  // update, on exit from Fluids editing

	edMode = newMode;
}


//  wnd vis
//-----------------------------------------------
void App::UpdVisGui()
{
	//  wnd
	bool g = bGuiFocus;
	bool notMain = g && !pSet->bMain;
	mWndMain->setVisible(g && pSet->bMain);
	mWndTrack->setVisible(notMain && pSet->inMenu == WND_Track);
	mWndEdit->setVisible(notMain && pSet->inMenu == WND_Edit);
	mWndHelp->setVisible(notMain && pSet->inMenu == WND_Help);
	mWndOpts->setVisible(notMain && pSet->inMenu == WND_Options);

	if (!g)  mWndPick->setVisible(false);
	if (!g)  mWndTrkFilt->setVisible(false);
	if (!g && gui->wndColor)  gui->wndColor->setVisible(false);
	if (gcom->bnQuit)  gcom->bnQuit->setVisible(g);

	//  mode
	if (gui->imgCam)
	{	gui->imgCam->setVisible(!g && bMoveCam);
		gui->imgEdit->setVisible(!g && !bMoveCam);
		gui->imgGui->setVisible(g);
	}

	bool vis = g || !bMoveCam;
	mInputWrapper->setMouseVisible(vis);
	mInputWrapper->setMouseRelative(!vis);
	mInputWrapper->setAllowGrab(pSet->mouse_capture);
	mInputWrapper->setGrabPointer(!vis);

	if (scn->road)  scn->road->SetTerHitVis(bEdit());
	if (!g && gcom->mToolTip)  gcom->mToolTip->setVisible(false);

	if (ovBrushPrv)
	if (edMode >= ED_Road || bMoveCam)
		ovBrushPrv->hide();  else  ovBrushPrv->show();

	for (int i=0; i < WND_ALL; ++i)
		mWndMainPanels[i]->setVisible(pSet->inMenu == i);
		
	if (gui->txWarn)  gui->txWarn->setVisible(false);

	//  1st center mouse
	static bool first = true;
	if (g && first)
	{	first = false;
		gcom->GuiCenterMouse();
	}
}

void CGui::toggleGui(bool toggle)
{
	if (app->edMode == ED_PrvCam)  return;
	if (toggle)
		app->bGuiFocus = !app->bGuiFocus;
	app->UpdVisGui();
}


//  bottom status bar
void CGui::Status(String s, float r,float g,float b)
{
	txtStatus->setCaption(TR(s));
	txtStatus->setTextColour(Colour(r,g,b));
	panStatus->setColour(Colour(r,g,b));
	panStatus->setAlpha(1.f);  panStatus->setVisible(true);
	app->fStFade = 1.5f;
}


///  Preview Camera mode  - - - - - - - - - - - - - - - - - - - - - - - -
void App::togPrvCam()
{
	static bool oldV = false, oldI = false;
	if (edMode == ED_PrvCam)  // leave
	{
		SetEdMode(edModeOld);
		mViewport->setVisibilityMask(RV_MaskAll);
		rt[RT_Last].ndMini->setVisible(false);
		ndCar->setVisible(true);

		scn->UpdateWaterRTT(mCamera);
		scn->UpdFog();  // restore fog, veget
		if (oldV)  {  bTrGrUpd = true;  oldV = false;  }
		pSet->bWeather = oldI;
		scn->UpdTerErr();

		scn->sc->camPos = mCamera->getPosition();
		scn->sc->camDir = mCamera->getDirection();
		mCamera->setPosition( mCamPosOld);
		mCamera->setDirection(mCamDirOld);
	}else  // enter
	{
		edModeOld = edMode;
		SetEdMode(ED_PrvCam);
		bMoveCam = true;  UpdVisGui();
		mViewport->setVisibilityMask(RV_MaskPrvCam);
		rt[RT_Last].ndMini->setVisible(true);
		ndCar->setVisible(false);

		scn->UpdateWaterRTT(rt[RT_View].cam);
		scn->UpdFog(true);  // on fog, veget, weather
		if (!pSet->bTrees)  {  bTrGrUpd = true;  oldV = true;  }
		oldI = pSet->bWeather;  pSet->bWeather = false;
		scn->mTerrainGlobals->setMaxPixelError(0.5f);  //hq ter

		mCamPosOld = mCamera->getPosition();
		mCamDirOld = mCamera->getDirection();
		mCamera->setPosition( scn->sc->camPos);
		mCamera->setDirection(scn->sc->camDir);
	}
	UpdEditWnds();
}


//  Gui Shortcut  alt-letters
//.......................................................................................
void CGui::GuiShortcut(WND_Types wnd, int tab, int subtab)
{
	if (subtab == -1 && (!app->bGuiFocus || pSet->inMenu != wnd))  subtab = -2;  // cancel subtab cycling

	if (!app->bGuiFocus)
	if (app->edMode != ED_PrvCam)  {
		app->bGuiFocus = !app->bGuiFocus;  app->UpdVisGui();  }

	//isFocGui = true;
	pSet->bMain = false;  pSet->inMenu = wnd;
	
	TabPtr mWndTabs = 0;
	std::vector<TabControl*>* subt = 0;
	
	switch (wnd)
	{	case WND_Track:		mWndTabs = app->mWndTabsTrack; subt = &vSubTabsTrack; break;
		case WND_Edit:		mWndTabs = app->mWndTabsEdit;  subt = &vSubTabsEdit;  break;
		case WND_Help:		mWndTabs = app->mWndTabsHelp;  subt = &vSubTabsHelp;  break;
		case WND_Options:	mWndTabs = app->mWndTabsOpts;  subt = &vSubTabsOpts;  break;
	}
	if (wnd != WND_Edit)
		app->mWndPick->setVisible(false);
	toggleGui(false);


	size_t t = mWndTabs->getIndexSelected();
	mWndTabs->setIndexSelected(tab);

	if (!subt)  return;
	TabControl* tc = (*subt)[tab];  if (!tc)  return;
	int  cnt = tc->getItemCount();

	if (t == tab && subtab == -1)  // cycle subpages if same tab
	{	if (app->shift)
			tc->setIndexSelected( (tc->getIndexSelected()-1+cnt) % cnt );
		else
			tc->setIndexSelected( (tc->getIndexSelected()+1) % cnt );
	}
	if (subtab > -1)
		tc->setIndexSelected( std::min(cnt-1, subtab) );
}


//  next num tab  alt-1,2
//.......................................................................................
void CGui::NumTabNext(int rel)
{
	if (!app->bGuiFocus || pSet->bMain /*|| pSet->inMenu != WND_Edit*/)  return;

	TabPtr tab = 0;

	#define tabNum(event)  {  \
		int cnt = tab->getItemCount();  \
		tab->setIndexSelected( (tab->getIndexSelected()+rel+cnt) % cnt );  \
		event(tab, tab->getIndexSelected());  }

	int id = app->mWndTabsEdit->getIndexSelected();
	switch (id)
	{
		case TAB_Layers:  tab = tabsTerLayers;  tabNum(tabTerLayer);  break;
		case TAB_Grass:  switch (vSubTabsEdit[id]->getIndexSelected())
		{	case 1:  tab = tabsGrLayers;  tabNum(tabGrLayers);  break;
			case 2:  tab = tabsGrChan;  tabNum(tabGrChan);  break;  }  break;
		case TAB_Veget:  tab = tabsPgLayers;  tabNum(tabPgLayers);  break;
		case TAB_Surface:  {  int t = (surfList->getIndexSelected() +5 +rel) % 5;
			surfList->setIndexSelected(t);  listSurf(surfList, t);  }  break;
	}
}


///  Update (frame start)  .,.,.,.,..,.,.,.,..,.,.,.,..,.,.,.,.
void CGui::GuiUpdate()
{
	gcom->UnfocusLists();
	
	if (iLoadNext)  // load next/prev track  (warnings)
	{	size_t cnt = gcom->trkList->getItemCount();
		if (cnt > 0)  
		{	int i = std::max(0, std::min((int)cnt-1, (int)gcom->trkList->getIndexSelected() + iLoadNext ));
			iLoadNext = 0;
			gcom->trkList->setIndexSelected(i);
			gcom->trkList->beginToItemAt(std::max(0, i-11));  // center
			gcom->listTrackChng(gcom->trkList,i);
			btnNewGame(0);
	}	}
	
	if (gcom->bGuiReinit)  // after language change from combo
	{	gcom->bGuiReinit = false;

		mGui->destroyWidgets(app->vwGui);
		gcom->bnQuit=0; app->mWndOpts=0; gcom->trkList=0; //todo: rest too..

		bGI = false;
		InitGui();
		
		SetGuiFromXmls();
		app->bWindowResized = true;
	}

	//  sort trk list
	gcom->SortTrkList();


	if (app->bWindowResized)
	{	app->bWindowResized = false;

		gcom->ResizeOptWnd();
		//bSizeHUD = true;
		gcom->SizeGUI();
		gcom->updTrkListDim();
		viewCanvas->setCoord(GetViewSize());

		//slSizeMinimap(0);
		//LoadTrack();  // shouldnt be needed ...
	}

}


///  Color tool window
//...............................................................................
void CGui::btnClrSet(WP w)
{
	SColor* v;  // rgb
	if (w == clrAmb)   v = &sc->lAmb;   else
	if (w == clrDiff)  v = &sc->lDiff;  else
	if (w == clrSpec)  v = &sc->lSpec;
	bool oth = wpClrSet != w;  wpClrSet = w;

	svAlp.setVisible(false);
	svHue.UpdF(&v->h);  svSat.UpdF(&v->s);  svVal.UpdF(&v->v);
	svNeg.UpdF(&v->n);

	IntPoint p = w->getAbsolutePosition();  p.left += 100;  p.top -= 50;
	wndColor->setPosition(p);
	if (!(wndColor->getVisible() && oth))  // dont hide if changed
		wndColor->setVisible(!wndColor->getVisible());
}
void CGui::btnClrSetA(WP w)
{
	SColor* v;  // rgba
	if (w == clrFog)   v = &sc->fogClr;   else
	if (w == clrFog2)  v = &sc->fogClr2;  else
	if (w == clrFogH)  v = &sc->fogClrH;  else
	if (w == clrTrail) {
		TerLayer* l = GetTerRdLay();
		v = &l->tclr;  }
	bool oth = wpClrSet != w;  wpClrSet = w;

	svAlp.setVisible(true);
	svHue.UpdF(&v->h);  svSat.UpdF(&v->s);  svVal.UpdF(&v->v);
	svAlp.UpdF(&v->a);  svNeg.UpdF(&v->n);

	IntPoint p = w->getAbsolutePosition();  p.left += 100;  p.top -= 50;
	wndColor->setPosition(p);
	if (!(wndColor->getVisible() && oth))
		wndColor->setVisible(!wndColor->getVisible());
}

void CGui::slUpdClr(SV* sv)
{
	SColor c(svHue.getF(), svSat.getF(), svVal.getF(), svAlp.getF(), svNeg.getF());
	Vector3 cc = c.GetRGB1();
	wpClrSet->setColour(Colour(cc.x,cc.y,cc.z));  // img

	WP w = wpClrSet;
	if (w == clrAmb) {  sc->lAmb = c;    scn->UpdSun();  }else
	if (w == clrDiff){  sc->lDiff = c;   scn->UpdSun();  }else
	if (w == clrSpec){  sc->lSpec = c;   scn->UpdSun();  }else

	if (w == clrFog) {  sc->fogClr = c;   scn->UpdFog();  }else
	if (w == clrFog2){  sc->fogClr2 = c;  scn->UpdFog();  }else
	if (w == clrFogH){  sc->fogClrH = c;  scn->UpdFog();  }else

	if (w == clrTrail) {
		TerLayer* l = GetTerRdLay();
		l->tclr = c;  }
}
