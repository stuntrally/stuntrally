#include "pch.h"
#include "Defines.h"
#include "OgreApp.h"
#include "../vdrift/pathmanager.h"
//#include "../road/Road.h"
using namespace MyGUI;
using namespace Ogre;


///  Gui Init
//----------------------------------------------------------------------------------------------------------------------

void App::InitGui()
{
	bGuiFocus = false/*true*/;  bMoveCam = true;  //*--
	if (!mGUI)  return;
	LanguageManager::getInstance().loadUserTags("core_theme_black_blue_tag.xml");

	//  load layout - wnds
	VectorWidgetPtr& rootV = LayoutManager::getInstance().load("Editor.layout");
	for (VectorWidgetPtr::iterator it = rootV.begin(); it != rootV.end(); ++it)
	{
		setToolTips((*it)->getEnumerator());
		const std::string& name = (*it)->getName();
		if (name == "CamWnd")  mWndCam = *it;  else
		if (name == "BrushWnd")  mWndBrush = *it;  else
		if (name == "RoadCur")   mWndRoadCur = *it;  else
		if (name == "RoadNew")   mWndRoadNew = *it;  else
		if (name == "RoadStats")  mWndRoadStats = *it;  else
		if (name == "OptionsWnd")  mWndOpts = *it;
	}
	if (mWndRoadStats)  mWndRoadStats->setVisible(false);

	//  tooltip
	mToolTip = Gui::getInstance().findWidget<Widget>("ToolTip");
	mToolTip->setVisible(false);
	mToolTipTxt = mToolTip->getChildAt(0)->castType<Edit>();

	//  Brush wnd  assign controls  ----------------------
	if (mWndBrush)
	{	for (int i=0; i<BR_TXT; ++i)
			brTxt[i] = (StaticTextPtr)mWndBrush->findWidget("brush"+toStr(i));
		brImg = (StaticImagePtr)mWndBrush->findWidget("brushImg");
	}

	//  Road wnds
	if (mWndRoadCur)  for (int i=0; i<RD_TXT; ++i)
		rdTxt[i] = (StaticTextPtr)mWndRoadCur->findWidget("rdCur"+toStr(i));
	if (mWndRoadStats)  for (int i=0; i<RDS_TXT; ++i)
		rdTxtSt[i] = (StaticTextPtr)mWndRoadStats->findWidget("rdStat"+toStr(i));

	//  Options wnd
	if (mWndOpts)
	{	mWndOpts->setVisible(false);
		int sx = mWindow->getWidth(), sy = mWindow->getHeight();
		IntSize w = mWndOpts->getSize();  // center
		mWndOpts->setPosition((sx-w.width)*0.5f, (sy-w.height)*0.5f);
		mWndTabs = (TabPtr)mWndOpts->findWidget("TabWnd");
		//mWndTabs->setIndexSelected(3);  //def*--
	}

	//  center mouse pos
	mGUI->setVisiblePointer(bGuiFocus || !bMoveCam);
	int xm = mWindow->getWidth()/2, ym = mWindow->getHeight()/2;
	InputManager::getInstance().injectMouseMove(xm, ym, 0);
	OIS::MouseState &ms = const_cast<OIS::MouseState&>(mMouse->getMouseState());
	ms.X.abs = xm;  ms.Y.abs = ym;
	
	//  hide  ---
	edMode = ED_Road;  UpdEditWnds();  // *  UpdVisHit(); //after track
	if (!mWndOpts)  return;  // error
	
	
	//  Shortcuts: find, assign event, set value
	//------------------------------------------------------------------------
	ButtonPtr btn, bchk;  ComboBoxPtr combo;
	HScrollPtr sl;  size_t v;

	#define Slv(name, vset)  \
		sl = (HScrollPtr)mWndOpts->findWidget(#name);  \
		if (sl)  sl->eventScrollChangePosition = newDelegate(this, &App::sl##name);  \
		val##name = (StaticTextPtr)(mWndOpts->findWidget(#name"Val"));  \
		v = vset*res;  if (sl)  sl->setScrollPosition(v);	sl##name(sl, v);

	#define Btn(name, event)  \
		btn = (ButtonPtr)mWndOpts->findWidget(name);  \
		if (btn)  btn->eventMouseButtonClick = newDelegate(this, &App::event);

	#define Chk(name, event, var)  \
		bchk = mGUI->findWidget<Button>(name);  \
		if (bchk)  {  bchk->eventMouseButtonClick = newDelegate(this, &App::event);  \
			bchk->setStateCheck(var);  }

	#define Edt(edit, name, event)  \
		edit = (EditPtr)mWndOpts->findWidget(name);  \
		if (edit)  edit->eventEditTextChange = newDelegate(this, &App::event);

	#define Ed(name, evt)  Edt(ed##name, #name, evt)
		
	#define Cmb(cmb, name, event)  \
		cmb = (ComboBoxPtr)mWndOpts->findWidget(name);  \
		cmb->eventComboChangePosition = newDelegate(this, &App::event);

	#define Tab(tab, name, event)  \
		tab = (TabPtr)mWndOpts->findWidget(name);  \
		tab->eventTabChangeSelect = newDelegate(this, &App::event);



	///  [Graphics]
	//------------------------------------------------------------------------
	//  detail
	Slv(TerDetail,	powf(pSet->terdetail /20.f, 0.5f));
	Slv(TerDist,	powf(pSet->terdist /2000.f, 0.5f));
	Slv(ViewDist,	powf((pSet->view_distance -50.f)/6950.f, 0.5f));
	Slv(RoadDist,	powf(pSet->road_dist /4.f, 0.5f));

	//  textures
	Cmb(combo, "TexFiltering", comboTexFilter);
	Slv(Anisotropy,	pSet->anisotropy /res);
	Slv(Shaders,	pSet->shaders /res);

	//  trees/grass
	Slv(Trees,		powf(pSet->trees /4.f, 0.5f));
	Slv(Grass,		powf(pSet->grass /4.f, 0.5f));
	Slv(TreesDist,	powf((pSet->trees_dist-0.5f) /6.5f, 0.5f));
	Slv(GrassDist,	powf((pSet->grass_dist-0.5f) /6.5f, 0.5f));
	Btn("TrGrReset", btnTrGrReset);

	//  shadows
	Slv(ShadowType,	pSet->shadow_type /res);
	Slv(ShadowCount,(pSet->shadow_count-2) /2.f);
	Slv(ShadowSize,	pSet->shadow_size /float(ciShadowNumSizes));
	Slv(ShadowDist,	powf((pSet->shadow_dist -50.f)/4750.f, 0.5f));
	Btn("Apply", btnShadows);


	///  [Settings]
	//------------------------------------------------------------------------
	Chk("Minimap", chkMinimap, pSet->trackmap);
	Slv(SizeMinmap,	(pSet->size_minimap-0.15f) /1.85f);
	Slv(CamSpeed, powf((pSet->cam_speed-0.1f) / 3.9f, 1.f));
	Slv(CamInert, pSet->cam_inert);
	Slv(TerUpd, pSet->ter_skip /res);
	Slv(MiniUpd, pSet->mini_skip /res);
	Slv(SizeRoadP, (pSet->road_sphr-0.1f) /11.9f);

	//  set camera btns
	Btn("CamView1", btnSetCam);  Btn("CamView2", btnSetCam);
	Btn("CamView3", btnSetCam);  Btn("CamView4", btnSetCam);
	Btn("CamTop", btnSetCam);
	Btn("CamLeft", btnSetCam);   Btn("CamRight", btnSetCam);
	Btn("CamFront", btnSetCam);  Btn("CamBack", btnSetCam);

	//  startup
	Chk("OgreDialog", chkOgreDialog, pSet->ogre_dialog);
	Chk("AutoStart", chkAutoStart, pSet->autostart);
	Chk("EscQuits", chkEscQuits, pSet->escquit);
	bnQuit = mGUI->findWidget<Button>("Quit");
	if (bnQuit)  {  bnQuit->eventMouseButtonClick = newDelegate(this, &App::btnQuit);  bnQuit->setVisible(false);  }
	

	///  [Sky]
	//----------------------------------------------------------------------------------------------
	Slv(SunPitch,0);  Slv(SunYaw,0);
	Slv(FogStart,0);  Slv(FogEnd,0);
	Chk("FogDisable", chkFogDisable, pSet->bFog);  chkFog = bchk;
	Ed(LiAmb, editLiAmb);  Ed(LiDiff, editLiDiff);  Ed(LiSpec, editLiSpec);
	Ed(FogClr, editFogClr);
	Slv(Rain1Rate,0);  Slv(Rain2Rate,0);


	///  [Terrain]
	//------------------------------------------------------------------------
	imgTexDiff = (StaticImagePtr)mWndOpts->findWidget("TerImgDiff");
	Tab(tabsHmap, "TabHMapSize", tabHmap);
	Tab(tabsTerLayers, "TabTerLay", tabTerLayer);

	Edt(edTerTriSize, "edTerTriSize", editTerTriSize);
	Edt(edTerLScale, "edTerLScale", editTerLScale);
	Slv(TerTriSize,	powf((sc.td.fTriangleSize -0.1f)/5.9f, 0.5f));
	Slv(TerLScale, 0);
	Btn("TerrainNew", btnTerrainNew);
	Btn("TerrainResize", btnTerrainResize);

	Chk("TerLayOn", chkTerLayOn, 1);  chkTerLay = bchk;
	valTerLAll = (StaticTextPtr)mWndOpts->findWidget("TerLayersAll");
	Chk("TexNormAuto", chkTexNormAutoOn, 1);  chkTexNormAuto = bchk;
	Ed(LDust, editLDust);	Ed(LDustS, editLDust);
	Ed(LMud,  editLDust);	Ed(LSmoke, editLDust);
	Ed(LTrlClr, editLTrlClr);
	Cmb(cmbParDust, "CmbParDust", comboParDust);
	Cmb(cmbParMud,  "CmbParMud",  comboParDust);
	Cmb(cmbParSmoke,"CmbParSmoke",comboParDust);

	Cmb(cmbSurfType, "SurfType", comboSurfType);
	Ed(SuBumpWave, editSurf);	Ed(SuBumpAmp, editSurf);
	Ed(SuRollDrag, editSurf);
	Ed(SuFrict, editSurf);	Ed(SuFrict2, editSurf);

	
	///  [Vegetation]  ------------------------------------
	Ed(GrassDens, editTrGr);  Ed(TreesDens, editTrGr);
	Ed(GrPage, editTrGr);  Ed(GrDist, editTrGr);  Ed(TrPage, editTrGr);  Ed(TrDist, editTrGr);
	Ed(GrMinX, editTrGr);  Ed(GrMaxX, editTrGr);  Ed(GrMinY, editTrGr);  Ed(GrMaxY, editTrGr);
	Ed(GrSwayDistr, editTrGr);  Ed(GrSwayLen, editTrGr);  Ed(GrSwaySpd, editTrGr);
	Ed(TrRdDist, editTrGr);  Ed(TrImpDist, editTrGr);
	imgPaged = (StaticImagePtr)mWndOpts->findWidget("ImgPaged");

	Chk("LTrEnabled", chkPgLayOn, 1);  chkPgLay = bchk;
	valLTrAll = (StaticTextPtr)mWndOpts->findWidget("LTrAll");
	Tab(tabsPgLayers, "LTrNumTab", tabPgLayers);
	Slv(LTrDens, 0);	Slv(LTrRdDist, 0);
	Slv(LTrMinSc, 0);	Slv(LTrMaxSc, 0);
	Slv(LTrWindFx, 0);	Slv(LTrWindFy, 0);
	
	
	///  [Road]  ------------------------------------
	Ed(RdTcMul, editRoad);  Ed(RdLenDim, editRoad);  Ed(RdWidthSteps,editRoad);
	Ed(RdHeightOfs, editRoad);  Ed(RdSkirtLen, editRoad);  Ed(RdSkirtH, editRoad);
	Ed(RdMergeLen, editRoad);  Ed(RdLodPLen, editRoad);
	Ed(RdColN, editRoad);  Ed(RdColR, editRoad);
	Ed(RdPwsM, editRoad);  Ed(RdPlsM, editRoad);
	

	///  [Tools]  ------------------------------------
	Btn("TrackCopySel", btnTrkCopySel);
	valTrkCpySel = (StaticTextPtr)mWndOpts->findWidget("TrkCopySelName");
	Btn("CopySun", btnCopySun);
	Btn("CopyTerHmap", btnCopyTerHmap);
	Btn("CopyTerLayers", btnCopyTerLayers);
	Btn("CopyVeget", btnCopyVeget);
	Btn("CopyRoad", btnCopyRoad);
	Btn("CopyRoadPars", btnCopyRoadPars);

	Btn("DeleteRoad", btnDeleteRoad);
	Btn("ScaleAll", btnScaleAll);
	Ed(ScaleAllMul, editScaleAllMul);
	

	///  Fill Combo boxes  . . . . . . .
	//------------------------------------------------------------------------------------------------------------
	
	//---------------------  SKYS  ---------------------
	Cmb(cmbSky, "SkyCombo", comboSky);

	GetMaterials("SkyDome.material");
	for (size_t i=0; i < vsMaterials.size(); ++i)  {
		String s = vsMaterials[i];  cmbSky->addItem(s);  //LogO(s);
	}
	//---------------------  WEATHER  ---------------------
	Cmb(cmbRain1, "Rain1Cmb", comboRain1);  cmbRain1->addItem("");
	Cmb(cmbRain2, "Rain2Cmb", comboRain2);  cmbRain2->addItem("");

	GetMaterials("weather.particle", "particle_system");
	for (size_t i=0; i < vsMaterials.size(); ++i)
	{	String s = vsMaterials[i];
		cmbRain1->addItem(s);  cmbRain2->addItem(s);
	}	


	//---------------------  TERRAIN  ---------------------
	Cmb(cmbTexDiff, "TexDiffuse", comboTexDiff);
	Cmb(cmbTexNorm, "TexNormal", comboTexNorm);  cmbTexNorm->addItem("flat_n.png");

	strlist li;
	GetFolderIndex(PATHMANAGER::GetDataPath() + "/terrain", li);
	for (strlist::iterator i = li.begin(); i != li.end(); ++i)
		if (StringUtil::endsWith(*i,".txt"))
			i = li.erase(i);

	for (strlist::iterator i = li.begin(); i != li.end(); ++i)
	{
		if (!StringUtil::match(*i, "*_prv.*", false))
		if (StringUtil::match(*i, "*_nh.*", false))
			cmbTexNorm->addItem(*i);
		else
			cmbTexDiff->addItem(*i);
	}
	
	//  particles
	GetMaterials("tires.particle", "particle_system");
	for (size_t i=0; i < vsMaterials.size(); ++i)
	{	String s = vsMaterials[i];
		cmbParDust->addItem(s);  cmbParMud->addItem(s);  cmbParSmoke->addItem(s);
	}
	
	//  surfaces
	for (int i=0; i < TRACKSURFACE::NumTypes; ++i)
		cmbSurfType->addItem(csTRKsurf[i]);

	
	//---------------------  TREES  ---------------------
	Cmb(cmbPgLay, "LTrCombo", comboPgLay);
	strlist lt;
	GetFolderIndex(PATHMANAGER::GetDataPath() + "/trees", lt);
	for (strlist::iterator i = lt.begin(); i != lt.end(); ++i)
		if (StringUtil::endsWith(*i,".mesh"))  cmbPgLay->addItem(*i);

	//---------------------  ROADS  ---------------------
	GetMaterials("road.material");
	for (size_t i=0; i<4; ++i)
	{
		Cmb(cmbRoadMtr[i], "RdMtr"+toStr(i+1), comboRoadMtr);
		Cmb(cmbPipeMtr[i], "RdMtrP"+toStr(i+1), comboPipeMtr);
		if (i>0)  {  cmbRoadMtr[i]->addItem("");  cmbPipeMtr[i]->addItem("");  }
	}
	for (size_t i=0; i < vsMaterials.size(); ++i)
	{	String s = vsMaterials[i];
		if (StringUtil::startsWith(s,"road") && !StringUtil::startsWith(s,"road_") && !StringUtil::endsWith(s,"_ter"))
			for (int i=0; i<4; ++i)  cmbRoadMtr[i]->addItem(s);
		if (StringUtil::startsWith(s,"pipe") && !StringUtil::startsWith(s,"pipe_"))
			for (int i=0; i<4; ++i)  cmbPipeMtr[i]->addItem(s);
	}
	//-----------------------------------------------------


	///  [Track]
	//------------------------------------------------------------------------
	sListTrack = pSet->track;  //! set last
	bListTrackU = pSet->track_user;
	sCopyTrack = "";  //! none
	bCopyTrackU = 0;

	//  list
	trkList = (ListPtr)mWndOpts->findWidget("TrackList");
	TrackListUpd();
	if (trkList)  trkList->eventListChangePosition = newDelegate(this, &App::listTrackChng);
	
	//  text desc
	Edt(trkDesc, "TrackDesc", editTrkDesc);
	trkName = (EditPtr)mWndOpts->findWidget("TrackName");
	if (trkName)  trkName->setCaption(pSet->track);
		
	//  preview images
	imgPrv = (StaticImagePtr)mWndOpts->findWidget("TrackImg");
	imgTer = (StaticImagePtr)mWndOpts->findWidget("TrkTerImg");
	imgMini = (StaticImagePtr)mWndOpts->findWidget("TrackMap");
	listTrackChng(trkList,0);

	//  btn change,  new, rename, delete
	//Btn("ChangeTrack",	btnChgTrack);
	Btn("TrackNew",		btnTrackNew);
	Btn("TrackRename",	btnTrackRename);
	Btn("TrackDelete",	btnTrackDel);

    //  load = new game
    for (int i=1; i<=3; ++i)
    {	Btn("NewGame"+toStr(i), btnNewGame);  }
	
	//  stats text
	for (int i=0; i < StTrk; ++i)
		stTrk[i] = (StaticTextPtr)mWndOpts->findWidget("iv"+toStr(i+1));
}
