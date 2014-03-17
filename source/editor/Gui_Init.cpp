#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/GuiCom.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../vdrift/pathmanager.h"
#include "../ogre/common/MultiList2.h"
#include "../ogre/common/Slider.h"
#include <boost/filesystem.hpp>
#include "../sdl4ogre/sdlcursormanager.hpp"
#include <MyGUI.h>
#include <MyGUI_InputManager.h>
#include <OgreTimer.h>
#include <OgreRenderWindow.h>
#include "../ogre/common/RenderBoxScene.h"
using namespace MyGUI;
using namespace Ogre;


///  Gui Init
//----------------------------------------------------------------------------------------------------------------------

void CGui::InitGui() 
{
	mGui = app->mGui;
	gcom->mGui = mGui;
	SliderValue::pGUI = mGui;
	SliderValue::bGI = &bGI;
	Check::pGUI = mGui;
	Check::bGI = &bGI;

	if (!mGui)  return;
	Ogre::Timer ti;


	//  new widgets
	FactoryManager::getInstance().registerFactory<MultiList2>("Widget");
	FactoryManager::getInstance().registerFactory<Slider>("Widget");

	//  load
	app->vwGui = LayoutManager::getInstance().loadLayout("Editor.layout");


	//  wnds
	app->mWndMain = fWnd("MainMenuWnd");
	app->mWndTrack = fWnd("TrackWnd");
	app->mWndEdit = fWnd("EditorWnd");
	app->mWndOpts = fWnd("OptionsWnd");
	app->mWndHelp = fWnd("HelpWnd");

	app->mWndCam =   fWnd("CamWnd");    app->mWndCam->setPosition(0,64);
	app->mWndStart = fWnd("StartWnd");  app->mWndStart->setPosition(0,64);
	app->mWndBrush = fWnd("BrushWnd");  app->mWndBrush->setPosition(0,64);

	app->mWndRoadCur =   fWnd("RoadCur");    app->mWndRoadCur->setPosition(0,40);
	app->mWndRoadStats = fWnd("RoadStats");  app->mWndRoadStats->setPosition(0,338);

	app->mWndFluids = fWnd("FluidsWnd");   app->mWndFluids->setPosition(0,64);
	app->mWndObjects= fWnd("ObjectsWnd");  app->mWndObjects->setPosition(0,64);
	app->mWndRivers = fWnd("RiversWnd");   app->mWndRivers->setPosition(0,64);


	//  for find defines
	Btn btn, bchk;
	Sl* sl;  SV* sv;  Ck* ck;

	
	//  Tabs
	TabPtr tab,sub;
	fTabW("TabWndTrack"); app->mWndTabsTrack = tab;
	fTabW("TabWndEdit");  app->mWndTabsEdit = tab;
	fTabW("TabWndOpts");  app->mWndTabsOpts = tab;
	fTabW("TabWndHelp");  app->mWndTabsHelp = tab;

	//  get sub tabs
	vSubTabsTrack.clear();
	for (size_t i=0; i < app->mWndTabsTrack->getItemCount(); ++i)
	{
		sub = (TabPtr)app->mWndTabsTrack->getItemAt(i)->findWidget("SubTab");
		vSubTabsTrack.push_back(sub);
	}
	vSubTabsEdit.clear();
	for (size_t i=0; i < app->mWndTabsEdit->getItemCount(); ++i)
	{
		sub = (TabPtr)app->mWndTabsEdit->getItemAt(i)->findWidget("SubTab");
		vSubTabsEdit.push_back(sub);  // 0 for not found
	}
	vSubTabsHelp.clear();
	for (size_t i=0; i < app->mWndTabsHelp->getItemCount(); ++i)
	{
		sub = (TabPtr)app->mWndTabsHelp->getItemAt(i)->findWidget("SubTab");
		vSubTabsHelp.push_back(sub);
	}
	vSubTabsOpts.clear();
	for (size_t i=0; i < app->mWndTabsOpts->getItemCount(); ++i)
	{
		sub = (TabPtr)app->mWndTabsOpts->getItemAt(i)->findWidget("SubTab");
		vSubTabsOpts.push_back(sub);
	}

	///  Gui common init  ---
	gcom->InitMainMenu();
	gcom->GuiInitTooltip();
	gcom->GuiInitLang();

	gcom->GuiInitGraphics();
	gcom->InitGuiScreenRes();


	//app->mWndTabs->setIndexSelected(3);  //default*--
	gcom->ResizeOptWnd(); //?


	//  center mouse pos
	app->mCursorManager->cursorVisibilityChange(app->bGuiFocus || !app->bMoveCam);
	gcom->GuiCenterMouse();
	
	//  hide  ---
	app->SetEdMode(ED_Deform);
	app->UpdEditWnds();  // UpdVisHit(); //after track
	

	#if 0  ///0 _tool_ fix video capture cursor
	imgCur = mGui->createWidget<ImageBox>("ImageBox", 100,100, 32,32, Align::Default, "Pointer");
	imgCur->setImageTexture("pointer.png");
	imgCur->setVisible(true);
	#endif


	//  tool window texts  ----------------------
	int i;
	for (i=0; i<MAX_TXT; ++i)
	{	String s = toStr(i);
		if (i<BR_TXT){  brTxt[i] = fTxt("brTxt"+s);  brVal[i] = fTxt("brVal"+s);  brKey[i] = fTxt("brKey"+s);  }
		if (i<RD_TXT){  rdTxt[i] = fTxt("rdTxt"+s);  rdVal[i] = fTxt("rdVal"+s);  rdKey[i] = fTxt("rdKey"+s);  }
		if (i<RDS_TXT){ rdTxtSt[i] = fTxt("rdTxtSt"+s);  rdValSt[i] = fTxt("rdValSt"+s);  }
		if (i<ST_TXT)   stTxt[i] = fTxt("stTxt"+s);    if (i<FL_TXT)  flTxt[i] = fTxt("flTxt"+s);
		if (i<OBJ_TXT)  objTxt[i]= fTxt("objTxt"+s);   if (i<RI_TXT)  riTxt[i] = fTxt("riTxt"+s);
	}


	///  brush presets   o o o o o o o o 
	ScrollView* scv = mGui->findWidget<ScrollView>("svBrushes");
	int j=0, n=0;  const int z = 128;
	for (i=0; i < app->brSetsNum; ++i,++n)
	{
		const App::BrushSet& st = app->brSets[i];  const String s = toStr(i);
		int x,y, xt,yt, sx, row1 = i-14;  // y,x for next lines
		if (row1 < 0)  // top row
		{	x = 10+ i*50;  y = 10;
			xt = x + 20;  yt = y + 50;  sx = 48;
		}else
		{	if (st.newLine==1 && n > 0 || n > 9) {  n=0;  ++j;  }  // 1 new line
			x = 20+ n*70;  y = 10+ j*70;
			xt = x + 25;  yt = y + 55;  sx = 64;
			if (st.newLine < 0)  n -= st.newLine;  // -1 empty x
		}
		Img img = scv->createWidget<ImageBox>("ImageBox", x,y, sx,sx, Align::Default, "brI"+s);
		img->eventMouseButtonClick += newDelegate(this, &CGui::btnBrushPreset);
		img->setUserString("tip", st.name);  img->setNeedToolTip(true);
		img->setImageTexture("brushes.png");
		img->setImageCoord(IntCoord(i%16*z,i/16*z, z,z));
		if (!st.name.empty())  img->eventToolTip += newDelegate(gcom, &CGuiCom::notifyToolTip);
		gcom->setOrigPos(img, "EditorWnd");
		
		Txt txt = scv->createWidget<TextBox>("TextBox", xt,yt, 40,22, Align::Default, "brT"+s);
		txt->setCaption(fToStr(st.Size,0,2));
			int edMode = st.edMode;
			float fB = app->brClr[edMode][0], fG = app->brClr[edMode][1], fR = app->brClr[edMode][2];
			float m = st.Size / 160.f + 0.4f;
			#define mul(v,m)  std::min(1.f, std::max(0.f, v * m))
		txt->setTextColour(Colour(mul(fB,m), mul(fG,m), mul(fR,m)) );
		gcom->setOrigPos(txt, "EditorWnd");
	}
	//scv->setCanvasSize(1020,j*90+300);


	///  [Settings]
	//------------------------------------------------------------------------
	sv= &svCamSpeed;	sv->Init("CamSpeed",	&pSet->cam_speed, 0.1f,4.f);  sv->DefaultF(0.9f);
	sv= &svCamInert;	sv->Init("CamInert",	&pSet->cam_inert, 0.f, 1.f);  sv->DefaultF(0.4f);

	ck= &ckMinimap;		ck->Init("Minimap",		&pSet->trackmap);  Cev(Minimap);
	sv= &svSizeMinimap;	sv->Init("SizeMinimap",	&pSet->size_minimap, 0.15f,2.f);  sv->DefaultF(0.55f);  Sev(SizeMinimap);

	sv= &svSizeRoadP;	sv->Init("SizeRoadP",	&pSet->road_sphr, 0.1f,12.f); sv->DefaultF(1.5f);  Sev(SizeRoadP);

	sv= &svTerUpd;		sv->Init("TerUpd",		&pSet->ter_skip,  0, 20);  sv->DefaultI(1);
	sv= &svMiniUpd;		sv->Init("MiniUpd",		&pSet->mini_skip, 0, 20);  sv->DefaultI(4);
	ck= &ckAutoBlendmap;ck->Init("AutoBlendmap",&pSet->autoBlendmap);

	ck= &ckWireframe;	ck->Init("Wireframe",	&app->mbWireFrame);  Cev(Wireframe);
	ck= &ckCamPos;		ck->Init("CamPos",		&pSet->camPos);    Cev(CamPos);
	ck= &ckInputBar;	ck->Init("InputBar",	&pSet->inputBar);  Cev(InputBar);
	ck= &ckAllowSave;	ck->Init("AllowSave",	&pSet->allow_save);

	//  set camera btns
	Btn("CamView1", btnSetCam);  Btn("CamView2", btnSetCam);
	Btn("CamView3", btnSetCam);  Btn("CamView4", btnSetCam);
	Btn("CamTop",   btnSetCam);
	Btn("CamLeft",  btnSetCam);  Btn("CamRight", btnSetCam);
	Btn("CamFront", btnSetCam);  Btn("CamBack",  btnSetCam);

	//  startup
	ck= &ckStartInMain;	ck->Init("StartInMain", &pSet->startInMain);
	ck= &ckAutoStart;	ck->Init("AutoStart",   &pSet->autostart);
	ck= &ckEscQuits;	ck->Init("EscQuits",    &pSet->escquit);
	ck= &ckOgreDialog;	ck->Init("OgreDialog",  &pSet->ogre_dialog);
	ck= &ckMouseCapture;ck->Init("MouseCapture",&pSet->mouse_capture);
	

	///  [Sun]
	//----------------------------------------------------------------------------------------------
	sv= &svSunPitch;	sv->Init("SunPitch",	&sc->ldPitch,    0.f,90.f,  1.f, 1,4);  sv->DefaultF(54.f);    Sev(UpdSun);
	sv= &svSunYaw;		sv->Init("SunYaw",		&sc->ldYaw,   -180.f,180.f, 1.f, 1,4);  sv->DefaultF(-123.f);  Sev(UpdSun);
	sv= &svRain1Rate;	sv->Init("Rain1Rate",	&sc->rainEmit,   0.f,6000.f);  sv->DefaultF(1000.f);
	sv= &svRain2Rate;	sv->Init("Rain2Rate",	&sc->rain2Emit,  0.f,6000.f);  sv->DefaultF(1000.f);
	//  fog
	sv= &svFogStart;	sv->Init("FogStart",	&sc->fogStart,   0.f,2000.f, 2.f, 0,3);  sv->DefaultF(100.f);  Sev(UpdFog);
	sv= &svFogEnd;		sv->Init("FogEnd",		&sc->fogEnd,     0.f,2000.f, 2.f, 0,3);  sv->DefaultF(600.f);  Sev(UpdFog);
	sv= &svFogHStart;	sv->Init("FogHStart",	&sc->fogHStart,  0.f,2000.f, 2.f, 0,3);  sv->DefaultF(0.f);    Sev(UpdFog);
	sv= &svFogHEnd;		sv->Init("FogHEnd",		&sc->fogHEnd,    0.f,2000.f, 2.f, 0,3);  sv->DefaultF(60.f);   Sev(UpdFog);
	sv= &svFogHeight;	sv->Init("FogHeight",	&sc->fogHeight, -200.f,200.f, 1.f, 1,4);  sv->DefaultF(-300.f);  Sev(UpdFog);
	sv= &svFogHDensity;	sv->Init("FogHDensity",	&sc->fogHDensity,  0.f,200.f, 2.f, 1,4);  sv->DefaultF(60.f);  Sev(UpdFog);

	ck= &ckFog;			ck->Init("FogDisable",		&pSet->bFog);  Cev(Fog);
	ck= &ckWeather;		ck->Init("WeatherDisable",	&pSet->bWeather);

	//  light
	Ed(LiAmb, editLiAmb);  Ed(LiDiff, editLiDiff);  Ed(LiSpec, editLiSpec);
	Ed(FogClr, editFogClr);  Ed(FogClr2, editFogClr2);  Ed(FogClrH, editFogClrH);

	clrAmb = fImg("ClrAmb");   clrDiff = fImg("ClrDiff");
	clrSpec= fImg("ClrSpec");  clrTrail= fImg("ClrTrail");
	clrFog = fImg("ClrFog");   clrFog2 = fImg("ClrFog2");
	clrFogH= fImg("ClrFogH");  //Todo: on click event - open color dialog


	///  [Terrain]
	//------------------------------------------------------------------------
	imgTexDiff = fImg("TerImgDiff");
	Tab(tabsHmap, "TabHMapSize", tabHmap);
	sv= &svTerErrorNorm;  sv->Init("TerErrorNorm", &sc->td.errorNorm,  1.5f,15.f, 1.5f,1,3);  sv->DefaultF(3.f);  Sev(TerErrorNorm);
	sv= &svTerNormScale;  sv->Init("TerNormScale", &sc->td.normScale,  0.f,2.f,   1.f, 1,3);  sv->DefaultF(1.f);  Sev(TerNormScale);

	Btn("TerrainNew", btnTerrainNew);
	Btn("TerrainGenAdd", btnTerGenerate);  Btn("TerrainGenSub", btnTerGenerate);    Btn("TerrainGenMul", btnTerGenerate);
	Btn("TerrainHalf",   btnTerrainHalf);  Btn("TerrainDouble", btnTerrainDouble);  Btn("TerrainMove",   btnTerrainMove);
	///TODO: show window with list, chks, sceneries,  ...
	//Btn("PickTex", btnPickTex);
	//Btn("PickGrass", btnPickGrass);
	//Btn("PickVeget", btnPickVeget);


	///  generator  . . . . . . .
	sv= &svTerGenScale;	sv->Init("TerGenScale",	&pSet->gen_scale, 0.f,160.f, 2.f, 2,4);  sv->DefaultF(52.f);
	sv= &svTerGenOfsX;	sv->Init("TerGenOfsX",	&pSet->gen_ofsx, -12.f,12.f, 1.f, 3,5);  sv->DefaultF(0.14f);   Sev(TerGen);
	sv= &svTerGenOfsY;	sv->Init("TerGenOfsY",	&pSet->gen_ofsy, -12.f,12.f, 1.f, 3,5);  sv->DefaultF(-1.54f);  Sev(TerGen);

	sv= &svTerGenFreq;	sv->Init("TerGenFreq",	&pSet->gen_freq,   0.06f,3.f, 2.f, 3,5);  sv->DefaultF(0.914f); Sev(TerGen);
	sv= &svTerGenOct;	sv->Init("TerGenOct",	&pSet->gen_oct,    0, 9);                 sv->DefaultI(4);      Sev(TerGen);
	sv= &svTerGenPers;	sv->Init("TerGenPers",	&pSet->gen_persist,0.f, 0.7f, 1.f, 3,5);  sv->DefaultF(0.347f); Sev(TerGen);
	sv= &svTerGenPow;	sv->Init("TerGenPow",	&pSet->gen_pow,    0.f, 6.f,  2.f, 2,4);  sv->DefaultF(1.f);    Sev(TerGen);
	
	sv= &svTerGenMul;	sv->Init("TerGenMul",	&pSet->gen_mul,    0.f, 6.f,  2.f, 2,4);  sv->DefaultF(1.f);
	sv= &svTerGenOfsH;	sv->Init("TerGenOfsH",	&pSet->gen_ofsh,   0.f, 60.f, 2.f, 2,4);  sv->DefaultF(0.f);
	sv= &svTerGenRoadSm;sv->Init("TerGenRoadSm",&pSet->gen_roadsm, 0.f, 6.f,  1.f, 2,4);  sv->DefaultF(0.f);

	sv= &svTerGenAngMin;sv->Init("TerGenAngMin",&pSet->gen_terMinA, 0.f,  90.f,  1.f, 1,4);  sv->DefaultF(0.f);
	sv= &svTerGenAngMax;sv->Init("TerGenAngMax",&pSet->gen_terMaxA, 0.f,  90.f,  1.f, 1,4);  sv->DefaultF(90.f);
	sv= &svTerGenAngSm;	sv->Init("TerGenAngSm",	&pSet->gen_terSmA,  0.f,  90.f,  2.f, 1,4);  sv->DefaultF(10.f);
	sv= &svTerGenHMin;	sv->Init("TerGenHMin",	&pSet->gen_terMinH,-150.f,150.f, 1.f, 0,1);  sv->DefaultF(-300.f);
	sv= &svTerGenHMax;	sv->Init("TerGenHMax",	&pSet->gen_terMaxH,-150.f,150.f, 1.f, 0,1);  sv->DefaultF( 300.f);
	sv= &svTerGenHSm;	sv->Init("TerGenHSm",	&pSet->gen_terSmH,  0.f,  100.f, 2.f, 1,4);  sv->DefaultF(20.f);


	///  [Layers]  ------------------------------------
	bool b;
	ck= &ckTerLayOn;	ck->Init("TerLayOn",	&b);   Cev(TerLayOn);
	valTerLAll = fTxt("TerLayersAll");
	valTriplAll = fTxt("TerTriplAll");
	Tab(tabsTerLayers, "TabTerLay", tabTerLayer);

	ck= &ckTexNormAuto;	ck->Init("TexNormAuto",	&bTexNormAuto);
	ck= &ckTerLayTripl;	ck->Init("TerLayTripl",	&b);   Cev(TerLayTripl);
	ck= &ckDebugBlend;	ck->Init("DebugBlend",  &bDebugBlend);  Cev(DebugBlend);
	dbgLclr = fImg("dbgTerLclr");

	float f=0.f;  i=0;  // temp vars
	//  ter layer
	sv= &svTerTriSize;	sv->Init("TerTriSize", &sc->td.fTriangleSize,  0.1f,6.f, 2.f);  sv->DefaultF(1.4f);  Sev(TerTriSize);
	sv= &svTerLScale;	sv->Init("TerLScale",  &f, 6.0f, 72.f,  2.f);  sv->DefaultF(8.f);  //Sev(TerLay);
	//  blendmap
	sv= &svTerLAngMin;  sv->Init("TerLAngMin", &f, 0.f,  90.f,  1.f, 1,4);  sv->DefaultF(0.f);  Sev(TerLay);
	sv= &svTerLAngMax;  sv->Init("TerLAngMax", &f, 0.f,  90.f,  1.f, 1,4);  sv->DefaultF(90.f);  Sev(TerLay);
	sv= &svTerLAngSm;   sv->Init("TerLAngSm",  &f, 0.f,  90.f,  2.f, 1,4);  sv->DefaultF(20.f);  Sev(TerLay);

	sv= &svTerLHMin;    sv->Init("TerLHMin",   &f,-150.f,150.f, 1.f, 0,2);  sv->DefaultF(-300.f);  Sev(TerLay);
	sv= &svTerLHMax;    sv->Init("TerLHMax",   &f,-150.f,150.f, 1.f, 0,2);  sv->DefaultF( 300.f);  Sev(TerLay);
	sv= &svTerLHSm;     sv->Init("TerLHSm",    &f, 0.f,  100.f, 2.f, 1,4);  sv->DefaultF(20.f);  Sev(TerLay);
	Btn("TerLmoveL", btnTerLmoveL);
	Btn("TerLmoveR", btnTerLmoveR);

	//  noise
	ck= &ckTerLNOnly;   ck->Init("TerLNonly",  &b);   Cev(TerLNOnly);
	sv= &svTerLNoise;   sv->Init("TerLNoise",  &f, 0.f,1.f);  sv->DefaultF(0.f);  Sev(TerLay);
	sv= &svTerLNprev;   sv->Init("TerLNprev",  &f, 0.f,1.f);  sv->DefaultF(0.f);  Sev(TerLay);
	sv= &svTerLNnext2;  sv->Init("TerLNnext2", &f, 0.f,1.f);  sv->DefaultF(0.f);  Sev(TerLay);
	//  noise params
	for (i=0; i<2; ++i)  {  String s = toStr(i+1);
	sv= &svTerLN_Freq[i];  sv->Init("TerLNFreq"+s,  &f, 1.f,300.f, 2.f, 1,3);   sv->DefaultF(30.f);  Sev(TerLay);
	sv= &svTerLN_Oct[i];   sv->Init("TerLNOct" +s,  &i, 1,5);                   sv->DefaultI(3);     Sev(TerLay);
	sv= &svTerLN_Pers[i];  sv->Init("TerLNPers"+s,  &f, 0.1f, 0.7f, 1.f, 3,5);  sv->DefaultF(0.3f);  Sev(TerLay);
	sv= &svTerLN_Pow[i];   sv->Init("TerLNPow" +s,  &f, 0.2f, 8.f,  2.f);       sv->DefaultF(1.f);   Sev(TerLay);  }
	//  noise btns
	Btn("TerLNbtn1", radN1);  bRn1 = btn;  bRn1->setStateSelected(true);
	Btn("TerLNbtn2", radN2);  bRn2 = btn;
	for (i=0; i < 15; ++i)
	{  Btn("TerLN_"+toStr(i), btnNpreset);  }
	Btn("TerLNrandom", btnNrandom);
	Btn("TerLNswap", btnNswap);
	
	//  particles
	Ed(LDust, editLDust);	Ed(LDustS, editLDust);
	Ed(LMud,  editLDust);	Ed(LSmoke, editLDust);
	Ed(LTrlClr, editLTrlClr);
	Cmb(cmbParDust, "CmbParDust", comboParDust);
	Cmb(cmbParMud,  "CmbParMud",  comboParDust);
	Cmb(cmbParSmoke,"CmbParSmoke",comboParDust);

	//  surface
	Cmb(cmbSurface, "Surface", comboSurface);  //1 txt-
	txtSuBumpWave = fTxt("SuBumpWave");   txtSuFrict  = fTxt("SuFrict");
	txtSuBumpAmp  = fTxt("SuBumpAmp");	  txtSurfTire = fTxt("SurfTire");
	txtSuRollDrag = fTxt("SuRollDrag");	  txtSurfType = fTxt("SurfType");
	SldUpd_TerL();

	
	///  [Vegetation]  ------------------------------------
	sv= &svGrassDens;	sv->Init("GrassDens",	&sc->densGrass, 0.f, 1.f, 2.f);  sv->DefaultF(0.2f);
	sv= &svTreesDens;	sv->Init("TreesDens",	&sc->densTrees, 0.f, 3.f, 2.f);  sv->DefaultF(0.3f);

	Ed(GrPage, editTrGr);  Ed(GrDist, editTrGr);
	Ed(TrPage, editTrGr);  Ed(TrDist, editTrGr);  Ed(TrImpDist, editTrGr);
	
	sv= &svTrRdDist;	sv->Init("TrRdDist",	&sc->trRdDist,     0,6);   sv->DefaultI(1);
	sv= &svGrDensSmooth;sv->Init("GrDensSmooth",&sc->grDensSmooth, 0,10);  sv->DefaultI(3);
	Ed(SceneryId, editTrGr);

	//  veget models
	ck= &ckPgLayOn;		ck->Init("LTrEnabled",	&b);   Cev(PgLayOn);
	valLTrAll = fTxt("LTrAll");
	Tab(tabsPgLayers, "LTrNumTab", tabPgLayers);

	sv= &svLTrDens;		sv->Init("LTrDens",		 &f, 0.f, 1.f, 2.f, 3,5);  sv->DefaultF(0.15f);
	
	sv= &svLTrRdDist;	sv->Init("LTrRdDist",	 &i, 0,20);  sv->DefaultI(0);
	sv= &svLTrRdDistMax;sv->Init("LTrRdDistMax", &i, 0,20);  sv->DefaultI(20);
	
	sv= &svLTrMinSc;	sv->Init("LTrMinSc",	 &f, 0.f,4.f, 3.f, 3,5);  sv->DefaultF(0.7f);
	sv= &svLTrMaxSc;	sv->Init("LTrMaxSc",	 &f, 0.f,4.f, 3.f, 3,5);  sv->DefaultF(1.2f);
	
	sv= &svLTrWindFx;	sv->Init("LTrWindFx",	 &f, 0.f,12.f, 3.f, 3,5);  sv->DefaultF(0.5f);
	sv= &svLTrWindFy;	sv->Init("LTrWindFy",	 &f, 0.f,0.4f, 3.f, 3,5);  sv->DefaultF(0.06f);
	
	sv= &svLTrMaxTerAng;sv->Init("LTrMaxTerAng", &f, 0.f,90.f, 2.f, 1,4);  sv->DefaultF(30.f);

	sv= &svLTrMinTerH;	sv->Init("LTrMinTerH",	 &f,-60.f,60.f, 1.f, 1,4);  sv->DefaultF(-100.f);
	sv= &svLTrMaxTerH;	sv->Init("LTrMaxTerH",	 &f, 0.f,120.f, 1.f, 1,4);  sv->DefaultF( 100.f);
	sv= &svLTrFlDepth;	sv->Init("LTrFlDepth",	 &f, 0.f,5.f, 2.f, 1,4);  sv->DefaultF(0.f);
	SldUpd_PgL();  // real &f set here


	///  Grass  ------------------------------------
	Ed(GrSwayDistr, editTrGr);  Ed(GrSwayLen, editTrGr);  Ed(GrSwaySpd, editTrGr);

	Cmb(cmbGrassMtr, "CmbGrMtr", comboGrassMtr);	imgGrass = fImg("ImgGrass");
	Cmb(cmbGrassClr, "CmbGrClr", comboGrassClr);	imgGrClr = fImg("ImgGrClr");

	//  grass channels
	sv= &svGrChAngMin;	sv->Init("GrChMinA",	&f, 0.f,90.f, 1.f, 1,4);  sv->DefaultF(30.f);
	sv= &svGrChAngMax;	sv->Init("GrChMaxA",	&f, 0.f,90.f, 1.f, 1,4);  sv->DefaultF(30.f);
	sv= &svGrChAngSm;	sv->Init("GrChSmA",		&f, 0.f,50.f, 2.f, 1,4);  sv->DefaultF(20.f);
									  
	sv= &svGrChHMin;	sv->Init("GrChMinH",	&f,-60.f,60.f,  1.f, 1,4);  sv->DefaultF(-200.f);
	sv= &svGrChHMax;	sv->Init("GrChMaxH",	&f,  0.f,120.f, 1.f, 1,4);  sv->DefaultF( 200.f);
	sv= &svGrChHSm;		sv->Init("GrChSmH",		&f,  0.f,60.f,  2.f, 1,4);  sv->DefaultF(20.f);
	sv= &svGrChRdPow;	sv->Init("GrChRdPow",	&f, -8.f, 8.f,  1.f, 1,4);  sv->DefaultF(0.f);
	//  noise
	sv= &svGrChNoise;	sv->Init("GrChNoise",	&f, 0.f,2.f,   1.f, 1,4);   sv->DefaultF(0.f);
	sv= &svGrChNfreq;	sv->Init("GrChNFreq",	&f, 1.f,300.f, 2.f, 1,3);   sv->DefaultF(30.f);
	sv= &svGrChNoct;	sv->Init("GrChNOct",	&i, 1,5);                   sv->DefaultI(3);
	sv= &svGrChNpers;	sv->Init("GrChNPers",	&f, 0.1f, 0.7f, 1.f, 3,5);  sv->DefaultF(0.3f);
	sv= &svGrChNpow;	sv->Init("GrChNPow",	&f, 0.2f, 8.f,  2.f);       sv->DefaultF(1.f);
	Tab(tabsGrChan, "GrChanTab", tabGrChan);
	SldUpd_GrChan();

	//  grass layers
	ck= &ckGrLayOn;		ck->Init("LGrEnabled",	&b);   Cev(GrLayOn);
	valLGrAll = fTxt("LGrAll");
	Tab(tabsGrLayers, "LGrLayTab", tabGrLayers);

	sv= &svGrMinX;	sv->Init("GrMinX",	&f, 0.1f,4.1, 2.f);  sv->DefaultF(1.3f);
	sv= &svGrMaxX;	sv->Init("GrMaxX",	&f, 0.1f,4.1, 2.f);  sv->DefaultF(1.5f);
	sv= &svGrMinY;	sv->Init("GrMinY",	&f, 0.1f,4.1, 2.f);  sv->DefaultF(1.4f);
	sv= &svGrMaxY;	sv->Init("GrMaxY",	&f, 0.1f,4.1, 2.f);  sv->DefaultF(1.6f);
	sv= &svGrChan;	sv->Init("LGrChan",	&i, 0,3);  sv->DefaultI(0);
	sv= &svLGrDens;	sv->Init("LGrDens",	&f, 0.001f,1.f, 2.f, 3,5);  sv->DefaultF(0.22f);
	SldUpd_GrL();

	
	///  [Road]  ------------------------------------
	sv= &svRdTcMul; 	sv->Init("RdTcMul", 	&f, 0.01f,0.3f, 1.5f, 3,5);  sv->DefaultF(0.1f);
	sv= &svRdTcMulW;	sv->Init("RdTcMulW",	&f, 0.01f,0.4f, 1.5f, 3,5);  sv->DefaultF(0.1f);
	sv= &svRdTcMulP;	sv->Init("RdTcMulP",	&f, 0.01f,0.3f, 1.5f, 3,5);  sv->DefaultF(0.2f);
	sv= &svRdTcMulPW;	sv->Init("RdTcMulPW",	&f, 0.01f,0.4f, 1.5f, 3,5);  sv->DefaultF(0.2f);
	sv= &svRdTcMulC;	sv->Init("RdTcMulC",	&f, 0.01f,0.4f, 1.5f, 3,5);  sv->DefaultF(0.2f);

	sv= &svRdLenDim;	sv->Init("RdLenDim",	&f, 0.5f, 4.f, 1.5f, 2,4);  sv->DefaultF(1.f);
	sv= &svRdWidthSteps;sv->Init("RdWidthSteps",&i, 3,16, 1.5f);  sv->DefaultI(6);
	sv= &svRdPlsM;		sv->Init("RdPlsM",		&f, 1.f, 8.f, 1.5f, 1,3);  sv->DefaultF(1.f);
	sv= &svRdPwsM;		sv->Init("RdPwsM",		&f, 1.f, 8.f, 1.5f, 1,3);  sv->DefaultF(4.f);

	sv= &svRdColN;		sv->Init("RdColN",		&i, 3,16, 1.5f);  sv->DefaultI(4);
	sv= &svRdColR;		sv->Init("RdColR",		&f, 1.0f, 6.f, 1.5f, 2,4);  sv->DefaultF(2.f);
	sv= &svRdMergeLen;	sv->Init("RdMergeLen",	&f, 40.f, 2000.f, 2.f, 0,2);  sv->DefaultF(400.f);
	sv= &svRdLodPLen;	sv->Init("RdLodPLen",	&f, 10.f, 160.f, 2.f, 0,2);  sv->DefaultF(20.f);
	SldUpd_Road();
	
	Ed(RdHeightOfs, editRoad);
	Ed(RdSkirtLen, editRoad);  Ed(RdSkirtH, editRoad);
	

	///  [Game]  ------------------------------------
	sv= &svDamage;		sv->Init("DamageMul",	&sc->damageMul, 0.f,2.f, 1.f, 2,4);  sv->DefaultF(1.f);
	sv= &svWind;		sv->Init("WindAmt",		&sc->windAmt,  -6.f,6.f, 1.0f, 2,5);  sv->DefaultF(0.f);
	sv= &svGravity;		sv->Init("Gravity",		&sc->gravity,   2.f,20.f, 1.5f, 2,4);  sv->DefaultF(9.81f);
	ck= &ckDenyReversed;	ck->Init("DenyReversed",	&sc->denyReversed);
	ck= &ckTiresAsphalt;	ck->Init("TiresAsphalt",	&sc->asphalt);
	ck= &ckTerrainEmissive;	ck->Init("TerrainEmissive",	&sc->td.emissive);
	

	///  [Tools]  ------------------------------------
	Btn("TrackCopySel", btnTrkCopySel);
	valTrkCpySel = fTxt("TrkCopySelName");

	Btn("CopySun", btnCopySun);				Btn("CopyTerHmap", btnCopyTerHmap);
	Btn("CopyTerLayers", btnCopyTerLayers);	Btn("CopyVeget", btnCopyVeget);
	Btn("CopyRoad", btnCopyRoad);			Btn("CopyRoadPars", btnCopyRoadPars);

	Btn("DeleteRoad", btnDeleteRoad);		Btn("DeleteFluids", btnDeleteFluids);
	Btn("DeleteObjects", btnDeleteObjects);

	sv= &svScaleAllMul;		sv->Init("ScaleAllMul",		&fScale,    0.5f,2.f, 1.5f);  sv->DefaultF(1.f);
	sv= &svScaleTerHMul;	sv->Init("ScaleTerHMul",	&fScaleTer, 0.5f,2.f, 1.5f);  sv->DefaultF(1.f);
	Btn("ScaleAll",  btnScaleAll);  Btn("ScaleTerH", btnScaleTerH);

	sv= &svAlignWidthAdd;	sv->Init("AlignWidthAdd",	&pSet->al_w_add,  0.f,20.f,1.f, 1,3);  sv->DefaultF(10.f);
	sv= &svAlignWidthMul;	sv->Init("AlignWidthMul",	&pSet->al_w_mul,  1.f,4.f, 1.f, 2,4);  sv->DefaultF(1.f);
	sv= &svAlignSmooth;		sv->Init("AlignSmooth",		&pSet->al_smooth, 0.f,6.f, 1.f, 1,3);  sv->DefaultF(3.f);

	
	///  [Warnings]  ------------------------------------
	edWarn = fEd("Warnings");
	txWarn = mGui->createWidget<TextBox>("TextBox", 300,20, 360,32, Align::Left, "Back");
	txWarn->setTextShadow(true);  txWarn->setTextShadowColour(Colour::Black);
	txWarn->setTextColour(Colour(1.0,0.4,0.2));  txWarn->setFontHeight(24);
	txWarn->setVisible(false);

	imgWarn = fImg("ImgWarn");  imgWarn->setVisible(false);
	imgInfo = fImg("ImgInfo");

	ck= &ckCheckSave;	ck->Init("CheckSave",	&pSet->check_save);
	ck= &ckCheckLoad;	ck->Init("CheckLoad",	&pSet->check_load);
	

	///  Fill Combo boxes  . . . . . . .
	//------------------------------------------------------------------------------------------------------------

	
	//---------------------  Skies  ---------------------
	Cmb(cmbSky, "SkyCombo", comboSky);
	std::string data = PATHMANAGER::Data();
	String sMat = data +"/materials/scene/";  // path

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
	PATHMANAGER::DirList(data + "/terrain", li);
	PATHMANAGER::DirList(data + "/terrain2", li);

	for (strlist::iterator i = li.begin(); i != li.end(); ++i)
	if (!StringUtil::match(*i, "*.txt", false) &&
		!StringUtil::match(*i, "*_prv.*", false))
	{
		String s = *i;
		if (StringUtil::match(*i, "*_n.*", false))
			cmbTexNorm->addItem(*i);
		else
		if (StringUtil::match(*i, "*_d.*", false))  //_T
			cmbTexDiff->addItem(*i);
	}
	
	//  particles
	GetMaterials("tires.particle", true, "particle_system");
	for (size_t i=0; i < vsMaterials.size(); ++i)
	{	const String& s = vsMaterials[i];
		cmbParDust->addItem(s);  cmbParMud->addItem(s);  cmbParSmoke->addItem(s);
	}
	
	//  surfaces
	for (size_t i=0; i < app->surfaces.size(); ++i)
		cmbSurface->addItem(app->surfaces[i].name);
	

	//---------------------  Grass  ---------------------
	GetMaterialsMat(sMat+"grass.mat");
	for (size_t i=0; i < vsMaterials.size(); ++i)
	{	String s = vsMaterials[i];
		if (s.length() > 5)  //!= "grass")
			cmbGrassMtr->addItem(s);
	}
	PATHMANAGER::DirList(data + "/grass", li);
	for (strlist::iterator i = li.begin(); i != li.end(); ++i)
	{
		if (StringUtil::startsWith(*i, "grClr", false))
			cmbGrassClr->addItem(*i);
	}

	//---------------------  Trees  ---------------------
	Cmb(cmbPgLay, "LTrCombo", comboPgLay);
	strlist lt;
	PATHMANAGER::DirList(data + "/trees", lt);
	PATHMANAGER::DirList(data + "/trees2", lt);
	PATHMANAGER::DirList(data + "/trees-old", lt);
	for (strlist::iterator i = lt.begin(); i != lt.end(); ++i)
		if (StringUtil::endsWith(*i,".mesh"))  {
			std::string s = *i;  s = s.substr(0, s.length()-5);
			cmbPgLay->addItem(s);  }


	//---------------------  Roads  ---------------------
	GetMaterialsMat(sMat+"road.mat");
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
			for (int n=0; n<4; ++n)  cmbRoadMtr[n]->addItem(s);
		if (StringUtil::startsWith(s,"pipe") && !StringUtil::startsWith(s,"pipe_"))
			for (int n=0; n<4; ++n)  cmbPipeMtr[n]->addItem(s);
		if (StringUtil::startsWith(s,"road_wall"))  cmbRoadWMtr->addItem(s);
		if (StringUtil::startsWith(s,"pipe_wall"))  cmbPipeWMtr->addItem(s);
		if (StringUtil::startsWith(s,"road_col"))  cmbRoadColMtr->addItem(s);
	}


	//---------------------  Objects  ---------------------
	app->vObjNames.clear();  strlist lo;
	PATHMANAGER::DirList(data + "/objects", lo);
	for (strlist::iterator i = lo.begin(); i != lo.end(); ++i)
		if (StringUtil::endsWith(*i,".mesh") && (*i) != "sphere.mesh")
			app->vObjNames.push_back((*i).substr(0,(*i).length()-5));  //no .ext
	
	objListDyn = fLi("ObjListDyn");  Lev(objListDyn, ObjsChng);
	objListSt  = fLi("ObjListSt");   Lev(objListSt,  ObjsChng);
	objListRck = fLi("ObjListRck");  Lev(objListRck, ObjsChng);
	objListBld = fLi("ObjListBld");  Lev(objListBld, ObjsChng);
	objPan = fWP("objPan");

	for (int i=0; i < app->vObjNames.size(); ++i)
	{	const std::string& name = app->vObjNames[i];
		if (name != "sphere")
		{
			if (StringUtil::startsWith(name,"pers_",false))
				objListBld->addItem("#E0E070"+name);  // buildings
			else
			if (StringUtil::startsWith(name,"rock",false)||StringUtil::startsWith(name,"cave",false))
				objListRck->addItem("#E0B070"+name);  // rocks
			else 
			if (boost::filesystem::exists(data+"/objects/"+ name + ".bullet"))
				objListDyn->addItem("#A0E0FF"+name);  // dynamic
			else
				objListSt->addItem("#C8C8C8"+name);
	}	}
	//objList->setIndexSelected(0);  //objList->findItemIndexWith(modeSel)


	//---------------------  Surfaces  ---------------------
	surfList = fLi("SurfList");  Lev(surfList, Surf);
	for (n=0; n < 4; ++n)  surfList->addItem("#80FF00"+TR("#{Layer} ")+toStr(n));
	for (n=0; n < 4; ++n)  surfList->addItem("#FFB020"+TR("#{Road} ")+toStr(n));
	for (n=0; n < 4; ++n)  surfList->addItem("#FFFF80"+TR("#{Pipe} ")+toStr(n));
	surfList->setIndexSelected(0);
	
	
	//---------------------  Tweak  ---------------------
	ComboBoxPtr cmbTwk;
	Cmb(cmbTwk, "TweakMtr", comboTweakMtr);

	GetMaterialsMat(data +"/materials/water.mat");
	GetMaterialsMat(sMat+"pipe.mat",false);
	GetMaterialsMat(sMat+"road.mat",false);
	GetMaterialsMat(sMat+"objects_static.mat",false);

	cmbTwk->addItem("");
	for (size_t i=0; i < vsMaterials.size(); ++i)
	{	String s = vsMaterials[i];
			cmbTwk->addItem(s);
	}
	cmbTwk->setIndexSelected( cmbTwk->findItemIndexWith(pSet->tweak_mtr) );
	//-----------------------------------------------------
	

	///  [Track]
	//------------------------------------------------------------------------
	gcom->sListTrack = pSet->gui.track;  //! set last
	gcom->bListTrackU = pSet->gui.track_user;
	sCopyTrack = "";  //! none
	bCopyTrackU = 0;
	
	//  text desc
	Edt(gcom->trkDesc[0], "TrackDesc", editTrkDesc);
	trkName = fEd("TrackName");
	if (trkName)  trkName->setCaption(pSet->gui.track);

	gcom->GuiInitTrack();
	
	//  btn  new, rename, delete
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
	viewCanvas = app->mWndEdit->createWidget<Canvas>("Canvas", GetViewSize(), Align::Stretch);
	viewCanvas->setInheritsAlpha(false);
	viewCanvas->setPointer("hand");
	viewCanvas->setVisible(false);
	viewBox->setCanvas(viewCanvas);
	viewBox->setBackgroundColour(Colour(0.32,0.35,0.37,0.7));
	viewBox->setAutoRotation(true);
	viewBox->setMouseRotation(true);
	

	bGI = true;  // gui inited, gui events can now save vals

	LogO(String("::: Time Init Gui: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");
}


IntCoord CGui::GetViewSize()
{
	IntCoord ic = app->mWndEdit->getClientCoord();
	return IntCoord(ic.width*0.63f, ic.height*0.45f, ic.width*0.34f, ic.height*0.45f);
}
