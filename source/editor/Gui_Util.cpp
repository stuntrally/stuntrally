#include "stdafx.h"
#include "OgreApp.h"
#include "../road/Road.h"
using namespace MyGUI;


//  Gui from xml (scene, road), after load
//..........................................................................................................

void App::SetGuiFromXmls()
{
	if (!mWndOpts)  return;
	// set slider value, upd text
	HScrollPtr sl;  size_t v;

	#define Slv(name, val)  \
		sl = (HScrollPtr)mWndOpts->findWidget(#name);  \
		v = val*res;  if (sl)  sl->setScrollPosition(v);  sl##name(sl, v);
	
	#define Ed(name, val)  ed##name->setCaption(toStr(val));
	#define Cmb(cmb, str)  cmb->setIndexSelected( cmb->findItemIndexWith(str) );
	

	//  [Sky]
	//-----------------------------------------------
	Cmb(cmbSky, sc.skyMtr );
	Slv(SunPitch, sc.ldPitch /90.f);
	Slv(SunYaw,   (sc.ldYaw + 180.f)  /360.f);
	Ed(LiAmb, sc.lAmb);  Ed(LiDiff, sc.lDiff);  Ed(LiSpec, sc.lSpec);
	Ed(FogClr, sc.fogClr);
	Slv(FogStart, powf(sc.fogStart /2000.f, 0.5f) );
	Slv(FogEnd,   powf(sc.fogEnd   /2000.f, 0.5f) );

	Cmb(cmbRain1, sc.rainName);		Slv(Rain1Rate, sc.rainEmit /6000.f);
	Cmb(cmbRain2, sc.rain2Name);	Slv(Rain2Rate, sc.rain2Emit /6000.f);	
	
	//  [Terrain]
	//-----------------------------------------------
	tabsHmap->setIndexSelected( tabsHmap->findItemIndexWith( toStr(sc.td.iTerSize-1) ));
	tabHmap(0,0);
	if (edTerTriSize)  edTerTriSize->setCaption(toStr(sc.td.fTriangleSize));
	editTerTriSize(edTerTriSize);
	
	tabTerLayer(tabsTerLayers, 0);  // set 1st
	Cmb(cmbParDust, sc.sParDust);	Cmb(cmbParMud,  sc.sParMud);
	Cmb(cmbParSmoke,sc.sParSmoke);

	//  [Vegetation]
	//-----------------------------------------------
	Ed(GrassDens, sc.densGrass);	Ed(TreesDens, sc.densTrees);
	Ed(GrPage, sc.grPage);		Ed(GrDist, sc.grDist);
	Ed(TrPage, sc.trPage);		Ed(TrDist, sc.trDist);
	Ed(GrMinX, sc.grMinSx);		Ed(GrMaxX, sc.grMaxSx);
	Ed(GrMinY, sc.grMinSy);		Ed(GrMaxY, sc.grMaxSy);
	Ed(GrSwayDistr, sc.grSwayDistr);
	Ed(GrSwayLen, sc.grSwayLen);	Ed(GrSwaySpd, sc.grSwaySpeed);
	Ed(TrRdDist, sc.trRdDist);		Ed(TrImpDist, sc.trDistImp);
	tabPgLayers(tabsPgLayers, 0);
	//MeshPtr mp = MeshManager::load(sc.pgLayersAll[0].name);
	//mp->getSubMesh(0)->

	//  [Road]
	//-----------------------------------------------
	for (int i=0; i < 4/*MTRs*/; ++i)
	{	Cmb(cmbRoadMtr[i], road->sMtrRoad[i]);
		Cmb(cmbPipeMtr[i], road->sMtrPipe[i]);  }

	Ed(RdTcMul,		road->tcMul);		Ed(RdColN, road->colN);
	Ed(RdLenDim,	road->lenDiv0);		Ed(RdColR, road->colR);
	Ed(RdWidthSteps,road->iw0);			Ed(RdPwsM, road->iwPmul);
	Ed(RdHeightOfs,	road->fHeight);		Ed(RdPlsM, road->ilPmul);
	Ed(RdSkirtLen,	road->skLen);		Ed(RdSkirtH,	road->skH);
	Ed(RdMergeLen,	road->setMrgLen);	Ed(RdLodPLen,	road->lposLen);
}


//  tracks list	 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
//-----------------------------------------------------------------------------------------------------------

void App::TrackListUpd()
{
	///  tracks list, text, chg btn
	//------------------------------------
	if (trkList)
	{	trkList->removeAllItems();
		int ii = 0, si = 0;  bool bFound = false;
		vsTracks.clear();

		strlist li;
		GetFolderIndex(pathTrk, li);
		for (strlist::iterator i = li.begin(); i != li.end(); ++i)
		{
			vsTracks.push_back(*i);
			string s = pathTrk + *i + "/scene.xml";
			ifstream check(s.c_str());
			if (check)  {
				trkList->addItem(*i);
				if (*i == pSet->track)  {  si = ii;
					trkList->setIndexSelected(si);
					bFound = true;  }
				ii++;  }
		}
		//  not found last track, set 1st
		if (!bFound)
		{	pSet->track = *li.begin();  UpdWndTitle();  }
		trkList->beginToItemAt(max(0, si-11));  // center
	}
}


void App::listTrackChng(List* li, size_t pos)
{
	if (!li)  return;
	size_t i = li->getIndexSelected();  if (i==ITEM_NONE)  return;
	const UString& sl = li->getItemNameAt(i);	sListTrack = sl;
	
	//  won't refresh if same-...
	if (imgPrv)  imgPrv->setImageTexture(sListTrack+".jpg");
	if (imgTer)  imgTer->setImageTexture(sListTrack+"_ter.jpg");
	if (imgMini)  imgMini->setImageTexture(sListTrack+"_mini.png");
	ReadTrkStats();
}

void App::btnChgTrack(WP)
{
	if (trkName)  trkName->setCaption(sListTrack.c_str());
	pSet->track = sListTrack;  //UpdWndTitle();//? load
}

void App::btnNewGame(WP)
{
	LoadTrack();
}



///  tools  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
/// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

//  track files
const int cnTrkF = 5, cnTrkFd = 2, cnTrkFo = 3;
const String csTrkFo[cnTrkFo] = {"/grass1.png", "/grassColor.png", "/grassDensity.png"},
	csTrkF[cnTrkF] = {"/heightmap.f32", "/road.xml", "/scene.xml", "/surfaces.txt", "/track.txt"},  // copy, new
	csTrkFd[cnTrkFd] = {"/heightmap-new.f32", "/records.txt"};  // del


void App::btnTrkCopySel(WP)  // set copy source
{
	sTrackCopy = sListTrack;
	if (valTrkCpySel)  valTrkCpySel->setCaption(sTrackCopy);
}

bool App::ChkTrkCopy()
{
	if (sTrackCopy == "")  // none
	{
		Message::createMessageBox(
			"Message", "Copy Track", "No source track selected.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return false;
	}
	if (sTrackCopy == pSet->track)
	{
		Message::createMessageBox(
			"Message", "Copy Track", "Source track and current track are the same.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return false;
	}
	return true;
}

///  copy Hmap
void App::btnCopyTerHmap(WP)
{
	if (!ChkTrkCopy())  return;

	String from = pathTrk + sTrackCopy,
		name = TrkDir() + "heightmap-new.f32";
	Copy(from + "/heightmap.f32", name);
	
	Scene sF;  sF.LoadXml(from + "/scene.xml");
	sc.td.iVertsX = sF.td.iTerSize;  //  ter sizes
	sc.td.fTriangleSize = sF.td.fTriangleSize;
	sc.td.UpdVals();
	bNewHmap = true;
	SetGuiFromXmls();	UpdateTrack();
	if (road)  road->UpdAllMarkers();
}

//  copy sun, etc.
void App::btnCopySun(WP)
{
	if (!ChkTrkCopy())  return;
	String from = pathTrk + sTrackCopy;
	Scene sF;  sF.LoadXml(from + "/scene.xml");

	sc.skyMtr = sF.skyMtr;  // sky
	sc.rainEmit = sF.rainEmit;  sc.rain2Emit = sF.rain2Emit;
	sc.rainName = sF.rainName;  sc.rain2Name = sF.rain2Name;
	sc.fogMode = sF.fogMode;  sc.fogExp = sF.fogExp;  // fog
	sc.fogClr = sF.fogClr;  sc.fogStart = sF.fogStart;  sc.fogEnd = sF.fogEnd;
	sc.ldPitch = sF.ldPitch;  sc.ldYaw = sF.ldYaw;  // light
	sc.lAmb = sF.lAmb;  sc.lDiff = sF.lDiff;  sc.lSpec = sF.lSpec;
	SetGuiFromXmls();	UpdateTrack();
}

//  copy ter layers
void App::btnCopyTerLayers(WP)
{
	if (!ChkTrkCopy())  return;
	String from = pathTrk + sTrackCopy;
	Scene sF;  sF.LoadXml(from + "/scene.xml");

	for (int i=0; i < sc.td.ciNumLay; ++i)
		sc.td.layersAll[i] = sF.td.layersAll[i];
	sc.sParDust = sF.sParDust;  sc.sParMud = sF.sParMud;
	sc.sParSmoke = sF.sParSmoke;
	sc.td.UpdLayers();

	//  copy surfaces.txt
	String sto = from + "/objects";  // from
	String to = TrkDir() + "objects";  // to, new
	for (int i=0; i < 2; ++i)  Copy(sto + csTrkFo[i], to + csTrkFo[i]);

	SetGuiFromXmls();	UpdateTrack();
}

//  copy paged layers
void App::btnCopyVeget(WP)
{
	if (!ChkTrkCopy())  return;
	String from = pathTrk + sTrackCopy;
	Scene sF;  sF.LoadXml(from + "/scene.xml");

	sc.densGrass = sF.densGrass;  sc.densTrees = sF.densTrees;
	sc.grPage = sF.grPage;  sc.grDist = sF.grDist;
	sc.trPage = sF.trPage;  sc.trDist = sF.trDist;
	sc.grMinSx = sF.grMinSx;  sc.grMaxSx = sF.grMaxSx;
	sc.grMinSy = sF.grMinSy;  sc.grMaxSy = sF.grMaxSy;
	sc.grSwayDistr = sF.grSwayDistr;  sc.grSwayLen = sF.grSwayLen;
	sc.grSwaySpeed = sF.grSwaySpeed;
	sc.trRdDist = sF.trRdDist;  sc.trDistImp = sF.trDistImp;

	for (int i=0; i < sc.ciNumPgLay; ++i)
		sc.pgLayersAll[i] = sF.pgLayersAll[i];

	//  copy grass1.png grassColor.png
	String sto = from + "/objects";  // from
	String to = TrkDir() + "objects";  // to, new
	for (int i=0; i < 2; ++i)  Copy(sto + csTrkFo[i], to + csTrkFo[i]);

	SetGuiFromXmls();	UpdateTrack();
}

//  copy road
void App::btnCopyRoad(WP)
{
	if (!ChkTrkCopy() || !road)  return;
	String from = pathTrk + sTrackCopy;
	road->LoadFile(from + "/road.xml");

	SetGuiFromXmls();	road->RebuildRoad(true);
	UpdPSSMMaterials();	road->UpdAllMarkers();
}

//  copy road pars
void App::btnCopyRoadPars(WP)
{
	if (!ChkTrkCopy() || !road)  return;
	String from = pathTrk + sTrackCopy;
	SplineRoad rd;  rd.LoadFile(from + "/road.xml",false);

	for (int i=0; i < MTRs; ++i)
	{	road->sMtrRoad[i] = rd.sMtrRoad[i];
		road->sMtrPipe[i] = rd.sMtrPipe[i];  }

	road->tcMul = rd.tcMul;		road->colN = rd.colN;
	road->lenDiv0 = rd.lenDiv0;	road->colR = rd.colR;
	road->iw0 =	rd.iw0;			road->iwPmul = rd.iwPmul;
	road->fHeight =	rd.fHeight;	road->ilPmul = rd.ilPmul;
	road->skLen = rd.skLen;		road->skH = rd.skH;
	road->setMrgLen = rd.setMrgLen;  road->lposLen = rd.lposLen;

	SetGuiFromXmls();	road->RebuildRoad(true);
	UpdPSSMMaterials();	road->UpdAllMarkers();
}


///  track 	. . . . . . . . . . . . . . . . . . . .	. . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void App::btnDeleteRoad(WP)
{
	int l = road->getNumPoints();
	for (int i=0; i < l; ++i)
	{
		road->iChosen = road->getNumPoints()-1;
		road->Delete();
	}
	//road->RebuildRoad(true);
}

void App::btnScaleAll(WP)
{
	if (!edScaleAllMul || !road)  return;
	Real sf = max(0.1f, s2r(edScaleAllMul->getCaption()) );  // scale mul
	
	//  road
	for (int i=0; i < road->getNumPoints(); ++i)
	{
		road->Scale1(i, sf - 1.f);
		road->mP[i].width *= sf;
	}
	road->bSelChng = true;

	//  ter
	sc.td.fTriangleSize *= sf;  sc.td.UpdVals();
	
	SetGuiFromXmls();	UpdateTrack();
	
	//  road upd mrk--
	//for (int i=0; i < road->getNumPoints(); ++i)
		//road->Move1(i, Vector3::ZERO);
	//road->RebuildRoad(true);

	//road->UpdAllMarkers();  //!?

	//start Pos
	const int n = 0;  // 1st entry - all same / edit 4..
	vStartPos[n][0] *= sf;
	vStartPos[n][1] *= sf;  UpdStartPos();
}

void App::editScaleAllMul(EditPtr)
{	}  // not used



///  track 	. . . . . . . . . . . . . . . . . . . .	. . . . . . . . . . . . . . . . . . . . . . . . . . . . .
//-----------------------------------------------------------------------------------------------------------

///  New (duplicate)
void App::btnTrackNew(WP)
{
	String name = trkName->getCaption();
	if (TrackExists(name))  {	Message::createMessageBox(
			"Message", "New Track", "Track " + name + " already exists.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return;  }

	String st = pathTrk + sListTrack, sto = st + "/objects";  // from
	String t = pathTrk + name, to = t + "/objects";  // to, new

	CreateDir(t);  CreateDir(to);
	for (int i=0; i < cnTrkFo; ++i)  Copy(sto + csTrkFo[i], to + csTrkFo[i]);
	for (int i=0; i < cnTrkF; ++i)   Copy(st + csTrkF[i], t + csTrkF[i]);
	//Copy(pathTrkPrv + sListTrack + ".jpg");  // no preview
	Copy(pathTrkPrv + sListTrack + "_mini.png", pathTrkPrv + name + "_mini.png");
	Copy(pathTrkPrv + sListTrack + "_ter.jpg", pathTrkPrv + name + "_ter.jpg");

	sListTrack = name;  pSet->track = name;  UpdWndTitle();
	TrackListUpd();
}

///  Rename
void App::btnTrackRename(WP)
{
	String name = trkName->getCaption();
	if (name != sListTrack)
	{	if (TrackExists(name))  {	Message::createMessageBox(
				"Message", "Rename Track", "Track " + name + " already exists.",
				MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
			return;  }
		
		//  track dir
		Rename(pathTrk + sListTrack, pathTrk + name);
		//  preview shot, minimap
		Rename(pathTrkPrv + sListTrack + ".jpg", pathTrkPrv + name + ".jpg");
		Rename(pathTrkPrv + sListTrack + "_mini.png", pathTrkPrv + name + "_mini.png");
		Rename(pathTrkPrv + sListTrack + "_ter.jpg", pathTrkPrv + name + "_ter.jpg");
		
		sListTrack = name;  pSet->track = sListTrack;  UpdWndTitle();
		TrackListUpd();  //listTrackChng(trkList,0);
	}
}

///  Delete
void App::btnTrackDel(WP)
{
	Message* message = Message::createMessageBox(
		"Message", "Delete Track ?", sListTrack,
		MessageBoxStyle::IconQuest | MessageBoxStyle::Yes | MessageBoxStyle::No);
	message->eventMessageBoxResult = newDelegate(this, &App::msgTrackDel);
	//message->setUserString("FileName", fileName);
}
void App::msgTrackDel(Message* sender, MessageBoxStyle result)
{
	if (result != MessageBoxStyle::Yes)
		return;
	String t = pathTrk + sListTrack, to = t + "/objects";
	for (int i=0; i < cnTrkFo; ++i)  Delete(to + csTrkFo[i]);
	for (int i=0; i < cnTrkF; ++i)   Delete(t + csTrkF[i]);
	for (int i=0; i < cnTrkFd; ++i)   Delete(t + csTrkFd[i]);
	DeleteDir(to);  DeleteDir(t);
	Delete(pathTrkPrv + sListTrack + ".jpg");
	Delete(pathTrkPrv + sListTrack + "_mini.png");
	Delete(pathTrkPrv + sListTrack + "_ter.jpg");

	String st = pSet->track;
	TrackListUpd();
	if (st != pSet->track)
		LoadTrack();  //load 1st if deleted cur
}



//-----------------------------------------------------------------------------------------------------------
//  Surfaces
//-----------------------------------------------------------------------------------------------------------

void App::LoadSurf(const String& trk)
{
	string path = pathTrk + trk + "/surfaces.txt";
	CONFIGFILE cf;
	if (!cf.Load(path))
	{	Log("Can't find surfaces configfile: " + path);  return;  }
	
	strlist sl;
	cf.GetSectionList(sl);
		
	int si = 0;
	for (strlist::const_iterator s = sl.begin(); s != sl.end(); ++s)
	{
		TRACKSURFACE surf;
		surf.name = *s;
		
		int id;
		cf.GetParam(*s + ".ID", id);	surf.setType(id);
		float f = 0.0;
		cf.GetParam(*s + ".BumpWaveLength", f);		surf.bumpWaveLength = f;
		cf.GetParam(*s + ".BumpAmplitude", f);		surf.bumpAmplitude = f;
		cf.GetParam(*s + ".FrictionNonTread", f);	surf.frictionNonTread = f;
		cf.GetParam(*s + ".FrictionTread", f);		surf.frictionTread = f;
		cf.GetParam(*s + ".RollResistanceCoefficient", f);  surf.rollResistanceCoefficient = f;
		cf.GetParam(*s + ".RollingDrag", f);		surf.rollingDrag = f;
		
		if (StringUtil::startsWith(surf.name, "l_"))
		{
			int l = surf.name[2]-'0';  // saved ter layers by editor, all + road
			if (l < 8)
				su[l] = surf;
		}else
		{
			int l = surf.name[0]-'B';  // A is road, B-F ter layers - used only
			if (l < 0)  su[6] = surf;
			else  su[l] = surf;
		}
	}
}

void App::SaveSurf(const String& trk)
{
	string path = pathTrk + trk + "/surfaces.txt";
	CONFIGFILE cf;
	
	int u=0;
	for (int i=0; i < 7; ++i)  // 6 ter layers + road in [6]
	{
		int n = 1;  // not used
		if (i==6 || (i < 6 && sc.td.layersAll[i].on))
			n = 2;  // used - write twice
		
		//  1st A-F sorted for game, 2nd L_ - read by editor
		for (int j=0; j < n; ++j)
		{
			if (j==0)	sprintf(s, "L_%d", i);  // editor
			else  if (i==6){  s[0] = 'A';  s[1]=0;  }  // used
				  else  {  s[0] = u+'B';  s[1]=0;  u++;  }  // used

			const TRACKSURFACE& surf = su[i];  string ss = s;
			cf.SetParam(ss + ".ID", surf.type);
			cf.SetParam(ss + ".BumpWaveLength", surf.bumpWaveLength);
			cf.SetParam(ss + ".BumpAmplitude", surf.bumpAmplitude);
			cf.SetParam(ss + ".FrictionNonTread", surf.frictionNonTread);
			cf.SetParam(ss + ".FrictionTread", surf.frictionTread);
			cf.SetParam(ss + ".RollResistanceCoefficient", surf.rollResistanceCoefficient);
			cf.SetParam(ss + ".RollingDrag", surf.rollingDrag);
		}
	}
	cf.Write(true, path);
}



///  . .  util tracks stats  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 

void App::ReadTrkStats()
{
	String sRd = pathTrk + sListTrack + "/road.xml";
	String sSc = pathTrk + sListTrack + "/scene.xml";

	SplineRoad rd;  rd.LoadFile(sRd,false);  // load
	Scene sc;  sc.LoadXml(sSc);  // fails to defaults
	UpdGuiRdStats(&rd,sc, 0.f);
}

void App::UpdGuiRdStats(const SplineRoad* rd, const Scene& sc, float time)
{
	Fmt(s, "%5.3f km", sc.td.fTerWorldSize / 1000.f);	if (stTrk[1])  stTrk[1]->setCaption(s);
	if (!rd)  return;
	Fmt(s, "%5.3f km", rd->st.Length / 1000.f);			if (stTrk[0])  stTrk[0]->setCaption(s);

	Fmt(s, "%4.2f m", rd->st.WidthAvg);		if (stTrk[2])  stTrk[2]->setCaption(s);
	Fmt(s, "%3.1f m", rd->st.HeightDiff);	if (stTrk[3])  stTrk[3]->setCaption(s);

	Fmt(s, "%3.1f%%", rd->st.OnTer);	if (stTrk[4])  stTrk[4]->setCaption(s);
	Fmt(s, "%3.1f%%", rd->st.Pipes);	if (stTrk[5])  stTrk[5]->setCaption(s);

	//Fmt(s, "%4.2f%%", rd->st.Yaw);	if (stTrk[6])  stTrk[6]->setCaption(s);
	//Fmt(s, "%4.2f%%", rd->st.Pitch);	if (stTrk[7])  stTrk[7]->setCaption(s);
	//Fmt(s, "%4.2f%%", rd->st.Roll);	if (stTrk[8])  stTrk[8]->setCaption(s);
	
	if (trkName)  //?.
		trkName->setCaption(sListTrack.c_str());
	if (trkDesc)  // desc
		trkDesc->setCaption(rd->sTxtDesc.c_str());
}


///  Gui ToolTips
//-----------------------------------------------------------------------------------------------------------

void App::setToolTips(EnumeratorWidgetPtr widgets)
{
    while (widgets.next())
    {
        WidgetPtr wp = widgets.current();
        bool tip = wp->isUserString("tip");
		if (tip)  // if has tooltip string
		{	wp->setNeedToolTip(true);
			wp->eventToolTip = newDelegate(this, &App::notifyToolTip);
		}
		//Log(wp->getName() + (tip ? "  *" : ""));
        setToolTips(wp->getEnumerator());
    }
}

void App::notifyToolTip(Widget *sender, const ToolTipInfo &info)
{
	if (info.type == ToolTipInfo::Show)
	{
		mToolTip->setSize(320, 96);  // start size for wrap
		mToolTipTxt->setCaption(sender->getUserString("tip"));
		const IntSize &textsize = mToolTipTxt->getTextSize();
		mToolTip->setSize(textsize.width +  6, textsize.height + 6);
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
	/*const IntSize& view_size = moving->getParentSize();
	if ((p.left + size.width) > view_size.width)
		p.left -= offset.left + offset.left + size.width;

	if ((p.top + size.height) > view_size.height)
		p.top -= offset.top + offset.top + size.height;
	}/**/
	moving->setPosition(p);
}


///  Get Materials
//-----------------------------------------------------------------------------------------------------------

void App::GetMaterials(String filename, String type)
{
	vsMaterials.clear();
	DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource(filename);
	if(!stream.isNull())
	{	try
		{	while(!stream->eof())
			{
				std::string line = stream->getLine();
				StringUtil::trim(line);
 
				if (StringUtil::startsWith(line, type/*"material"*/))
				{
					//Log(line);
					Ogre::vector<String>::type vec = StringUtil::split(line," \t:");
					bool skipFirst = true;
					for (Ogre::vector<String>::type::iterator it = vec.begin(); it < vec.end(); ++it)
					{
						if (skipFirst)
						{	skipFirst = false;
							continue;	}
						
						std::string match = (*it);
						StringUtil::trim(match);
						if (!match.empty())
						{
							//Log(match);
							vsMaterials.push_back(match);						
							break;
						}
					}
			}	}
		}catch (Ogre::Exception &e)
		{
			StringUtil::StrStreamType msg;
			msg << "Exception: FILE: " << __FILE__ << " LINE: " << __LINE__ << " DESC: " << e.getFullDescription() << std::endl;
			Log(msg.str());
	}	}
	stream->close();
}


///  system file, dir
//-----------------------------------------------------------------------------------------------------------

void App::Rename(String from, String to)
{
	#ifdef WIN32	
	if (MoveFileA(from.c_str(), to.c_str()) == 0)
	{
		DWORD e = GetLastError()&0xFFFF;
		Log("Can't Rename! GetLastError = " + toStr(e));
		//p(s)"%s\nto:\n%s\nError: %d %s", old, name, e,
		//	e==ERROR_SHARING_VIOLATION? "file is opened":
		//if (e==ERROR_FILE_NOT_FOUND || e==ERROR_PATH_NOT_FOUND)  t->dis = 1;
		//Info(s,"Can't rename file");
	}
	#endif
}

bool App::TrackExists(String name)
{
	for (size_t i=0; i < vsTracks.size(); ++i)
		if (vsTracks[i] == name)
			return true;
	return false;
}

void App::Delete(String file)
{
	#ifdef WIN32
	DeleteFileA(file.c_str());
	#endif
}

void App::DeleteDir(String dir)
{
	#ifdef WIN32
	RemoveDirectory(dir.c_str());
	#endif
}

void App::CreateDir(String dir)
{
	#ifdef WIN32
	CreateDirectoryA(dir.c_str(),0);
	#endif
}

void App::Copy(String file, String to)
{
	#ifdef WIN32
	CopyFileA(file.c_str(), to.c_str(), 0);
	#endif
}
