#include "pch.h"
#include "../ogre/common/Defines.h"
#include "OgreApp.h"
#include "../road/Road.h"
#include <boost/filesystem.hpp>
#include "../ogre/common/Gui_Def.h"
#include "../ogre/common/Slider.h"
using namespace MyGUI;
using namespace Ogre;


//  Gui from xml (scene, road), after load
//..........................................................................................................

void App::SetGuiFromXmls()
{
	if (!mWndEdit)  return;
	bGI = false;
	// set slider value, upd text
	Slider* sl;

	#define _Slv(name, val)  \
		sl = (Slider*)mWndEdit->findWidget(#name);  \
		if (sl)  sl->setValue(val);  sl##name(sl, val);
	
	#define _Ed(name, val)  ed##name->setCaption(toStr(val));
	#define _Clr(name, val)  clr##name->setColour(Colour(val.x,val.y,val.z));
	#define _Cmb(cmb, str)  cmb->setIndexSelected( cmb->findItemIndexWith(str) );
	

	//  [Sky]
	//-----------------------------------------------
	_Cmb(cmbSky, sc->skyMtr);
	_Slv(SunPitch, sc->ldPitch /90.f);
	_Slv(SunYaw,   (sc->ldYaw + 180.f) /360.f);
	_Ed(LiAmb, sc->lAmb);  _Ed(LiDiff, sc->lDiff);  _Ed(LiSpec, sc->lSpec);
	_Clr(Amb, sc->lAmb);  _Clr(Diff, sc->lDiff);  _Clr(Spec, sc->lSpec);
	//  fog
	_Clr(Fog, sc->fogClr);  _Clr(Fog2, sc->fogClr2);  _Clr(FogH, sc->fogClrH);
	_Ed(FogClr, sc->fogClr);  _Ed(FogClr2, sc->fogClr2);  _Ed(FogClrH, sc->fogClrH);
	_Slv(FogStart, powf(sc->fogStart /2000.f, 0.5f));  _Slv(FogEnd,   powf(sc->fogEnd   /2000.f, 0.5f));
	_Slv(FogHStart,powf(sc->fogHStart/2000.f, 0.5f));
	_Slv(FogHeight, (sc->fogHeight +200.f) /400.f );
	_Slv(FogHEnd,  powf(sc->fogHEnd  /2000.f, 0.5f));  _Slv(FogHDensity,powf(sc->fogHDensity /200.f, 1/ 2.f));

	_Cmb(cmbRain1, sc->rainName);	_Slv(Rain1Rate, sc->rainEmit /6000.f);
	_Cmb(cmbRain2, sc->rain2Name);	_Slv(Rain2Rate, sc->rain2Emit /6000.f);	
	
	//  [Terrain]
	//-----------------------------------------------
	static std::map<int,int> hs;
	if (hs.empty()) {  hs[128]=0; hs[256]=1; hs[512]=2; hs[1024]=3; hs[2048]=4;  }
	tabsHmap->setIndexSelected( hs[sc->td.iTerSize-1] );
	tabHmap(0,0);
	if (edTerTriSize)  edTerTriSize->setCaption(toStr(sc->td.fTriangleSize));
	editTerTriSize(edTerTriSize);
	if (edTerErrorNorm)  edTerErrorNorm->setCaption(fToStr(sc->td.errorNorm,2,4));
	
	tabTerLayer(tabsTerLayers, 0);  // set 1st
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
	for (int i=0; i < 4/*MTRs*/; ++i)
	{	_Cmb(cmbRoadMtr[i], road->sMtrRoad[i]);
		_Cmb(cmbPipeMtr[i], road->sMtrPipe[i]);  }
	_Cmb(cmbRoadWMtr, road->sMtrWall);  _Cmb(cmbRoadColMtr, road->sMtrCol);
	_Cmb(cmbPipeWMtr, road->sMtrWallPipe);

	_Ed(RdTcMul, road->tcMul);  _Ed(RdTcMulW, road->tcMul);
	_Ed(RdTcMulP, road->tcMul); _Ed(RdTcMulPW, road->tcMul);  _Ed(RdTcMulC, road->tcMul);
	_Ed(RdColN, road->colN);		_Ed(RdColR, road->colR);
	_Ed(RdLenDim, road->lenDiv0);	_Ed(RdWidthSteps,road->iw0);
	_Ed(RdPwsM, road->iwPmul);		_Ed(RdPlsM, road->ilPmul);
	_Ed(RdHeightOfs,road->fHeight);
	_Ed(RdSkirtLen,	road->skirtLen);	_Ed(RdSkirtH,	road->skirtH);
	_Ed(RdMergeLen,	road->setMrgLen);	_Ed(RdLodPLen,	road->lposLen);
	bGI = true;
}


void App::btnNewGame(WP)
{
	if (trkName)  trkName->setCaption(sListTrack.c_str());
	pSet->gui.track = sListTrack;
	pSet->gui.track_user = bListTrackU;  //UpdWndTitle();//? load
	LoadTrack();
}



///  tools  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
/// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

//  track files
const int cnTrkFm = 4, cnTrkFd = 2, cnTrkFo = 2, cnTrkFp = 3;
const Ogre::String
	csTrkFm[cnTrkFm] = {"/heightmap.f32", "/road.xml", "/scene.xml", "/track.txt"},  // copy, new
	csTrkFo[cnTrkFo] = {"/grassDensity.png", "/waterDepth.png"},
	csTrkFp[cnTrkFp] = {"/view.jpg", "/road.png", "/terrain.jpg"},
	csTrkFd[cnTrkFd] = {"/heightmap-new.f32", "/records.txt"};  // del


void App::btnTrkCopySel(WP)  // set copy source
{
	sCopyTrack = sListTrack;
	bCopyTrackU = bListTrackU;
	if (valTrkCpySel)  valTrkCpySel->setCaption(sCopyTrack);
}

bool App::ChkTrkCopy()
{
	if (sCopyTrack == "")  // none
	{
		Message::createMessageBox(
			"Message", "Copy Track", "No source track selected.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return false;
	}
	if (sCopyTrack == pSet->gui.track && bCopyTrackU == (pSet->gui.track_user ? 1 : 0))
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

	String from = PathCopyTrk(),
		name = TrkDir() + "heightmap-new.f32";
	Copy(from + "/heightmap.f32", name);
	
	Scene sF;  sF.LoadXml(from + "/scene.xml");
	sc->td.iVertsX = sF.td.iTerSize;  //  ter sizes
	sc->td.fTriangleSize = sF.td.fTriangleSize;
	sc->td.UpdVals();
	bNewHmap = true;
	SetGuiFromXmls();	UpdateTrack();
	if (road)  road->UpdAllMarkers();
}

//  copy sun, etc.
void App::btnCopySun(WP)
{
	if (!ChkTrkCopy())  return;
	String from = PathCopyTrk();
	Scene sF;  sF.LoadXml(from + "/scene.xml");

	sc->skyMtr = sF.skyMtr;  // sky
	sc->rainEmit = sF.rainEmit;  sc->rain2Emit = sF.rain2Emit;
	sc->rainName = sF.rainName;  sc->rain2Name = sF.rain2Name;

	sc->fogClr = sF.fogClr;  sc->fogClr2 = sF.fogClr2;  sc->fogClrH = sF.fogClrH;
	sc->fogStart = sF.fogStart;  sc->fogEnd = sF.fogEnd;
	sc->fogHStart = sF.fogHStart;  sc->fogHEnd = sF.fogHEnd;
	sc->fogHDensity = sF.fogHDensity;  sc->fogHeight = sF.fogHeight;

	sc->ldPitch = sF.ldPitch;  sc->ldYaw = sF.ldYaw;  // light
	sc->lAmb = sF.lAmb;  sc->lDiff = sF.lDiff;  sc->lSpec = sF.lSpec;
	SetGuiFromXmls();	UpdateTrack();
	DestroyWeather();  CreateWeather();
}

//  copy ter layers
void App::btnCopyTerLayers(WP)
{
	if (!ChkTrkCopy())  return;
	String from = PathCopyTrk();
	Scene sF;  sF.LoadXml(from + "/scene.xml");

	for (int i=0; i < sc->td.ciNumLay; ++i)
		sc->td.layersAll[i] = sF.td.layersAll[i];
	sc->sParDust = sF.sParDust;  sc->sParMud = sF.sParMud;
	sc->sParSmoke = sF.sParSmoke;
	sc->td.UpdLayers();

	//  copy grass dens only
	String sto = from + "/objects";  // from
	String to = TrkDir() + "objects";  // to, new
	Copy(sto + csTrkFo[0], to + csTrkFo[0]);

	SetGuiFromXmls();	UpdateTrack();
}

//  copy paged layers
void App::btnCopyVeget(WP)
{
	if (!ChkTrkCopy())  return;
	String from = PathCopyTrk();
	Scene sF;  sF.LoadXml(from + "/scene.xml");

	sc->densGrass = sF.densGrass;  sc->densTrees = sF.densTrees;
	sc->trPage = sF.trPage;  sc->trDist = sF.trDist;
	sc->trRdDist = sF.trRdDist;  sc->trDistImp = sF.trDistImp;

	sc->grPage = sF.grPage;  sc->grDist = sF.grDist;

	for (int i=0; i < sc->ciNumGrLay; ++i)
		sc->grLayersAll[i] = sF.grLayersAll[i];

	for (int i=0; i < sc->ciNumPgLay; ++i)
		sc->pgLayersAll[i] = sF.pgLayersAll[i];

	SetGuiFromXmls();	UpdateTrack();
}

//  copy road
void App::btnCopyRoad(WP)
{
	if (!ChkTrkCopy() || !road)  return;
	String from = PathCopyTrk();
	road->LoadFile(from + "/road.xml");

	SetGuiFromXmls();	road->RebuildRoad(true);
	UpdPSSMMaterials();	road->UpdAllMarkers();
}

//  copy road pars
void App::btnCopyRoadPars(WP)
{
	if (!ChkTrkCopy() || !road)  return;
	String from = PathCopyTrk();
	SplineRoad rd(this);  rd.LoadFile(from + "/road.xml",false);

	for (int i=0; i < MTRs; ++i)
	{	road->sMtrRoad[i] = rd.sMtrRoad[i];
		road->SetMtrPipe(i, rd.sMtrPipe[i]);  }

	road->tcMul = rd.tcMul;		road->colN = rd.colN;
	road->lenDiv0 = rd.lenDiv0;	road->colR = rd.colR;
	road->iw0 =	rd.iw0;			road->iwPmul = rd.iwPmul;
	road->fHeight =	rd.fHeight;	road->ilPmul = rd.ilPmul;
	road->skirtLen = rd.skirtLen;	road->skirtH = rd.skirtH;
	road->setMrgLen = rd.setMrgLen;  road->lposLen = rd.lposLen;

	SetGuiFromXmls();	road->RebuildRoad(true);
	UpdPSSMMaterials();	road->UpdAllMarkers();
}


///  tools 	. . . . . . . . . . . . . . . . . . . .	. . . . . . . . . . . . . . . . . . . . . . . . . . . . .

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
void App::btnDeleteFluids(WP)
{
	sc->fluids.clear();
	bRecreateFluids = true;
}
void App::btnDeleteObjects(WP)
{
	DestroyObjects(true);
	iObjCur = -1;
}

void App::btnScaleAll(WP)
{
	if (!edScaleAllMul || !road)  return;
	Real sf = std::max(0.1f, s2r(edScaleAllMul->getCaption()) );  // scale mul
	
	//  road
	for (int i=0; i < road->getNumPoints(); ++i)
	{
		road->Scale1(i, sf - 1.f);
		road->mP[i].width *= sf;
	}
	road->bSelChng = true;
	
	//  fluids
	for (int i=0; i < sc->fluids.size(); ++i)
	{
		FluidBox& fb = sc->fluids[i];
		fb.pos.x *= sf;  fb.pos.z *= sf;
		fb.size.x *= sf;  fb.size.z *= sf;
	}

	//  ter
	sc->td.fTriangleSize *= sf;  sc->td.UpdVals();
	
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
{	}

void App::editScaleTerHMul(EditPtr)
{	}



///  track 	. . . . . . . . . . . . . . . . . . . .	. . . . . . . . . . . . . . . . . . . . . . . . . . . . .
//-----------------------------------------------------------------------------------------------------------

///  New (duplicate)
void App::btnTrackNew(WP)
{
	String name = trkName->getCaption();
	name = StringUtil::replaceAll(name, "*", "");

	if (TrackExists(name))  {
		Message::createMessageBox(
			"Message", "New Track", "Track " + name + " already exists.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return;  }

	String st = pathTrk[bListTrackU] + sListTrack, t = pathTrk[1] + name,
		sto = st + "/objects", stp = st + "/preview",  // from
		to = t + "/objects",   tp = t + "/preview";  // to,new

	//  Copy
	CreateDir(t);  CreateDir(to);  CreateDir(tp);
	int i;
	for (i=0; i < cnTrkFm; ++i)  Copy(st + csTrkFm[i], t + csTrkFm[i]);
	for (i=0; i < cnTrkFo; ++i)  Copy(sto + csTrkFo[i], to + csTrkFo[i]);
	for (i=1; i < cnTrkFp; ++i)  Copy(stp + csTrkFp[i], tp + csTrkFp[i]);  // 1-not view.jpg

	sListTrack = name;  pSet->gui.track = name;  pSet->gui.track_user = 1;  UpdWndTitle();
	FillTrackLists();
	TrackListUpd();
}

///  Rename
void App::btnTrackRename(WP)
{
	String name = trkName->getCaption();
	if (name == sListTrack)  return;

	/*if (bListTrackU==0)  {  // could force when originals writable..
		Message::createMessageBox(
			"Message", "Rename Track", "Track " + name + " is original and can't be renamed.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
			return;  }/**/

	if (TrackExists(name))  {
		Message::createMessageBox(
			"Message", "Rename Track", "Track " + name + " already exists.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return;  }
	
	//  Rename
	Rename(pathTrk[bListTrackU] + sListTrack, pathTrk[/*1*/bListTrackU] + name);
	
	sListTrack = name;  pSet->gui.track = sListTrack;  pSet->gui.track_user = 1;/**/  UpdWndTitle();
	FillTrackLists();
	TrackListUpd();  //listTrackChng(trkList,0);
}

///  Delete
void App::btnTrackDel(WP)
{
	Message* message = Message::createMessageBox(
		"Message", bListTrackU==0 ? "Delete original Track ?" : "Delete Track ?", sListTrack,
		MessageBoxStyle::IconQuest | MessageBoxStyle::Yes | MessageBoxStyle::No);
	message->eventMessageBoxResult += newDelegate(this, &App::msgTrackDel);
	//message->setUserString("FileName", fileName);
}
void App::msgTrackDel(Message* sender, MessageBoxStyle result)
{
	if (result != MessageBoxStyle::Yes)
		return;
	String t = pathTrk[bListTrackU] + sListTrack,
		to = t + "/objects", tp = t + "/preview";
	int i;
	for (i=0; i < cnTrkFo; ++i)  Delete(to + csTrkFo[i]);
	for (i=0; i < cnTrkFp; ++i)  Delete(tp + csTrkFp[i]);
	for (i=0; i < cnTrkFm; ++i)  Delete(t + csTrkFm[i]);
	for (i=0; i < cnTrkFd; ++i)  Delete(t + csTrkFd[i]);
	DeleteDir(to);  DeleteDir(tp);  DeleteDir(t);

	String st = pSet->gui.track;
	FillTrackLists();
	TrackListUpd();
	if (st != pSet->gui.track)
		LoadTrack();  //load 1st if deleted cur
}



///  Get Materials
//-----------------------------------------------------------------------------------------------------------

/*void App::GetMaterialsFromDef(String filename, bool clear)
{
	if (clear)
		vsMaterials.clear();
	
	ConfigFile cf;
	cf.load(filename, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, "\t:=", true);
	
	ConfigFile::SectionIterator seci = cf.getSectionIterator();
	String secName, key, value;

	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		
		if (!(cf.getSetting("abstract", secName) == "true"))
			vsMaterials.push_back(secName);
		
		seci.getNext();
	}
}/**/

void App::GetMaterials(String filename, bool clear, String type)
{
	if (clear)
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
					//LogO(line);
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
							//LogO(match);
							vsMaterials.push_back(match);						
							break;
						}
					}
			}	}
		}catch (Ogre::Exception &e)
		{
			StringUtil::StrStreamType msg;
			msg << "Exception: FILE: " << __FILE__ << " LINE: " << __LINE__ << " DESC: " << e.getFullDescription() << std::endl;
			LogO(msg.str());
	}	}
	stream->close();
}

void App::GetMaterialsMat(String filename, bool clear, String type)
{
	if (clear)
		vsMaterials.clear();
	
	std::ifstream stream(filename.c_str(), std::ifstream::in);
	if (!stream.fail())
	{	try
		{	while(!stream.eof())
			{
				char ch[256+2];
				stream.getline(ch,256);
				std::string line = ch;
				StringUtil::trim(line);
 
				if (StringUtil::startsWith(line, type/*"material"*/))
				{
					//LogO(line);
					Ogre::vector<String>::type vec = StringUtil::split(line," \t:");
					bool skipFirst = true;
					for (Ogre::vector<String>::type::iterator it = vec.begin(); it < vec.end(); ++it)
					{
						std::string match = (*it);
						StringUtil::trim(match);
						if (!match.empty())
						{
							if (skipFirst)
							{	skipFirst = false;  continue;	}

							//LogO(match);
							vsMaterials.push_back(match);						
							break;
						}
					}
			}	}
		}catch (Ogre::Exception &e)
		{
			StringUtil::StrStreamType msg;
			msg << "Exception: FILE: " << __FILE__ << " LINE: " << __LINE__ << " DESC: " << e.getFullDescription() << std::endl;
			LogO(msg.str());
	}	}
	stream.close();
}


///  system file, dir
//-----------------------------------------------------------------------------------------------------------
namespace bfs = boost::filesystem;

bool App::TrackExists(String name/*, bool user*/)
{	// ignore letters case..
	for (strlist::const_iterator it = liTracks.begin(); it != liTracks.end(); ++it)
		if (*it == name)  return true;
	for (strlist::const_iterator it = liTracksUser.begin(); it != liTracksUser.end(); ++it)
		if (*it == name)  return true;
	return false;
}

bool App::Rename(String from, String to)
{
	try
	{	if (bfs::exists(from.c_str()))
			bfs::rename(from.c_str(), to.c_str());
	}
	catch (const bfs::filesystem_error & ex)
	{
		String s = "Error: Renaming file " + from + " to " + to + " failed ! \n" + ex.what();
		strFSerrors += "\n" + s;
		LogO(s);
		return false;
	}
	return true;
}

bool App::Delete(String file)
{
	try
	{	bfs::remove(file.c_str());
	}
	catch (const bfs::filesystem_error & ex)
	{
		String s = "Error: Deleting file " + file + " failed ! \n" + ex.what();
		strFSerrors += "\n" + s;
		LogO(s);
		return false;
	}
	return true;
}

bool App::DeleteDir(String dir)
{
	try
	{	bfs::remove_all(dir.c_str());
	}
	catch (const bfs::filesystem_error & ex)
	{
		String s = "Error: Deleting directory " + dir + " failed ! \n" + ex.what();
		strFSerrors += "\n" + s;
		LogO(s);
		return false;
	}
	return true;
}

bool App::CreateDir(String dir)
{
	try
	{	bfs::create_directory(dir.c_str());
	}
	catch (const bfs::filesystem_error & ex)
	{
		String s = "Error: Creating directory " + dir + " failed ! \n" + ex.what();
		strFSerrors += "\n" + s;
		LogO(s);
		return false;
	}
	return true;
}

bool App::Copy(String file, String to)
{
	try
	{	if (bfs::exists(to.c_str()))
			bfs::remove(to.c_str());

		if (bfs::exists(file.c_str()))
			bfs::copy_file(file.c_str(), to.c_str());
	}
	catch (const bfs::filesystem_error & ex)
	{
		String s = "Error: Copying file " + file + " to " + to + " failed ! \n" + ex.what();
		strFSerrors += "\n" + s;
		LogO(s);
		return false;
	}
	return true;
}
