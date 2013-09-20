#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/RenderConst.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include "../ogre/common/MultiList2.h"
#include "../sdl4ogre/sdlcursormanager.hpp"
#include "../sdl4ogre/sdlinputwrapper.hpp"
#include <OgreTerrain.h>
using namespace MyGUI;
using namespace Ogre;


//  set Gui from xml (scene, road), after track load
//..........................................................................................................
void CGui::SetGuiFromXmls()
{
	if (!app->mWndEdit)  return;
	bGI = false;
	
	#define _Ed(name, val)  ed##name->setCaption(toStr(val))
	#define _Clr(name, val)  clr##name->setColour(Colour(val.x,val.y,val.z))
	#define _Cmb(cmb, str)  cmb->setIndexSelected( cmb->findItemIndexWith(str) )
	

	//  [Sky]
	//-----------------------------------------------
	_Cmb(cmbSky, sc->skyMtr);
	svSunPitch.Upd();
	svSunYaw.Upd();
	_Ed(LiAmb, sc->lAmb);  _Ed(LiDiff, sc->lDiff);  _Ed(LiSpec, sc->lSpec);
	_Clr(Amb, sc->lAmb);  _Clr(Diff, sc->lDiff);  _Clr(Spec, sc->lSpec);
	//  fog
	_Clr(Fog, sc->fogClr);  _Clr(Fog2, sc->fogClr2);  _Clr(FogH, sc->fogClrH);
	_Ed(FogClr, sc->fogClr);  _Ed(FogClr2, sc->fogClr2);  _Ed(FogClrH, sc->fogClrH);
	svFogStart.Upd();	svFogEnd.Upd();
	svFogHStart.Upd();	svFogHEnd.Upd();
	svFogHeight.Upd();	svFogHDensity.Upd();
	svRain1Rate.Upd();	svRain2Rate.Upd();	
	_Cmb(cmbRain1, sc->rainName);
	_Cmb(cmbRain2, sc->rain2Name);
	
	//  [Terrain]
	//-----------------------------------------------
	updTabHmap();
	svTerTriSize.Upd();
	if (edTerErrorNorm)  edTerErrorNorm->setCaption(fToStr(sc->td.errorNorm,2,4));
	
	tabTerLayer(tabsTerLayers, idTerLay);
	_Cmb(cmbParDust, sc->sParDust);	_Cmb(cmbParMud,  sc->sParMud);
	_Cmb(cmbParSmoke,sc->sParSmoke);

	//  [Vegetation]
	//-----------------------------------------------
	_Ed(GrassDens, sc->densGrass);	_Ed(TreesDens, sc->densTrees);
	_Ed(TrPage, sc->trPage);		_Ed(TrDist, sc->trDist);
	_Ed(GrPage, sc->grPage);		_Ed(GrDist, sc->grDist);

	_Ed(TrRdDist, sc->trRdDist);	_Ed(TrImpDist, sc->trDistImp);
	_Ed(GrDensSmooth, sc->grDensSmooth);
	edSceneryId->setCaption(sc->sceneryId);

	tabGrLayers(tabsGrLayers, idGrLay);
	tabPgLayers(tabsPgLayers, idPgLay);

	//MeshPtr mp = MeshManager::load(sc->pgLayersAll[0].name);
	//mp->getSubMesh(0)->

	//  [Road]
	//-----------------------------------------------
	SplineRoad* rd = app->road;
	for (int i=0; i < 4/*MTRs*/; ++i)
	{	_Cmb(cmbRoadMtr[i], rd->sMtrRoad[i]);
		_Cmb(cmbPipeMtr[i], rd->sMtrPipe[i]);  }
	_Cmb(cmbRoadWMtr, rd->sMtrWall);  _Cmb(cmbRoadColMtr, rd->sMtrCol);
	_Cmb(cmbPipeWMtr, rd->sMtrWallPipe);

	_Ed(RdTcMul,  rd->tcMul);  _Ed(RdTcMulW,  rd->tcMul);
	_Ed(RdTcMulP, rd->tcMul);  _Ed(RdTcMulPW, rd->tcMul);  _Ed(RdTcMulC, rd->tcMul);
	_Ed(RdColN, rd->colN);  _Ed(RdPwsM, rd->iwPmul);
	_Ed(RdColR, rd->colR);  _Ed(RdPlsM, rd->ilPmul);
	_Ed(RdLenDim, rd->lenDiv0);  _Ed(RdWidthSteps, rd->iw0);
	_Ed(RdSkirtLen,	rd->skirtLen);  _Ed(RdHeightOfs,rd->fHeight);
	_Ed(RdSkirtH,	rd->skirtH);
	_Ed(RdMergeLen,	rd->setMrgLen);  _Ed(RdLodPLen,	rd->lposLen);
	bGI = true;
}


