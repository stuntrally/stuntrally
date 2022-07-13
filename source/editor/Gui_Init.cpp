#include "pch.h"
#include "../ogre/common/Def_Str.h"
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/GuiCom.h"
#include "../ogre/common/data/CData.h"
#include "../ogre/common/data/TracksXml.h"
#include "../ogre/common/data/PresetsXml.h"
#include "settings.h"
#include "CApp.h"
#include "CGui.h"
#include "../vdrift/pathmanager.h"
#include "../ogre/common/MultiList2.h"
#include "../ogre/common/Slider.h"
#include <boost/filesystem.hpp>
#include "../sdl4ogre/sdlinputwrapper.hpp"
#include <MyGUI.h>
#include <MyGUI_InputManager.h>
#include <OgreTimer.h>
#include <OgreRenderWindow.h>
#include "../ogre/common/RenderBoxScene.h"
using namespace MyGUI;
using namespace Ogre;
using namespace std;


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

	//  fonts
	gcom->CreateFonts();

	//  new widgets
	FactoryManager::getInstance().registerFactory<MultiList2>("Widget");
	FactoryManager::getInstance().registerFactory<Slider>("Widget");


	///  Load .layout files
	auto Load = [&](string file)
	{
		auto v = LayoutManager::getInstance().loadLayout(file + ".layout");
		app->vwGui.insert(app->vwGui.end(), v.begin(), v.end());
	};
	Load("Common");  Load("Editor_Main");  Load("Editor");  Load("Editor_Track");
	Load("Editor_Utils");  Load("Editor_Tools");  Load("Editor_Help");  Load("Editor_Options");


	//  wnds main
	app->mWndMain = fWnd("MainMenuWnd");
	app->mWndTrack = fWnd("TrackWnd");  app->mWndEdit = fWnd("EditorWnd");
	app->mWndOpts = fWnd("OptionsWnd"); app->mWndHelp = fWnd("HelpWnd");
	
	app->mWndPick = fWnd("PickWnd");
	app->mWndTrkFilt = fWnd("TrackFilterWnd");

	//  wnds ed
	app->mWndCam =   fWnd("CamWnd");    app->mWndCam->setPosition(0,64);
	app->mWndStart = fWnd("StartWnd");  app->mWndStart->setPosition(0,64);
	app->mWndBrush = fWnd("BrushWnd");  app->mWndBrush->setPosition(0,64);

	app->mWndRoadCur =   fWnd("RoadCur");    app->mWndRoadCur->setPosition(0,40);
	app->mWndRoadStats = fWnd("RoadStats");  app->mWndRoadStats->setPosition(0,390);

	app->mWndFluids =    fWnd("FluidsWnd");   app->mWndFluids->setPosition(0,64);
	app->mWndObjects =   fWnd("ObjectsWnd");  app->mWndObjects->setPosition(0,64);
	app->mWndParticles = fWnd("ParticlesWnd"); app->mWndParticles->setPosition(0,64);


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
	size_t u;
	for (u=0; u < app->mWndTabsTrack->getItemCount(); ++u)
	{
		sub = gcom->FindSubTab(app->mWndTabsTrack->getItemAt(u));
		vSubTabsTrack.push_back(sub);
	}
	vSubTabsEdit.clear();
	for (u=0; u < app->mWndTabsEdit->getItemCount(); ++u)
	{
		sub = gcom->FindSubTab(app->mWndTabsEdit->getItemAt(u));
		vSubTabsEdit.push_back(sub);
	}
	vSubTabsHelp.clear();
	for (u=0; u < app->mWndTabsHelp->getItemCount(); ++u)
	{
		sub = gcom->FindSubTab(app->mWndTabsHelp->getItemAt(u));
		vSubTabsHelp.push_back(sub);
	}
	vSubTabsOpts.clear();
	for (u=0; u < app->mWndTabsOpts->getItemCount(); ++u)
	{
		sub = gcom->FindSubTab(app->mWndTabsOpts->getItemAt(u));
		vSubTabsOpts.push_back(sub);
	}

	///  Gui common init  ---
	InitMainMenu();
	gcom->GuiInitTooltip();
	gcom->GuiInitLang();

	gcom->GuiInitGraphics();
	gcom->InitGuiScreenRes();


	//app->mWndTabs->setIndexSelected(3);  //default*--
	gcom->ResizeOptWnd(); //?


	//  center mouse pos
	app->mInputWrapper->setMouseVisible(app->bGuiFocus || !app->bMoveCam);
	gcom->GuiCenterMouse();
	
	//  hide all wnd  ---
	app->SetEdMode(ED_Deform);
	app->UpdEditWnds();
	app->mWndBrush->setVisible(false);
	

	#if 0  ///0 _tool_ fix video capture cursor
	imgCur = mGui->createWidget<ImageBox>("ImageBox", 100,100, 32,32, Align::Default, "Pointer");
	imgCur->setImageTexture("pointer.png");
	imgCur->setVisible(true);
	#endif


	//  tool window texts  ----------------------
	int i;
	for (i=0; i < RD_TXT/*MAX*/; ++i)
	{	String s = toStr(i);
		if (i<BR_TXT){  brTxt[i] = fTxt("brTxt"+s);  brVal[i] = fTxt("brVal"+s);  brKey[i] = fTxt("brKey"+s);  }
		if (i<RD_TXT){  rdTxt[i] = fTxt("rdTxt"+s);  rdVal[i] = fTxt("rdVal"+s);  rdKey[i] = fTxt("rdKey"+s);  rdImg[i] = fImg("rdImg"+s);  }
		if (i<RDS_TXT){ rdTxtSt[i] = fTxt("rdTxtSt"+s);  rdValSt[i] = fTxt("rdValSt"+s);  }

		if (i<ST_TXT)   stTxt[i] = fTxt("stTxt"+s);    if (i<FL_TXT)  flTxt[i] = fTxt("flTxt"+s);
		if (i<OBJ_TXT)  objTxt[i]= fTxt("objTxt"+s);  if (i<EMT_TXT)  emtTxt[i]= fTxt("emtTxt"+s);
	}


	//  mode, status
	imgCam  = fImg("modeCam");  imgEdit = fImg("modeEdit");  imgGui  = fImg("modeGui");
	panStatus = fWP("panStatus");  txtStatus = fTxt("txtStatus");


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
			#define mul(v,m)  min(1.f, max(0.f, v * m))
		txt->setTextColour(Colour(mul(fB,m), mul(fG,m), mul(fR,m)) );
		gcom->setOrigPos(txt, "EditorWnd");
	}
	//scv->setCanvasSize(1020,j*90+300);


	float f=0.f;  i=0;  // temp vars
	
	///  Color tool wnd  ----
	wndColor = fWnd("ColorWnd");											 //0.165
	sv= &svHue;		sv->Init("clrHue",	&f, 0.f,1.f, 1.f, 3,5);  sv->DefaultF(0.53f);  Sev(UpdClr);
	sv= &svSat;		sv->Init("clrSat",	&f, 0.f,1.f, 1.f, 3,5);  sv->DefaultF(0.1f);  Sev(UpdClr);
	sv= &svVal;		sv->Init("clrVal",	&f, 0.f,3.f, 2.f, 3,5);  sv->DefaultF(0.5f);  Sev(UpdClr);

	sv= &svAlp;		sv->Init("clrAlp",	&f, 0.f,2.f, 2.f, 3,5);  sv->DefaultF(1.f);   Sev(UpdClr);
	sv= &svNeg;		sv->Init("clrNeg",	&f, 0.f,1.f, 1.f, 3,5);  sv->DefaultF(0.f);   Sev(UpdClr);


	///  [Settings]
	//------------------------------------------------------------------------
	sv= &svCamSpeed;	sv->Init("CamSpeed",	&pSet->cam_speed, 0.1f,4.f);  sv->DefaultF(0.9f);
	sv= &svCamInert;	sv->Init("CamInert",	&pSet->cam_inert, 0.f, 1.f);  sv->DefaultF(0.4f);

	ck= &ckMinimap;		ck->Init("Minimap",		&pSet->trackmap);  Cev(Minimap);
	sv= &svSizeMinimap;	sv->Init("SizeMinimap",	&pSet->size_minimap, 0.15f,2.f);  sv->DefaultF(0.55f);  Sev(SizeMinimap);

	sv= &svSizeRoadP;	sv->Init("SizeRoadP",	&pSet->road_sphr, 0.1f,12.f);  sv->DefaultF(2.f);  Sev(SizeRoadP);

	sv= &svTerUpd;		sv->Init("TerUpd",		&pSet->ter_skip,  0, 20);  sv->DefaultI(1);
	sv= &svMiniUpd;		sv->Init("MiniUpd",		&pSet->mini_skip, 0, 20);  sv->DefaultI(4);

	ck= &ckFps;			ck->Init("Fps",			&pSet->show_fps);  Cev(Fps);
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
	ck= &ckScreenPng;   ck->Init("ScreenPng",   &pSet->screen_png);
	

	///  [Sun]
	//------------------------------------------------------------------------
	sv= &svSkyYaw;		sv->Init("SkyYaw",		&sc->skyYaw,  -180.f,180.f, 1.f, 1,4);  sv->DefaultF(0.f);  Sev(UpdSky);
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
	sv= &svFogHDmg;		sv->Init("FogHDmg",		&sc->fHDamage,    0.f, 30.f, 1.5f, 1,3);  sv->DefaultF(0.f);

	ck= &ckFog;			ck->Init("FogDisable",		&pSet->bFog);  Cev(Fog);
	ck= &ckWeather;		ck->Init("WeatherDisable",	&pSet->bWeather);

	//  clr imgs
	ImgB(clrAmb,  "ClrAmb",  btnClrSet);  ImgB(clrDiff, "ClrDiff",  btnClrSet);
	ImgB(clrSpec, "ClrSpec", btnClrSet);
	ImgB(clrFog,  "ClrFog",  btnClrSetA); ImgB(clrFog2, "ClrFog2",  btnClrSetA);
	ImgB(clrFogH, "ClrFogH", btnClrSetA);
	ImgB(clrTrail,"ClrTrail",btnClrSetA);


	///  [Terrain]
	//------------------------------------------------------------------------
	imgTexDiff = fImg("TerImgDiff");
	Tab(tabsHmap, "TabHMapSize", tabHmap);
	sv= &svTerErrorNorm;  sv->Init("TerErrorNorm", &sc->td.errorNorm,  1.5f,15.f, 1.5f,1,3);  sv->DefaultF(3.f);  Sev(TerErrorNorm);
	sv= &svTerNormScale;  sv->Init("TerNormScale", &sc->td.normScale,  0.01f,3.f, 1.f, 1,3);  sv->DefaultF(1.f);  Sev(TerPar);
	sv= &svTerSpecPow;    sv->Init("TerSpecPow",   &sc->td.specularPow,   0.2f,128.f,2.f, 1,4);  sv->DefaultF(32.f); Sev(TerPar);
	sv= &svTerSpecPowEm;  sv->Init("TerSpecPowEm", &sc->td.specularPowEm, 0.5f,4.f,  1.f, 1,3);  sv->DefaultF(2.f);  Sev(TerPar);

	Btn("TerrainNew", btnTerrainNew);
	Btn("TerrainGenAdd", btnTerGenerate);  Btn("TerrainGenSub", btnTerGenerate);    Btn("TerrainGenMul", btnTerGenerate);
	Btn("TerrainHalf",   btnTerrainHalf);  Btn("TerrainDouble", btnTerrainDouble);  Btn("TerrainMove",   btnTerrainMove);


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


	///  [Layers]
	//------------------------------------------------------------------------
	bool b;
	ck= &ckTerLayOn;	ck->Init("TerLayOn",	&b);   Cev(TerLayOn);
	valTerLAll = fTxt("TerLayersAll");
	valTriplAll = fTxt("TerTriplAll");
	Tab(tabsTerLayers, "TabTerLay", tabTerLayer);
	Btn("UpdateLayers", btnUpdateLayers);

	ck= &ckTexNormAuto;	ck->Init("TexNormAuto",	&bTexNormAuto);
	ck= &ckTerLayTripl;	ck->Init("TerLayTripl",	&b);   Cev(TerLayTripl);
	ck= &ckDebugBlend;	ck->Init("DebugBlend",  &bDebugBlend);  Cev(DebugBlend);
	dbgLclr = fImg("dbgTerLclr");

	//  ter layer
	sv= &svTerTriSize;	sv->Init("TerTriSize", &sc->td.fTriangleSize,  0.5f,3.f, 1.f);  sv->DefaultF(1.4f);  Sev(TerTriSize);
	sv= &svTerLScale;	sv->Init("TerLScale",  &f, 6.0f, 72.f,  2.f);  sv->DefaultF(8.f);  //Sev(TerLay);
	//  blendmap
	sv= &svTerLAngMin;  sv->Init("TerLAngMin", &f, 0.f,  90.f,  1.f, 1,4);  sv->DefaultF(0.f);  Sev(TerLay);
	sv= &svTerLAngMax;  sv->Init("TerLAngMax", &f, 0.f,  90.f,  1.f, 1,4);  sv->DefaultF(90.f);  Sev(TerLay);
	sv= &svTerLAngSm;   sv->Init("TerLAngSm",  &f, 0.f,  90.f,  2.f, 1,4);  sv->DefaultF(20.f);  Sev(TerLay);

	sv= &svTerLHMin;    sv->Init("TerLHMin",   &f,-150.f,150.f, 1.f, 0,2);  sv->DefaultF(-300.f);  Sev(TerLay);
	sv= &svTerLHMax;    sv->Init("TerLHMax",   &f,-150.f,150.f, 1.f, 0,2);  sv->DefaultF( 300.f);  Sev(TerLay);
	sv= &svTerLHSm;     sv->Init("TerLHSm",    &f, 0.f,  100.f, 2.f, 1,4);  sv->DefaultF(20.f);  Sev(TerLay);
	Btn("TerLmoveL", btnTerLmoveL);  Btn("TerLmoveR", btnTerLmoveR);

	//  noise
	ck= &ckTerLNOnly;   ck->Init("TerLNonly",  &b);   Cev(TerLNOnly);
	sv= &svTerLNoise;   sv->Init("TerLNoise",  &f, 0.f,1.f);  sv->DefaultF(0.f);  Sev(TerLay);
	sv= &svTerLNprev;   sv->Init("TerLNprev",  &f, 0.f,1.f);  sv->DefaultF(0.f);  Sev(TerLay);
	sv= &svTerLNnext2;  sv->Init("TerLNnext2", &f, 0.f,1.f);  sv->DefaultF(0.f);  Sev(TerLay);
	sv= &svTerLDmg;     sv->Init("TerLDmg",    &f, 0.f, 6.f, 1.5f, 1,3);  sv->DefaultF(0.f);
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
	sv= &svLDust;   sv->Init("LDust",  &f, 0.f,1.f);  sv->DefaultF(0.f);
	sv= &svLDustS;  sv->Init("LDustS", &f, 0.f,1.f);  sv->DefaultF(0.f);
	sv= &svLMud;    sv->Init("LMud",   &f, 0.f,1.f);  sv->DefaultF(0.f);
	sv= &svLSmoke;  sv->Init("LSmoke", &f, 0.f,1.f);  sv->DefaultF(0.f);
	SldUpd_Surf();
	Cmb(cmbParDust, "CmbParDust", comboParDust);
	Cmb(cmbParMud,  "CmbParMud",  comboParDust);
	Cmb(cmbParSmoke,"CmbParSmoke",comboParDust);

	//  surface
	Cmb(cmbSurface, "Surface", comboSurface);  //1 txt-
	txtSuBumpWave = fTxt("SuBumpWave");   txtSuFrict  = fTxt("SuFrict");
	txtSuBumpAmp  = fTxt("SuBumpAmp");	  txtSurfTire = fTxt("SurfTire");
	txtSuRollDrag = fTxt("SuRollDrag");	  txtSurfType = fTxt("SurfType");
	SldUpd_TerL();

	
	///  [Vegetation]
	//------------------------------------------------------------------------
	sv= &svGrassDens;	sv->Init("GrassDens",	&sc->densGrass, 0.f, 1.f, 2.f, 3,5);  sv->DefaultF(0.2f);
	sv= &svTreesDens;	sv->Init("TreesDens",	&sc->densTrees, 0.f, 3.f, 2.f, 2,4);  sv->DefaultF(0.3f);

	Ed(GrPage, editTrGr);  Ed(GrDist, editTrGr);
	Ed(TrPage, editTrGr);  Ed(TrDist, editTrGr);  Ed(TrImpDist, editTrGr);
	
	sv= &svTrRdDist;	sv->Init("TrRdDist",	&sc->trRdDist,     0,6);   sv->DefaultI(1);
	sv= &svGrDensSmooth;sv->Init("GrDensSmooth",&sc->grDensSmooth, 0,10);  sv->DefaultI(3);

	//  veget models
	ck= &ckPgLayOn;		ck->Init("LTrEnabled",	&b);   Cev(PgLayOn);
	valLTrAll = fTxt("LTrAll");
	Tab(tabsPgLayers, "LTrNumTab", tabPgLayers);
	Btn("UpdateVeget", btnUpdateVeget);

	sv= &svLTrDens;		sv->Init("LTrDens",		 &f, 0.f, 1.f, 2.f, 3,5);  sv->DefaultF(0.15f);
	
	sv= &svLTrRdDist;	sv->Init("LTrRdDist",	 &i, 0,20);  sv->DefaultI(0);
	sv= &svLTrRdDistMax;sv->Init("LTrRdDistMax", &i, 0,20);  sv->DefaultI(20);
	
	sv= &svLTrMinSc;	sv->Init("LTrMinSc",	 &f, 0.f,4.f, 3.f, 3,5);  sv->DefaultF(0.7f);  Sev(LTrSc);
	sv= &svLTrMaxSc;	sv->Init("LTrMaxSc",	 &f, 0.f,4.f, 3.f, 3,5);  sv->DefaultF(1.2f);  Sev(LTrSc);
	
	sv= &svLTrWindFx;	sv->Init("LTrWindFx",	 &f, 0.f,12.f, 3.f, 3,5);  sv->DefaultF(0.5f);
	sv= &svLTrWindFy;	sv->Init("LTrWindFy",	 &f, 0.f,0.4f, 3.f, 3,5);  sv->DefaultF(0.06f);
	
	sv= &svLTrMaxTerAng;sv->Init("LTrMaxTerAng", &f, 0.f,90.f, 2.f, 1,4);  sv->DefaultF(30.f);

	sv= &svLTrMinTerH;	sv->Init("LTrMinTerH",	 &f,-60.f,60.f, 1.f, 1,4);  sv->DefaultF(-100.f);
	sv= &svLTrMaxTerH;	sv->Init("LTrMaxTerH",	 &f, 0.f,120.f, 1.f, 1,4);  sv->DefaultF( 100.f);
	sv= &svLTrFlDepth;	sv->Init("LTrFlDepth",	 &f, 0.f,5.f, 2.f, 1,4);  sv->DefaultF(0.f);
	SldUpd_PgL();  // real &f set here

	txVCnt = fTxt("LTrInfCnt");
	txVHmin = fTxt("LTrInfHmin");  txVHmax = fTxt("LTrInfHmax");
	txVWmin = fTxt("LTrInfWmin");  txVWmax = fTxt("LTrInfWmax");
	

	///  Grass
	//------------------------------------------------------------------------
	Ed(GrSwayDistr, editTrGr);  Ed(GrSwayLen, editTrGr);  Ed(GrSwaySpd, editTrGr);

	imgGrass = fImg("ImgGrass");  imgGrClr = fImg("ImgGrClr");
	Cmb(cmbGrassClr, "CmbGrClr", comboGrassClr);

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
	Btn("UpdateGrass", btnUpdateGrass);

	sv= &svGrMinX;	sv->Init("GrMinX",	&f, 0.5f,4.f, 1.5f);  sv->DefaultF(1.2f);
	sv= &svGrMaxX;	sv->Init("GrMaxX",	&f, 0.5f,4.1, 1.5f);  sv->DefaultF(1.6f);
	sv= &svGrMinY;	sv->Init("GrMinY",	&f, 0.5f,4.f, 1.5f);  sv->DefaultF(1.2f);
	sv= &svGrMaxY;	sv->Init("GrMaxY",	&f, 0.5f,4.f, 1.5f);  sv->DefaultF(1.6f);
	sv= &svGrChan;	sv->Init("LGrChan",	&i, 0,3);  sv->DefaultI(0);
	sv= &svLGrDens;	sv->Init("LGrDens",	&f, 0.001f,1.f, 2.f, 3,5);  sv->DefaultF(0.22f);
	SldUpd_GrL();

	
	///  [Road]
	//------------------------------------------------------------------------
	sv= &svRdTcMul; 	sv->Init("RdTcMul", 	&f, 0.001f,0.3f, 1.5f, 3,5);  sv->DefaultF(0.1f);
	sv= &svRdTcMulW;	sv->Init("RdTcMulW",	&f, 0.005f,0.4f, 1.5f, 3,5);  sv->DefaultF(0.1f);
	sv= &svRdTcMulP;	sv->Init("RdTcMulP",	&f, 0.005f,0.3f, 1.5f, 3,5);  sv->DefaultF(0.2f);
	sv= &svRdTcMulPW;	sv->Init("RdTcMulPW",	&f, 0.005f,0.4f, 1.5f, 3,5);  sv->DefaultF(0.2f);
	sv= &svRdTcMulC;	sv->Init("RdTcMulC",	&f, 0.005f,0.4f, 1.5f, 3,5);  sv->DefaultF(0.2f);

	sv= &svRdLenDim;	sv->Init("RdLenDim",	&f, 0.5f, 4.f, 1.5f, 2,4);  sv->DefaultF(1.f);
	sv= &svRdWidthSteps;sv->Init("RdWidthSteps",&i, 3,16, 1.5f);  sv->DefaultI(6);
	sv= &svRdPlsM;		sv->Init("RdPlsM",		&f, 1.f, 8.f, 1.5f, 1,3);  sv->DefaultF(1.f);
	sv= &svRdPwsM;		sv->Init("RdPwsM",		&f, 1.f, 8.f, 1.5f, 1,3);  sv->DefaultF(4.f);

	sv= &svRdColN;		sv->Init("RdColN",		&i, 3,16, 1.5f);  sv->DefaultI(4);
	sv= &svRdColR;		sv->Init("RdColR",		&f, 1.0f, 8.f, 1.5f, 2,4);  sv->DefaultF(2.f);

	sv= &svRdMergeLen;	sv->Init("RdMergeLen",	&f, 60.f, 2400.f, 2.f, 0,3);  sv->DefaultF(800.f);
	sv= &svRdLodPLen;	sv->Init("RdLodPLen",	&f, 10.f, 200.f, 2.f, 0,2);  sv->DefaultF(30.f);
	sv= &svRdVisDist;	sv->Init("RdVisDist",	&f, 100.f, 2400.f, 2.f, 0,3);  sv->DefaultF(600.f);
	sv= &svRdVisBehind;	sv->Init("RdVisBehind",	&f, 100.f, 2400.f, 2.f, 0,3);  sv->DefaultF(600.f);
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
	ck= &ckNoWrongChks;		ck->Init("NoWrongChks",		&sc->noWrongChks);
	txtEdInfo = fTxt("EdInfo");
	
	
	///  [Pacenotes]  ------------------------------------
	sv= &svPaceShow;	sv->Init("PaceShow",	&pSet->pace_show,   0,5, 1.f);  sv->DefaultI(3);
	sv= &svPaceDist;	sv->Init("PaceDist",	&pSet->pace_dist,   20.f,2000.f, 2.f, 0,3);  sv->DefaultF(600.f);

	sv= &svPaceSize;	sv->Init("PaceSize",	&pSet->pace_size,   0.1f,2.f);  sv->DefaultF(1.f);  Sev(Upd_Pace);
	sv= &svPaceNear;	sv->Init("PaceNear",	&pSet->pace_near,   0.1f,2.f);  sv->DefaultF(1.f);  Sev(Upd_Pace);
	sv= &svPaceAlpha;	sv->Init("PaceAlpha",	&pSet->pace_alpha,  0.3f,1.f);  sv->DefaultF(1.f);  Sev(Upd_Pace);
	slUpd_Pace(0);
	ck= &ckTrkReverse;	ck->Init("ReverseOn",	&pSet->trk_reverse);  Cev(TrkReverse);
	

	///  [Surface]
	ck= &ckRoad1Mtr;	ck->Init("Road1Mtr",	&sc->td.road1mtr);
	

	///  [Tools]  ------------------------------------
	Btn("TrackCopySel", btnTrkCopySel);
	valTrkCpySel = fTxt("TrkCopySelName");

	Btn("CopySun", btnCopySun);				Btn("CopyTerHmap", btnCopyTerHmap);
	Btn("CopyTerLayers", btnCopyTerLayers);	Btn("CopyVeget", btnCopyVeget);
	Btn("CopyRoad", btnCopyRoad);			Btn("CopyRoadPars", btnCopyRoadPars);

	Btn("DeleteRoad", btnDeleteRoad);		Btn("DeleteFluids", btnDeleteFluids);
	Btn("DeleteObjects", btnDeleteObjects); Btn("DeleteParticles", btnDeleteParticles);

	sv= &svScaleAllMul;		sv->Init("ScaleAllMul",		&fScale,    0.5f,2.f, 1.5f);  sv->DefaultF(1.f);
	sv= &svScaleTerHMul;	sv->Init("ScaleTerHMul",	&fScaleTer, 0.5f,2.f, 1.5f);  sv->DefaultF(1.f);
	Btn("ScaleAll",  btnScaleAll);  Btn("ScaleTerH", btnScaleTerH);

	sv= &svAlignWidthAdd;	sv->Init("AlignWidthAdd",	&pSet->al_w_add,  0.f,20.f,1.f, 1,3);  sv->DefaultF(10.f);
	sv= &svAlignWidthMul;	sv->Init("AlignWidthMul",	&pSet->al_w_mul,  1.f,4.f, 1.f, 2,4);  sv->DefaultF(1.f);
	sv= &svAlignSmooth;		sv->Init("AlignSmooth",		&pSet->al_smooth, 0.f,6.f, 1.f, 1,3);  sv->DefaultF(3.f);

	
	///  [Warnings]  ------------------------------------
	edWarn = fEd("Warnings");
	txWarn = mGui->createWidget<TextBox>("TextBox", 300,20, 360,32, Align::Default, "Back");
	txWarn->setTextShadow(true);  txWarn->setTextShadowColour(Colour::Black);
	txWarn->setTextColour(Colour(1.0,0.4,0.2));  txWarn->setFontHeight(24);
	txWarn->setVisible(false);

	imgWarn = fImg("ImgWarn");  imgWarn->setVisible(false);
	imgInfo = fImg("ImgInfo");

	ck= &ckCheckSave;	ck->Init("CheckSave",	&pSet->check_save);
	ck= &ckCheckLoad;	ck->Init("CheckLoad",	&pSet->check_load);
	

	///  Fill Combo boxes  . . . . . . .
	//------------------------------------------------------------------------------------------------------------

	string sData = PATHMANAGER::Data();
	String sMat = sData +"/materials/scene/";  // path

	
	//---------------------  Game, Reverbs  ---------------------
	txtRevebDescr = fTxt("txtRevebDescr");
	Cmb(cmbReverbs, "CmbReverbs", comboReverbs);

	for (u=0; u < data->reverbs->revs.size(); ++u)
		cmbReverbs->addItem(data->reverbs->revs[u].name);


	//---------------------  Weather  ---------------------
	Cmb(cmbRain1, "Rain1Cmb", comboRain1);  cmbRain1->addItem("");
	Cmb(cmbRain2, "Rain2Cmb", comboRain2);  cmbRain2->addItem("");

	GetMaterials("weather.particle", true, "particle_system");
	for (u=0; u < vsMaterials.size(); ++u)
	{	const String& s = vsMaterials[u];
		cmbRain1->addItem(s);  cmbRain2->addItem(s);
	}	

	//---------------------  Emitters  ---------------------
	GetMaterials("emitters.particle", true, "particle_system");
	app->vEmtNames.clear();
	for (u=0; u < vsMaterials.size(); ++u)
		app->vEmtNames.push_back(vsMaterials[u]);


	//---------------------  Terrain  ---------------------
	Cmb(cmbTexNorm, "TexNormal", comboTexNorm);  cmbTexNorm->addItem("flat_n.png");

	strlist li;
	PATHMANAGER::DirList(sData + (pSet->tex_size > 0 ? "/terrain" : "/terrain_s"), li);
	for (auto q : li)
	{
		if (StringUtil::match(q, "*_n.*", false))
			cmbTexNorm->addItem(q);
		//else
		//if (StringUtil::match(*q, "*_d.*", false))  //_T
		//	cmbTexDiff->addItem(*q);
	}
	
	//  particles
	GetMaterials("tires.particle", true, "particle_system");
	for (u=0; u < vsMaterials.size(); ++u)
	{	const String& s = vsMaterials[u];
		cmbParDust->addItem(s);  cmbParMud->addItem(s);  cmbParSmoke->addItem(s);
	}
	
	//  surfaces
	for (u=0; u < app->surfaces.size(); ++u)
		cmbSurface->addItem(app->surfaces[u].name);
	

	//---------------------  Grass  ---------------------
	PATHMANAGER::DirList(sData + "/grass", li);
	for (auto q : li)
	{
		if (StringUtil::startsWith(q, "grClr", false))
			cmbGrassClr->addItem(q);
	}


	//---------------------  Objects  ---------------------
	app->vObjNames.clear();  strlist lo;
	PATHMANAGER::DirList(sData + "/objects2", lo);
	PATHMANAGER::DirList(sData + "/objects", lo);
	for (auto q : lo)
		if (StringUtil::endsWith(q, ".mesh"))
		{	string name = q.substr(0, q.length()-5);  //no .ext
			if (name != "sphere")
				app->vObjNames.push_back(name);
		}
	
	objListDyn = fLi("ObjListDyn");  Lev(objListDyn, ObjsChng);
	objListSt  = fLi("ObjListSt");   Lev(objListSt,  ObjsChng);
	objListBld = fLi("ObjListBld");  Lev(objListBld, ObjsChng);
	objListCat = fLi("ObjListCat");  Lev(objListCat, ObjsCatChng);
	objPan = fWP("objPan");


	for (u=0; u < app->vObjNames.size(); ++u)
	{	const string& name = app->vObjNames[u];
		if (name != "sphere" && PATHMANAGER::FileExists(sData+"/objects/"+ name + ".bullet"))
			objListDyn->addItem("#A0E0FF"+name);  // dynamic
	}
	
	//  buildings, group categories, more with same prefix  ----
	app->vBuildings.clear();
	objListCat->removeAllItems();

	auto AddPath = [&](auto path)
	{
		std::map<string, int> cats;  // yeah cats are fun
		int b0 = app->vBuildings.size();
		lo.clear();
		PATHMANAGER::DirList(sData + path, lo);

		for (auto q : lo)
			if (StringUtil::endsWith(q,".mesh"))
			{	string name = q.substr(0, q.length()-5);  //no .ext
				if (name != "sphere" && !PATHMANAGER::FileExists(sData+"/objects/"+ name + ".bullet"))
				{	// no dynamic

					// auto id = name.find('_') ?..
					string cat = name.substr(0,4);
					++cats[cat];
					app->vBuildings.push_back(name);
					app->vObjNames.push_back(name);  // in -,=
			}	}
		//  get cats  ----
		for (auto it : cats)
		{
			string cat = it.first;  int n = it.second;
			//LogO(cat+" "+toStr(n));
			if (n > 1)  // add category (> 1 Bld with this prefix)
				objListCat->addItem("#F0B0B0"+cat);
		}
		//  push Bld back to Obj list (if <= 1, rare cat)
		for (u = b0; u < app->vBuildings.size(); ++u)
		{
			const string& name = app->vBuildings[u];
			string cat = name.substr(0,4);
			if (cats[cat] <= 1)
				objListSt->addItem("#D0D0C8"+name);
		}
		{	int il = objListCat->findItemIndexWith("#E09090"+pSet->objGroup);
			objListCat->setIndexSelected(il);
			listObjsCatChng(objListCat, il);
		}
	};
	AddPath("/objects");
	AddPath("/objects2");
	objListCat->addItem("#FFB060---- "+TR("#{ObjRocks}"));
	AddPath("/rocks");
	objListCat->addItem("#C0C8C0---- City");
	AddPath("/objectsC");
	objListCat->addItem("#E0E0A0---- 0 A.D.");
	AddPath("/objects0");


	//---------------------  Surfaces  ---------------------
	surfList = fLi("SurfList");  Lev(surfList, Surf);
	for (n=0; n < 4; ++n)  surfList->addItem("#80FF00"+TR("#{Layer} ")+toStr(n));
	for (n=0; n < 4; ++n)  surfList->addItem("#FFB020"+TR("#{Road} ")+toStr(n));
	for (n=0; n < 4; ++n)  surfList->addItem("#FFFF80"+TR("#{Pipe} ")+toStr(n));
	surfList->setIndexSelected(0);
	
	
	//---------------------  Tweak  ---------------------
	ComboBoxPtr cmbTwk;
	Cmb(cmbTwk, "TweakMtr", comboTweakMtr);

	GetMaterialsMat(sData +"/materials/water.mat");
	GetMaterialsMat(sMat+"pipe.mat",false);
	GetMaterialsMat(sMat+"road.mat",false);
	GetMaterialsMat(sMat+"objects_static.mat",false);

	cmbTwk->addItem("");
	for (u=0; u < vsMaterials.size(); ++u)
		cmbTwk->addItem(vsMaterials[u]);

	cmbTwk->setIndexSelected( cmbTwk->findItemIndexWith(pSet->tweak_mtr) );

	
	///  [Pick window]
	///------------------------------------------------------------------------------------------------------------
	//  Pick btns
	Btn("PickSky", btnPickSky);      btn->eventMouseWheel += newDelegate(this, &CGui::wheelSky);  btnSky = btn;
	Btn("PickTex", btnPickTex);      btn->eventMouseWheel += newDelegate(this, &CGui::wheelTex);  btnTexDiff = btn;
	Btn("PickGrass", btnPickGrass);  btn->eventMouseWheel += newDelegate(this, &CGui::wheelGrs);  btnGrassMtr = btn;
	Btn("PickVeget", btnPickVeget);  btn->eventMouseWheel += newDelegate(this, &CGui::wheelVeg);  btnVeget = btn;
	
	auto btnWhRd = [&](){  btn->eventMouseWheel += newDelegate(this, &CGui::wheelRd);  };
	for (n=0; n < 4; ++n)
	{	Btn(toStr(n)+"RdMtr",  btnPickRoad);  btnRoad[n] = btn;  btnWhRd();
		Btn(toStr(n)+"RdMtrP", btnPickPipe);  btnPipe[n] = btn;  btnWhRd();
	}
	Btn("0RdMtrW",  btnPickRoadW);    btnRoadW = btn;    btnWhRd();
	Btn("0RdMtrPW", btnPickPipeW);    btnPipeW = btn;    btnWhRd();
	Btn("0RdMtrC",  btnPickRoadCol);  btnRoadCol = btn;  btnWhRd();

	ck= &ckPickSetPar;	ck->Init("PickSetPar",	&pSet->pick_setpar);
	panPick = fWP("PanelPick");

	FillPickLists();

    //TrackListUpd(true);  //upd
	//listTrackChng(trkList,0);

	

	///  [Track]
	//------------------------------------------------------------------------
	gcom->sListTrack = pSet->gui.track;  //! set last
	gcom->bListTrackU = pSet->gui.track_user;
	sCopyTrack = "";  //! none
	bCopyTrackU = 0;
	
	//  text desc
	Edt(gcom->trkDesc[0], "TrackDesc0", editTrkDescr);
	Edt(gcom->trkAdvice[0], "TrackAdvice0", editTrkAdvice);
	trkName = fEd("TrackName");
	if (trkName)  trkName->setCaption(pSet->gui.track);

	gcom->GuiInitTrack();
	
	//  btn  new, rename, delete
	Btn("TrackNew",		btnTrackNew);
	Btn("TrackRename",	btnTrackRename);
	Btn("TrackDelete",	btnTrackDel);

	Btn("OpenEdTut", btnEdTut);

    //  load = new game
    for (i=0; i <= 2; ++i)
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


