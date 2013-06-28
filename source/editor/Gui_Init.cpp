#include "pch.h"
#include "../ogre/common/Defines.h"
#include "OgreApp.h"
#include "../vdrift/pathmanager.h"

#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/MultiList2.h"
#include "../ogre/common/Slider.h"
#include <boost/filesystem.hpp>

using namespace MyGUI;
using namespace Ogre;


///  Gui Init
//----------------------------------------------------------------------------------------------------------------------

void App::InitGui()
{
	if (!mGUI)  return;
	QTimer ti;  ti.update();  /// time

	//  new widgets
	FactoryManager::getInstance().registerFactory<MultiList2>("Widget");
	FactoryManager::getInstance().registerFactory<Slider>("Widget");
	int i;

	//  load layout - wnds
	vwGui = LayoutManager::getInstance().loadLayout("Editor.layout");
	for (VectorWidgetPtr::iterator it = vwGui.begin(); it != vwGui.end(); ++it)
	{
		const std::string& name = (*it)->getName();
		setToolTips((*it)->getEnumerator());
		if (name == "MainMenuWnd"){  mWndMain = *it;	} else
		if (name == "EditorWnd")  {  mWndEdit = *it;	} else
		if (name == "OptionsWnd") {  mWndOpts = *it;	} else
		if (name == "HelpWnd")    {  mWndHelp = *it;	} else

		if (name == "CamWnd")     {  mWndCam = *it;		(*it)->setPosition(0,64);	} else
		if (name == "StartWnd")   {  mWndStart = *it;	(*it)->setPosition(0,64);	} else
		if (name == "BrushWnd")   {  mWndBrush = *it;	(*it)->setPosition(0,64);	} else

		if (name == "RoadCur")    {  mWndRoadCur = *it;		(*it)->setPosition(0,34);	} else
		if (name == "RoadStats")  {  mWndRoadStats = *it;	(*it)->setPosition(0,328);	} else

		if (name == "FluidsWnd")  {  mWndFluids = *it;	(*it)->setPosition(0,64);	} else
		if (name == "ObjectsWnd") {  mWndObjects = *it;	(*it)->setPosition(0,64);	} else
		if (name == "RiversWnd")  {  mWndRivers = *it;	(*it)->setPosition(0,64);	}
	}
	if (mWndRoadStats)  mWndRoadStats->setVisible(false);

	//  main menu
	for (i=0; i < WND_ALL; ++i)
	{
		const String s = toStr(i);
		mWndMainPanels[i] = mWndMain->findWidget("PanMenu"+s);
		mWndMainBtns[i] = (ButtonPtr)mWndMain->findWidget("BtnMenu"+s);
		mWndMainBtns[i]->eventMouseButtonClick += newDelegate(this, &App::MainMenuBtn);
	}
	//  center
	int sx = mWindow->getWidth(), sy = mWindow->getHeight();
	IntSize w = mWndMain->getSize();
	mWndMain->setPosition((sx-w.width)*0.5f, (sy-w.height)*0.5f);


	GuiInitTooltip();
	
	//  assign controls, tool window texts  ----------------------
	if (mWndBrush)
	for (i=0; i<BR_TXT; ++i)
	{
		brTxt[i] = mGUI->findWidget<StaticText>("brTxt"+toStr(i),false);
		brVal[i] = mGUI->findWidget<StaticText>("brVal"+toStr(i),false);
		brKey[i] = mGUI->findWidget<StaticText>("brKey"+toStr(i),false);
	}
	brImg = mGUI->findWidget<StaticImage>("brushImg", false);

	if (mWndRoadCur)
	for (i=0; i<RD_TXT; ++i)
	{	rdTxt[i] = mGUI->findWidget<StaticText>("rdTxt"+toStr(i),false);
		rdVal[i] = mGUI->findWidget<StaticText>("rdVal"+toStr(i),false);
		rdKey[i] = mGUI->findWidget<StaticText>("rdKey"+toStr(i),false);
	}
	if (mWndRoadStats)
	for (i=0; i<RDS_TXT; ++i)
	{	rdTxtSt[i] = mGUI->findWidget<StaticText>("rdTxtSt"+toStr(i),false);
		rdValSt[i] = mGUI->findWidget<StaticText>("rdValSt"+toStr(i),false);
	}
	
	if (mWndStart)
		for (i=0; i<ST_TXT; ++i)	stTxt[i] = mGUI->findWidget<StaticText>("stTxt"+toStr(i),false);

	if (mWndFluids)
		for (i=0; i<FL_TXT; ++i)	flTxt[i] = mGUI->findWidget<StaticText>("flTxt"+toStr(i),false);
		
	if (mWndObjects)
		for (i=0; i<OBJ_TXT; ++i)	objTxt[i] = mGUI->findWidget<StaticText>("objTxt"+toStr(i),false);
	objPan = mGUI->findWidget<Widget>("objPan",false);  if (objPan)  objPan->setVisible(false);

	if (mWndRivers)
		for (i=0; i<RI_TXT; ++i)	riTxt[i] = mGUI->findWidget<StaticText>("riTxt"+toStr(i),false);
		
	//  Tabs
	TabPtr tab;
	tab = mGUI->findWidget<Tab>("TabWndEdit");  mWndTabsEdit = tab;  tab->setIndexSelected(1);  tab->eventTabChangeSelect += newDelegate(this, &App::MenuTabChg);
	tab = mGUI->findWidget<Tab>("TabWndOpts");	mWndTabsOpts = tab;	 tab->setIndexSelected(1);	tab->eventTabChangeSelect += newDelegate(this, &App::MenuTabChg);
	tab = mGUI->findWidget<Tab>("TabWndHelp");	mWndTabsHelp = tab;	 tab->setIndexSelected(1);	tab->eventTabChangeSelect += newDelegate(this, &App::MenuTabChg);

	//  Options
	if (mWndOpts)
	{	/*mWndOpts->setVisible(false);
		int sx = mWindow->getWidth(), sy = mWindow->getHeight();
		IntSize w = mWndOpts->getSize();  // center
		mWndOpts->setPosition((sx-w.width)*0.5f, (sy-w.height)*0.5f);*/

		//  get sub tabs
		vSubTabsEdit.clear();
		TabPtr sub;
		for (size_t i=0; i < mWndTabsEdit->getItemCount(); ++i)
		{
			sub = (TabPtr)mWndTabsEdit->getItemAt(i)->findWidget("SubTab");
			vSubTabsEdit.push_back(sub);  // 0 for not found
		}
		vSubTabsHelp.clear();
		for (size_t i=0; i < mWndTabsHelp->getItemCount(); ++i)
		{
			sub = (TabPtr)mWndTabsHelp->getItemAt(i)->findWidget("SubTab");
			vSubTabsHelp.push_back(sub);
		}
		vSubTabsOpts.clear();
		for (size_t i=0; i < mWndTabsOpts->getItemCount(); ++i)
		{
			sub = (TabPtr)mWndTabsOpts->getItemAt(i)->findWidget("SubTab");
			vSubTabsOpts.push_back(sub);
		}
		//mWndTabs->setIndexSelected(3);  //default*--
		ResizeOptWnd();
	}

	//  center mouse pos
	PointerManager::getInstance().setVisible(bGuiFocus || !bMoveCam);
	GuiCenterMouse();
	
	//  hide  ---
	SetEdMode(ED_Deform);  UpdEditWnds();  // *  UpdVisHit(); //after track
	if (!mWndOpts) 
	{
		LogO("WARNING: failed to create options window");
		return;  // error
	}
	
	ButtonPtr btn, bchk;  ComboBoxPtr combo;  // for defines
	Slider* sl;

	///  [Graphics]
	//------------------------------------------------------------------------
	GuiInitGraphics();


	///  [Settings]
	//------------------------------------------------------------------------
	Chk("Minimap", chkMinimap, pSet->trackmap);
	Slv(SizeMinmap,	(pSet->size_minimap-0.15f) /1.85f);
	Slv(CamSpeed, powf((pSet->cam_speed-0.1f) / 3.9f, 1.f));
	Slv(CamInert, pSet->cam_inert);
	Slv(TerUpd, pSet->ter_skip /20.f);
	Slv(MiniUpd, pSet->mini_skip /20.f);
	Slv(SizeRoadP, (pSet->road_sphr-0.1f) /11.9f);
	Chk("AutoBlendmap", chkAutoBlendmap, pSet->autoBlendmap);  chAutoBlendmap = bchk;
	Chk("CamPos", chkCamPos, pSet->camPos);
	Chk("InputBar", chkInputBar, pSet->inputBar);  chInputBar = bchk;

	//  set camera btns
	Btn("CamView1", btnSetCam);  Btn("CamView2", btnSetCam);
	Btn("CamView3", btnSetCam);  Btn("CamView4", btnSetCam);
	Btn("CamTop", btnSetCam);
	Btn("CamLeft", btnSetCam);   Btn("CamRight", btnSetCam);
	Btn("CamFront", btnSetCam);  Btn("CamBack", btnSetCam);

	//  startup
	Chk("MouseCapture", chkMouseCapture, pSet->capture_mouse);
	Chk("StartInMain", chkStartInMain, pSet->startInMain);

	Chk("AutoStart", chkAutoStart, pSet->autostart);
	Chk("EscQuits", chkEscQuits, pSet->escquit);
	Chk("OgreDialog", chkOgreDialog, pSet->ogre_dialog);

	bnQuit = mGUI->findWidget<Button>("Quit");
	if (bnQuit)  {  bnQuit->eventMouseButtonClick += newDelegate(this, &App::btnQuit);  bnQuit->setVisible(false);  }
	

	///  [Sun]
	//----------------------------------------------------------------------------------------------
	Slv(SunPitch,0);  Slv(SunYaw,0);
	Slv(FogStart,0);  Slv(FogEnd,0);  Slv(FogHStart,0);  Slv(FogHEnd,0);  Slv(FogHeight,0);  Slv(FogHDensity,0);
	Chk("FogDisable", chkFogDisable, pSet->bFog);  chkFog = bchk;
	Chk("WeatherDisable", chkWeatherDisable, pSet->bWeather);  chkWeather = bchk;
	Ed(LiAmb, editLiAmb);  Ed(LiDiff, editLiDiff);  Ed(LiSpec, editLiSpec);
	Ed(FogClr, editFogClr);  Ed(FogClr2, editFogClr2);  Ed(FogClrH, editFogClrH);
	clrAmb = mGUI->findWidget<ImageBox>("ClrAmb");		clrDiff = mGUI->findWidget<ImageBox>("ClrDiff");
	clrSpec = mGUI->findWidget<ImageBox>("ClrSpec");	clrTrail = mGUI->findWidget<ImageBox>("ClrTrail");
	clrFog = mGUI->findWidget<ImageBox>("ClrFog");	clrFog2 = mGUI->findWidget<ImageBox>("ClrFog2");
	clrFogH = mGUI->findWidget<ImageBox>("ClrFogH");	//Todo: on click event - open color r,g,b dialog
	Slv(Rain1Rate,0);  Slv(Rain2Rate,0);


	///  [Terrain]
	//------------------------------------------------------------------------
	imgTexDiff = mGUI->findWidget<StaticImage>("TerImgDiff");
	Tab(tabsHmap, "TabHMapSize", tabHmap);
	Tab(tabsTerLayers, "TabTerLay", tabTerLayer);

	Edt(edTerTriSize, "edTerTriSize", editTerTriSize);
	Edt(edTerErrorNorm, "edTerErrorNorm", editTerErrorNorm);
	Edt(edTerLScale, "edTerLScale", editTerLScale);
	Slv(TerTriSize,	powf((sc->td.fTriangleSize -0.1f)/5.9f, 0.5f));
	Slv(TerLScale, 0);  sldTerLScale = sl;
	Btn("TerrainNew", btnTerrainNew);
	Btn("TerrainGenerate", btnTerGenerate);
	Btn("TerrainHalf", btnTerrainHalf);
	Btn("TerrainDouble", btnTerrainDouble);
	Btn("TerrainMove", btnTerrainMove);

	for (i=0; i < brSetsNum; ++i)  // brush preset
	{
		const BrushSet& st = brSets[i];  const String s = toStr(i);
		int x,y, xt,yt, sx;
		if (i < 14)	{  x = 10+      i*50;  y =  10;  xt= x + 20;  yt= y + 50;  sx = 48;  } else
		if (i < 24)	{  x = 20+ (i-14)*70;  y = 100;  xt= x + 25;  yt= y + 55;  sx = 64;  }
		else		{  x = 20+ (i-24)*70;  y = 190;  xt= x + 25;  yt= y + 55;  sx = 64;  }

		ScrollView* sv = mGUI->findWidget<ScrollView>("svBrushes");

		StaticImage* img = sv->createWidget<StaticImage>("ImageBox", x,y, sx,sx, Align::Default, "brI"+s);
		img->eventMouseButtonClick += newDelegate(this, &App::btnBrushPreset);
		img->setUserString("tip", st.name);  img->setNeedToolTip(true);
		img->setImageTexture("brush"+s+".png");
		img->eventToolTip += newDelegate(this, &App::notifyToolTip);
		setOrigPos(img, "EditorWnd");
		
		StaticText* txt = sv->createWidget<StaticText>("TextBox", xt,yt, 40,22, Align::Default, "brT"+s);
		txt->setCaption(fToStr(st.Size,0,2));
		txt->setTextColour(Colour(0.6,0.8,1.0));
		setOrigPos(txt, "EditorWnd");
	}

	Slv(TerGenScale,powf(pSet->gen_scale /160.f, 1.f/2.f));  // generate
	Slv(TerGenOfsX, (pSet->gen_ofsx+2.f) /4.f);
	Slv(TerGenOfsY, (pSet->gen_ofsy+2.f) /4.f);
	Slv(TerGenOct,  Real(pSet->gen_oct)	/9.f);
	Slv(TerGenFreq, pSet->gen_freq    /0.7f);
	Slv(TerGenPers, pSet->gen_persist /0.7f);
	Slv(TerGenPow,  powf(pSet->gen_pow /6.f,  1.f/2.f));


	///  [Layers]  ------------------------------------
	Chk("TerLayOn", chkTerLayOn, 1);  chkTerLay = bchk;
	valTerLAll = mGUI->findWidget<StaticText>("TerLayersAll");
	Chk("TexNormAuto", chkTexNormAutoOn, 1);  chkTexNormAuto = bchk;
	Chk("TerLayTripl", chkTerLayTriplOn, 1);  chkTerLayTripl = bchk;
	
	Slv(TerLAngMin,0);  Slv(TerLHMin,0);  Slv(TerLAngSm,0);  // blendmap
	Slv(TerLAngMax,0);  Slv(TerLHMax,0);  Slv(TerLHSm,0);
	Slv(TerLNoise,0);   Chk("TerLNoiseOnly", chkTerLNoiseOnlyOn, 0);  chkTerLNoiseOnly = bchk;
	
	Ed(LDust, editLDust);	Ed(LDustS, editLDust);
	Ed(LMud,  editLDust);	Ed(LSmoke, editLDust);
	Ed(LTrlClr, editLTrlClr);
	Cmb(cmbParDust, "CmbParDust", comboParDust);
	Cmb(cmbParMud,  "CmbParMud",  comboParDust);
	Cmb(cmbParSmoke,"CmbParSmoke",comboParDust);

	Cmb(cmbSurface, "Surface", comboSurface);
	txtSuBumpWave	= mGUI->findWidget<StaticText>("SuBumpWave");
	txtSuBumpAmp	= mGUI->findWidget<StaticText>("SuBumpAmp");
	txtSuRollDrag	= mGUI->findWidget<StaticText>("SuRollDrag");
	txtSuFrict		= mGUI->findWidget<StaticText>("SuFrict");
	txtSurfTire		= mGUI->findWidget<StaticText>("SurfTire");
	txtSurfType		= mGUI->findWidget<StaticText>("SurfType");

	
	///  [Vegetation]  ------------------------------------
	Ed(GrassDens, editTrGr);  Ed(TreesDens, editTrGr);
	Ed(GrPage, editTrGr);  Ed(GrDist, editTrGr);  Ed(TrPage, editTrGr);  Ed(TrDist, editTrGr);
	Ed(TrRdDist, editTrGr);  Ed(TrImpDist, editTrGr);
	Ed(GrDensSmooth, editTrGr);  Ed(SceneryId, editTrGr);

	Chk("LTrEnabled", chkPgLayOn, 1);  chkPgLay = bchk;
	valLTrAll = mGUI->findWidget<StaticText>("LTrAll");
	Tab(tabsPgLayers, "LTrNumTab", tabPgLayers);

	Slv(LTrDens, 0);	Slv(LTrRdDist, 0);  Slv(LTrRdDistMax, 0);
	Slv(LTrMinSc, 0);	Slv(LTrMaxSc, 0);	Slv(LTrWindFx, 0);	Slv(LTrWindFy, 0);
	Slv(LTrMaxTerAng, 0);  Ed(LTrMinTerH, editLTrMinTerH);  Ed(LTrMaxTerH, editLTrMaxTerH);
	Ed(LTrFlDepth, editLTrFlDepth);

	///  grass
	Slv(GrMinX, 0);  Slv(GrMaxX, 0);  Slv(GrMinY, 0);  Slv(GrMaxY, 0);
	Ed(GrSwayDistr, editTrGr);  Ed(GrSwayLen, editTrGr);  Ed(GrSwaySpd, editTrGr);
	Ed(GrTerMaxAngle, editTrGr);  Ed(GrTerSmAngle, editTrGr);
	Ed(GrTerMinHeight, editTrGr);  Ed(GrTerMaxHeight, editTrGr);  Ed(GrTerSmHeight, editTrGr);
	Cmb(cmbGrassMtr, "CmbGrMtr", comboGrassMtr);	imgGrass = mGUI->findWidget<StaticImage>("ImgGrass");
	Cmb(cmbGrassClr, "CmbGrClr", comboGrassClr);	imgGrClr = mGUI->findWidget<StaticImage>("ImgGrClr");

	Chk("LGrEnabled", chkGrLayOn, 1);  chkGrLay = bchk;
	valLGrAll = mGUI->findWidget<StaticText>("LGrAll");
	Tab(tabsGrLayers, "LGrLayTab", tabGrLayers);
	Slv(LGrDens, 0);

	
	///  [Road]  ------------------------------------
	Ed(RdTcMul, editRoad);  Ed(RdTcMulW, editRoad);
	Ed(RdTcMulP, editRoad);  Ed(RdTcMulPW, editRoad);  Ed(RdTcMulC, editRoad);
	Ed(RdLenDim, editRoad);  Ed(RdWidthSteps,editRoad);
	Ed(RdHeightOfs, editRoad);  Ed(RdSkirtLen, editRoad);  Ed(RdSkirtH, editRoad);
	Ed(RdMergeLen, editRoad);  Ed(RdLodPLen, editRoad);
	Ed(RdColN, editRoad);  Ed(RdColR, editRoad);
	Ed(RdPwsM, editRoad);  Ed(RdPlsM, editRoad);
	

	///  [Tools]  ------------------------------------
	Btn("TrackCopySel", btnTrkCopySel);
	valTrkCpySel = mGUI->findWidget<StaticText>("TrkCopySelName");
	Btn("CopySun", btnCopySun);				Btn("CopyTerHmap", btnCopyTerHmap);
	Btn("CopyTerLayers", btnCopyTerLayers);	Btn("CopyVeget", btnCopyVeget);
	Btn("CopyRoad", btnCopyRoad);			Btn("CopyRoadPars", btnCopyRoadPars);
	Btn("DeleteRoad", btnDeleteRoad);		Btn("DeleteFluids", btnDeleteFluids);
	Btn("DeleteObjects", btnDeleteObjects);
	Btn("ScaleAll", btnScaleAll);	Ed(ScaleAllMul, editScaleAllMul);
	Btn("ScaleTerH", btnScaleTerH);	Ed(ScaleTerHMul, editScaleTerHMul);

	Slv(AlignWidthAdd, pSet->al_w_add /20.f);
	Slv(AlignWidthMul, (pSet->al_w_mul-1.f) /4.f);
	Slv(AlignSmooth, pSet->al_smooth /6.f);
	

	///  Fill Combo boxes  . . . . . . .
	//------------------------------------------------------------------------------------------------------------

	GuiInitLang();
	
	//---------------------  Skies  ---------------------
	Cmb(cmbSky, "SkyCombo", comboSky);
	String sMat = PATHMANAGER::Data()+"/materials/";  // path

	GetMaterialsMat(sMat+"sky.mat");
	for (size_t i=0; i < vsMaterials.size(); ++i)
	{	const String& s = vsMaterials[i];
		if (s != "" && s != "base_sky")
			cmbSky->addItem(s);  //LogO(s);
	}
	//---------------------  Weather  ---------------------
	Cmb(cmbRain1, "Rain1Cmb", comboRain1);  cmbRain1->addItem("");
	Cmb(cmbRain2, "Rain2Cmb", comboRain2);  cmbRain2->addItem("");

	GetMaterials("weather.particle", true, "particle_system");
	for (size_t i=0; i < vsMaterials.size(); ++i)
	{	const String& s = vsMaterials[i];
		cmbRain1->addItem(s);  cmbRain2->addItem(s);
	}	


	//---------------------  Terrain  ---------------------
	Cmb(cmbTexDiff, "TexDiffuse", comboTexDiff);
	Cmb(cmbTexNorm, "TexNormal", comboTexNorm);  cmbTexNorm->addItem("flat_n.png");

	strlist li;
	GetFolderIndex(PATHMANAGER::Data() + "/terrain", li);
	GetFolderIndex(PATHMANAGER::Data() + "/terrain2", li);

	for (strlist::iterator i = li.begin(); i != li.end(); ++i)
	if (!StringUtil::match(*i, "*.txt", false))
	{
		if (!StringUtil::match(*i, "*_prv.*", false))
		if (StringUtil::match(*i, "*_nh.*", false))
			cmbTexNorm->addItem(*i);
		else
			cmbTexDiff->addItem(*i);
	}
	
	//  particles
	GetMaterials("tires.particle", true, "particle_system");
	for (size_t i=0; i < vsMaterials.size(); ++i)
	{	const String& s = vsMaterials[i];
		cmbParDust->addItem(s);  cmbParMud->addItem(s);  cmbParSmoke->addItem(s);
	}
	
	//  surfaces
	for (size_t i=0; i < surfaces.size(); ++i)
		cmbSurface->addItem(surfaces[i].name);
	

	//---------------------  Grass  ---------------------
	GetMaterialsMat(sMat+"grass.mat");
	for (size_t i=0; i < vsMaterials.size(); ++i)
	{	String s = vsMaterials[i];
		if (s.length() > 5)  //!= "grass")
			cmbGrassMtr->addItem(s);
	}
	GetFolderIndex(PATHMANAGER::Data() + "/grass", li);
	for (strlist::iterator i = li.begin(); i != li.end(); ++i)
	{
		if (StringUtil::startsWith(*i, "grClr", false))
			cmbGrassClr->addItem(*i);
	}

	//---------------------  Trees  ---------------------
	Cmb(cmbPgLay, "LTrCombo", comboPgLay);
	strlist lt;
	GetFolderIndex(PATHMANAGER::Data() + "/trees", lt);
	GetFolderIndex(PATHMANAGER::Data() + "/trees2", lt);
	GetFolderIndex(PATHMANAGER::Data() + "/trees-old", lt);
	for (strlist::iterator i = lt.begin(); i != lt.end(); ++i)
		if (StringUtil::endsWith(*i,".mesh"))  {
			std::string s = *i;  s = s.substr(0, s.length()-5);
			cmbPgLay->addItem(s);  }


	//---------------------  Roads  ---------------------
	GetMaterialsMat(sMat+"road.mat");
	GetMaterialsMat(sMat+"road_wall_pipe.mat",false);
	GetMaterialsMat(sMat+"pipe.mat",false);
	for (size_t i=0; i<4; ++i)
	{
		Cmb(cmbRoadMtr[i], "RdMtr"+toStr(i+1), comboRoadMtr);
		Cmb(cmbPipeMtr[i], "RdMtrP"+toStr(i+1), comboPipeMtr);
		if (i>0)  {  cmbRoadMtr[i]->addItem("");  cmbPipeMtr[i]->addItem("");  }
	}
	Cmb(cmbRoadWMtr, "RdMtrW1", comboRoadWMtr);
	Cmb(cmbPipeWMtr, "RdMtrPW1", comboPipeWMtr);
	Cmb(cmbRoadColMtr, "RdMtrC1", comboRoadColMtr);

	for (size_t i=0; i < vsMaterials.size(); ++i)
	{	String s = vsMaterials[i];
		if (StringUtil::startsWith(s,"road") && !StringUtil::startsWith(s,"road_") && !StringUtil::endsWith(s,"_ter") && s != "road")
			for (int i=0; i<4; ++i)  cmbRoadMtr[i]->addItem(s);
		if (StringUtil::startsWith(s,"pipe") && !StringUtil::startsWith(s,"pipe_"))
			for (int i=0; i<4; ++i)  cmbPipeMtr[i]->addItem(s);
		if (StringUtil::startsWith(s,"road_wall"))  cmbRoadWMtr->addItem(s);
		if (StringUtil::startsWith(s,"pipe_wall"))  cmbPipeWMtr->addItem(s);
		if (StringUtil::startsWith(s,"road_col"))  cmbRoadColMtr->addItem(s);
	}


	//---------------------  Objects  ---------------------
	strlist lo;  vObjNames.clear();
	GetFolderIndex(PATHMANAGER::Data() + "/objects", lo);
	for (strlist::iterator i = lo.begin(); i != lo.end(); ++i)
		if (StringUtil::endsWith(*i,".mesh") && (*i) != "sphere.mesh")
			vObjNames.push_back((*i).substr(0,(*i).length()-5));  //no .ext
	
	objListSt = mGUI->findWidget<List>("ObjListSt");
	objListDyn = mGUI->findWidget<List>("ObjListDyn");
	objListBld = mGUI->findWidget<List>("ObjListBld");
	if (objListSt && objListDyn && objListDyn)
	{
		for (int i=0; i < vObjNames.size(); ++i)
		{	const std::string& name = vObjNames[i];
			if (name != "sphere")
			{
				if (StringUtil::startsWith(name,"pers_",false))
					objListBld->addItem("#E0E070"+name);  // buildings
				else
				if (boost::filesystem::exists(PATHMANAGER::Data()+"/objects/"+ name + ".bullet"))
					objListDyn->addItem("#80D0FF"+name);  // dynamic
				else
					objListSt->addItem("#C8C8C8"+name);
		}	}
		//objList->setIndexSelected(0);  //objList->findItemIndexWith(modeSel)
		objListSt->eventListChangePosition += newDelegate(this, &App::listObjsChng);
		objListDyn->eventListChangePosition += newDelegate(this, &App::listObjsChng);
		objListBld->eventListChangePosition += newDelegate(this, &App::listObjsChng);
	}

	
	//---------------------  Tweak  ---------------------
	ComboBoxPtr cmbTwk;
	Cmb(cmbTwk, "TweakMtr", comboTweakMtr);

	GetMaterialsMat(sMat+"water.mat");
	GetMaterialsMat(sMat+"pipe.mat",false);
	GetMaterialsMat(sMat+"road.mat",false);
	GetMaterialsMat(sMat+"objects_static.mat",false);

	for (size_t i=0; i < vsMaterials.size(); ++i)
	{	String s = vsMaterials[i];
			cmbTwk->addItem(s);
	}
	cmbTwk->setIndexSelected( cmbTwk->findItemIndexWith(pSet->tweak_mtr) );
	//-----------------------------------------------------

	InitGuiScreenRes();
	

	///  [Track]
	//------------------------------------------------------------------------
	sListTrack = pSet->gui.track;  //! set last
	bListTrackU = pSet->gui.track_user;
	sCopyTrack = "";  //! none
	bCopyTrackU = 0;
	
	//  text desc
	Edt(trkDesc[0], "TrackDesc", editTrkDesc);
	trkName = mGUI->findWidget<Edit>("TrackName");
	if (trkName)  trkName->setCaption(pSet->gui.track);

	GuiInitTrack();
	
	//  btn change,  new, rename, delete
	//Btn("ChangeTrack",	btnChgTrack);
	Btn("TrackNew",		btnTrackNew);
	Btn("TrackRename",	btnTrackRename);
	Btn("TrackDelete",	btnTrackDel);
	
    //  load = new game
    for (int i=1; i<=2; ++i)
    {	Btn("NewGame"+toStr(i), btnNewGame);  }

	CreateGUITweakMtr();
	

	///  3d view []  (veget models, objects)
	//--------------------------------------------
	//rndCanvas = mGUI->findWidget<Canvas>("CanVeget");  //?
	viewCanvas = mWndEdit->createWidget<Canvas>("Canvas", GetViewSize(), Align::Stretch);
	viewCanvas->setInheritsAlpha(false);
	viewCanvas->setPointer("hand");
	viewCanvas->setVisible(false);
	viewBox.setCanvas(viewCanvas);
	viewBox.setBackgroundColour(Colour(0.32,0.35,0.37,0.7));
	viewBox.setAutoRotation(true);
	viewBox.setMouseRotation(true);
	

	bGI = true;  // gui inited, gui events can now save vals

	ti.update();  /// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Init Gui: ") + toStr(dt) + " ms");
}


IntCoord App::GetViewSize()
{
	IntCoord ic = mWndEdit->getClientCoord();
	return IntCoord(ic.width*0.62f, ic.height*0.45f, ic.width*0.34f, ic.height*0.45f);
}