void CGui::btnNewGame(WP)
{
	if (trkName)  trkName->setCaption(sListTrack.c_str());
	pSet->gui.track = sListTrack;
	pSet->gui.track_user = bListTrackU;  //UpdWndTitle();//? load
	app->LoadTrack();
}


//  Update  input, info
//---------------------------------------------------------------------------------------------------------------
//  tool wnds show/hide
void App::UpdEditWnds()
{
	if (mWndBrush)
	{	if (edMode == ED_Deform)
		{	static_cast<StaticTextPtr>(mWndBrush)->setCaption("Terrain Deform");  
			mWndBrush->setColour(MyGUI::Colour(0.5f, 0.9f, 0.3f));
			mWndBrush->setVisible(true);  }
		else if (edMode == ED_Filter)
		{	static_cast<StaticTextPtr>(mWndBrush)->setCaption("Terrain Filter");
			mWndBrush->setColour(MyGUI::Colour(0.5f, 0.75f, 1.0f));
			mWndBrush->setVisible(true);  }
		else if (edMode == ED_Smooth)
		{	static_cast<StaticTextPtr>(mWndBrush)->setCaption("Terrain Smooth");
			mWndBrush->setColour(MyGUI::Colour(0.3f, 0.8f, 0.8f));
			mWndBrush->setVisible(true);  }
		else if (edMode == ED_Height)
		{	static_cast<StaticTextPtr>(mWndBrush)->setCaption("Terrain Height");
			mWndBrush->setColour(MyGUI::Colour(0.7f, 1.0f, 0.7f));
			mWndBrush->setVisible(true);  }
		else
			mWndBrush->setVisible(false);
	}
	if (mWndRoadCur) mWndRoadCur->setVisible(edMode == ED_Road);
	if (mWndCam)     mWndCam->setVisible(edMode == ED_PrvCam);
	
	if (mWndStart)   mWndStart->setVisible(edMode == ED_Start);

	if (mWndFluids)  mWndFluids->setVisible(edMode == ED_Fluids);
	UpdFluidBox();

	if (mWndObjects) mWndObjects->setVisible(edMode == ED_Objects);

	if (mWndRivers)  mWndRivers->setVisible(edMode == ED_Rivers);

	UpdStartPos();  // StBox visible
	UpdVisGui();  //br prv..

	UpdMtrWaterDepth();
}

void App::SetEdMode(ED_MODE newMode)
{
	static bool first = true;
	if (newMode == ED_Objects && first)
	{
		gui->SetObjNewType(gui->iObjTNew);
		first = false;
	}

	//if (pSet->autoWaterDepth)  //..?
	if (edMode == ED_Fluids && newMode != ED_Fluids)
		SaveWaterDepth();  // update, on exit from Fluids editing

	edMode = newMode;
}


//  wnd vis
void App::UpdVisGui()
{
	bool notMain = bGuiFocus && !pSet->isMain;
	if (mWndMain)	mWndMain->setVisible(bGuiFocus && pSet->isMain);
	if (mWndEdit)	mWndEdit->setVisible(notMain && pSet->inMenu == WND_Edit);
	if (mWndHelp)	mWndHelp->setVisible(notMain && pSet->inMenu == WND_Help);
	if (mWndOpts)	mWndOpts->setVisible(notMain && pSet->inMenu == WND_Options);

	if (gui->bnQuit)  gui->bnQuit->setVisible(bGuiFocus);

	bool vis = bGuiFocus || !bMoveCam;
	mCursorManager->cursorVisibilityChange(vis);
	mInputWrapper->setMouseRelative(!vis);
	mInputWrapper->setGrabPointer(!vis);

	if (road)  road->SetTerHitVis(bEdit());
	if (!bGuiFocus && gui->mToolTip)  gui->mToolTip->setVisible(false);

	if (ovBrushPrv)
	if (edMode >= ED_Road || bMoveCam)
		ovBrushPrv->hide();  else  ovBrushPrv->show();

	for (int i=0; i < WND_ALL; ++i)
		mWndMainPanels[i]->setVisible(pSet->inMenu == i);
		
	if (gui->txWarn)  gui->txWarn->setVisible(false);

	//  1st center mouse
	static bool first = true;
	if (bGuiFocus && first)
	{	first = false;
		gui->GuiCenterMouse();
	}
}

void CGui::toggleGui(bool toggle)
{
	if (app->edMode == ED_PrvCam)  return;
	if (toggle)
		app->bGuiFocus = !app->bGuiFocus;
	app->UpdVisGui();
}