///  Fill Pick Lists
///------------------------------------------------------------------------------------------------------------
void CGui::FillPickLists()
{
	///  Sky Mtr  --------
	Mli2 lp;  int l,u;
	lp = app->mWndPick->createWidget<MultiList2>("MultiListBox",8,8,400,800, Align::Left | Align::VStretch);
	liSky = lp;  lp->eventListChangePosition += newDelegate(this, &CGui::listPickSky);
	lp->setColour(Colour(0.7,0.85,1.0));  lp->setInheritsAlpha(false);
	const int rt = 22, sl = 21;
	
	lp->removeAllColumns();  lp->removeAllItems();
	lp->addColumn("#90C0F0", 15);
	lp->addColumn("#E0F0FF"+TR("#{Sky}"), 200);
	lp->addColumn("#E0F0FF"+TR("#{Pitch}"), 60);
	lp->addColumn(" ", sl);
	liPickX[P_Sky] = 0.45f;  liPickW[P_Sky] = 280;

	for (u=0; u < data->pre->sky.size(); ++u)
	{	const PSky& s = data->pre->sky[u];
		String c = s.clr;
		lp->addItem(c, 0);
		l = lp->getItemCount()-1;

		lp->setSubItemNameAt(1,l, c+ s.mtr);
		lp->setSubItemNameAt(2,l, c+ fToStr( s.ldPitch, 0,2));
	}

	///  Terrain  Tex Diff  --------
	lp = app->mWndPick->createWidget<MultiList2>("MultiListBox",8,8,400,800, Align::Left | Align::VStretch);
	liTex = lp;  lp->eventListChangePosition += newDelegate(this, &CGui::listPickTex);
	lp->setColour(Colour(0.8,0.9,0.7));  lp->setInheritsAlpha(false);
	
	lp->removeAllColumns();  lp->removeAllItems();
	lp->addColumn("#90C0F0*", rt);
	lp->addColumn("#E0FFE0"+TR("#{Diffuse}"), 190);
	lp->addColumn("#80E0E0"+TR("#{Scale}"), 40);
	lp->addColumn("#80FF80|", 27);
	lp->addColumn(" ", sl);
	liPickX[P_Tex] = 0.45f;  liPickW[P_Tex] = 330;

	for (u=0; u < data->pre->ter.size(); ++u)
	{	const PTer& t = data->pre->ter[u];
		String c = gcom->scnClr[gcom->scnN[t.sc]];  if (c.empty())  c = "#000000";
		lp->addItem(
			t.rate ? gcom->getClrRating(t.rate) + toStr(t.rate) : c, 0);
		l = lp->getItemCount()-1;

		lp->setSubItemNameAt(1,l, c+ t.texFile.substr(0, t.texFile.length()-2));  // no _d
		if (t.rate)
		{	lp->setSubItemNameAt(2,l, gcom->getClrLong(t.tiling * 0.15f) + fToStr(t.tiling, 0,2));
			lp->setSubItemNameAt(3,l, (t.triplanar ? "#E0E0E01" : "#6565650"));
	}	}

	///  Grass  --------
	lp = app->mWndPick->createWidget<MultiList2>("MultiListBox",8,8,400,800, Align::Left | Align::VStretch);
	liGrs = lp;  lp->eventListChangePosition += newDelegate(this, &CGui::listPickGrs);
	lp->setColour(Colour(0.7,0.9,0.7));  lp->setInheritsAlpha(false);
	
	lp->removeAllColumns();  lp->removeAllItems();
	lp->addColumn("#90C0F0*", rt);
	lp->addColumn("#E0FFE0"+TR("#{GrMaterial}"), 152);
	lp->addColumn("#D0FFD0-", 41);  lp->addColumn("#C0F0C0|", 41);
	lp->addColumn(" ", sl);
	liPickX[P_Grs] = 0.36f;  liPickW[P_Grs] = 310;

	for (u=0; u < data->pre->gr.size(); ++u)
	{	const PGrass& t = data->pre->gr[u];
		String c = gcom->scnClr[gcom->scnN[t.sc]];  if (c.empty())  c = "#000000";
		lp->addItem(
			t.rate ? gcom->getClrRating(t.rate) + toStr(t.rate) : c, 0);
		l = lp->getItemCount()-1;

		lp->setSubItemNameAt(1,l, c+ t.mtr);
		if (t.rate)
		{	lp->setSubItemNameAt(2,l, c+ fToStr(t.maxSx, 1,3));
			lp->setSubItemNameAt(3,l, c+ fToStr(t.maxSy, 1,3));
	}	}

	///  Veget  --------------------------------
	lp = app->mWndPick->createWidget<MultiList2>("MultiListBox",8,8,400,800, Align::Left | Align::VStretch);
	liVeg = lp;  lp->eventListChangePosition += newDelegate(this, &CGui::listPickVeg);
	lp->setColour(Colour(0.7,0.9,0.9));  lp->setInheritsAlpha(false);
	
	lp->removeAllColumns();  lp->removeAllItems();
	lp->addColumn("#90C0F0*", rt);
	lp->addColumn("#E0FFE0"+TR("#{Model}"), 207);
	lp->addColumn("#80E0E0"+TR("#{Scale}"), 40);
	lp->addColumn(" ", sl);
	liPickX[P_Veg] = 0.36f;  liPickW[P_Veg] = 320;

	for (u=0; u < data->pre->veg.size(); ++u)
	{	const PVeget& t = data->pre->veg[u];
		String c = gcom->scnClr[gcom->scnN[t.sc]];  if (c.empty())  c = "#000000";
		lp->addItem(
			t.rate ? gcom->getClrRating(t.rate) + toStr(t.rate) : c, 0);
		l = lp->getItemCount()-1;

		lp->setSubItemNameAt(1,l, c+ t.name);
		if (t.rate)
			lp->setSubItemNameAt(2,l, c+ fToStr(t.maxScale, 1,3));
			//lp->setSubItemNameAt(3,l, c+ fToStr(t.maxTerAng, 0,2));
	}

	///  Road  --------------------------------
	lp = app->mWndPick->createWidget<MultiList2>("MultiListBox",8,8,400,800, Align::Left | Align::VStretch);
	liRd = lp;  lp->eventListChangePosition += newDelegate(this, &CGui::listPickRd);
	lp->setColour(Colour(0.9,0.8,0.7));  lp->setInheritsAlpha(false);
	
	lp->removeAllColumns();  lp->removeAllItems();
	lp->addColumn("#90C0F0*", rt);
	lp->addColumn("#FFE0D0"+TR("#{GrMaterial}"), 257);
	//lp->addColumn("#80E0E0"+TR("#{Surface}"), 80);
	lp->addColumn(" ", sl);
	liPickX[P_Rd] = 0.36f;  liPickW[P_Rd] = 320;

	for (u=0; u < data->pre->rd.size(); ++u)
	{	const PRoad& t = data->pre->rd[u];
		String c = gcom->scnClr[gcom->scnN[t.sc]];  if (c.empty())  c = "#000000";
		lp->addItem(
			t.rate ? gcom->getClrRating(t.rate) + toStr(t.rate) : c, 0);
		l = lp->getItemCount()-1;

		lp->setSubItemNameAt(1,l, c+ t.mtr);
		/*String su = t.surfName;  if (su.substr(0,4)=="road")  su = su.substr(4, su.length());
		if (t.rate && t.mtr.substr(0,5) != "River")
			lp->setSubItemNameAt(2,l, c+ su);*/
	}
	
	// todo: auto groups chks gui? to filter pick lists
}