void CGui::Status(String s, float r,float g,float b)
{
	app->ovStat->setColour(ColourValue(r,g,b));
	app->ovStat->setCaption(s);
	app->ovSt->setMaterialName("hud/Times");
	app->ovSt->show();
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
		rt[RTs].ndMini->setVisible(false);
		ndCar->setVisible(true);

		UpdateWaterRTT(mCamera);
		UpdFog();  // restore fog, veget
		if (oldV)  {  bTrGrUpd = true;  oldV = false;  }
		pSet->bWeather = oldI;
		UpdTerErr();

		sc->camPos = mCamera->getPosition();
		sc->camDir = mCamera->getDirection();
		mCamera->setPosition( mCamPosOld);
		mCamera->setDirection(mCamDirOld);
	}else  // enter
	{
		edModeOld = edMode;
		SetEdMode(ED_PrvCam);
		bMoveCam = true;  UpdVisGui();
		mViewport->setVisibilityMask(RV_MaskPrvCam);
		rt[RTs].ndMini->setVisible(true);
		ndCar->setVisible(false);

		UpdateWaterRTT(rt[3].rndCam);
		UpdFog(true);  // on fog, veget, weather
		if (!pSet->bTrees)  {  bTrGrUpd = true;  oldV = true;  }
		oldI = pSet->bWeather;  pSet->bWeather = false;
		mTerrainGlobals->setMaxPixelError(0.5f);  //hq ter

		mCamPosOld = mCamera->getPosition();
		mCamDirOld = mCamera->getDirection();
		mCamera->setPosition( sc->camPos);
		mCamera->setDirection(sc->camDir);
	}
	UpdEditWnds();
}


//  key util
void CGui::trkListNext(int rel)  //Gui..
{
	bool b = app->bGuiFocus && (app->mWndTabsEdit->getIndexSelected() == 1)
		&& !pSet->isMain && pSet->inMenu == WND_Edit;
	if (!b)  return;
	
	size_t cnt = trkList->getItemCount();
	if (cnt == 0)  return;
	int i = std::max(0, std::min((int)cnt-1, (int)trkList->getIndexSelected()+rel ));
	trkList->setIndexSelected(i);
	trkList->beginToItemAt(std::max(0, i-11));  // center
	listTrackChng(trkList,i);
}

void CGui::MainMenuBtn(MyGUI::WidgetPtr wp)
{
	for (int i=0; i < WND_ALL; ++i)
		if (wp == app->mWndMainBtns[i])
		{
			pSet->isMain = false;
			pSet->inMenu = i;
			toggleGui(false);
			return;
		}
}

void CGui::MenuTabChg(MyGUI::TabPtr tab, size_t id)
{
	if (id != 0)  return;
	tab->setIndexSelected(1);  // dont switch to 0
	pSet->isMain = true;
	toggleGui(false);  // back to main
}


//  Gui Shortcut  alt-letters
void CGui::GuiShortcut(WND_Types wnd, int tab, int subtab)
{
	if (subtab == -1 && (!app->bGuiFocus || pSet->inMenu != wnd))  subtab = -2;  // cancel subtab cycling

	if (!app->bGuiFocus)
	if (app->edMode != ED_PrvCam)  {
		app->bGuiFocus = !app->bGuiFocus;  app->UpdVisGui();  }

	//isFocGui = true;
	pSet->isMain = false;  pSet->inMenu = wnd;
	
	MyGUI::TabPtr mWndTabs = 0;
	std::vector<MyGUI::TabControl*>* subt = 0;
	
	switch (wnd)
	{	case WND_Edit:		mWndTabs = app->mWndTabsEdit;  subt = &vSubTabsEdit;  break;
		case WND_Help:		mWndTabs = app->mWndTabsHelp;  subt = &vSubTabsHelp;  break;
		case WND_Options:	mWndTabs = app->mWndTabsOpts;  subt = &vSubTabsOpts;  break;
	}
	toggleGui(false);


	size_t t = mWndTabs->getIndexSelected();
	mWndTabs->setIndexSelected(tab);

	if (!subt)  return;
	MyGUI::TabControl* tc = (*subt)[tab];  if (!tc)  return;
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
void CGui::NumTabNext(int rel)
{
	if (!app->bGuiFocus || pSet->isMain || pSet->inMenu != WND_Edit)  return;

	MyGUI::TabPtr tab = 0;

	#define tabNum(event)  {  \
		int cnt = tab->getItemCount();  \
		tab->setIndexSelected( (tab->getIndexSelected()+rel+cnt) % cnt );  \
		event(tab, tab->getIndexSelected());  }

	int id = app->mWndTabsEdit->getIndexSelected();
	switch (id)
	{
		case 4:  // Layers
		{	tab = tabsTerLayers;  tabNum(tabTerLayer);
		}	break;
		case 5:  // Vegetation
		{
			int sid = vSubTabsEdit[id]->getIndexSelected();
			switch (sid)
			{
			case 1:  tab = tabsGrLayers;  tabNum(tabGrLayers);  break;
			case 2:  tab = tabsPgLayers;  tabNum(tabPgLayers);  break;
			}
		}	break;
	}
	//Tab(tabsTerLayers, "TabTerLay", tabTerLayer);
	//Tab(tabsGrLayers, "LGrLayTab", tabGrLayers);
	//Tab(tabsPgLayers, "LTrNumTab", tabPgLayers);
}
