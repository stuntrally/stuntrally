#include "pch.h"
#include "Defines.h"
#include "OgreApp.h"
#include "../road/Road.h"
#include <boost/filesystem.hpp>
#include "../ogre/common/Gui_Def.h"
using namespace MyGUI;
using namespace Ogre;


//  Gui from xml (scene, road), after load
//..........................................................................................................

void App::SetGuiFromXmls()
{
	if (!mWndOpts)  return;
	bGI = false;
	// set slider value, upd text
	ScrollBar* sl;  size_t v;

	#define Slv(name, val)  \
		sl = (ScrollBar*)mWndOpts->findWidget(#name);  \
		v = val*res;  if (sl)  sl->setScrollPosition(v);  sl##name(sl, v);
	
	#define Ed(name, val)  ed##name->setCaption(toStr(val));
	#define Cmb(cmb, str)  cmb->setIndexSelected( cmb->findItemIndexWith(str) );
	

	//  [Sky]
	//-----------------------------------------------
	Cmb(cmbSky, sc.skyMtr);
	Slv(SunPitch, sc.ldPitch /90.f);
	Slv(SunYaw,   (sc.ldYaw + 180.f)  /360.f);
	Ed(LiAmb, sc.lAmb);  Ed(LiDiff, sc.lDiff);  Ed(LiSpec, sc.lSpec);
	Ed(FogClr, sc.fogClr);
	Slv(FogStart, powf(sc.fogStart /2000.f, 0.5f));
	Slv(FogEnd,   powf(sc.fogEnd   /2000.f, 0.5f));

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
	Ed(GrSwayDistr, sc.grSwayDistr);  Ed(GrDensSmooth, sc.grDensSmooth);
	Ed(GrSwayLen, sc.grSwayLen);	Ed(GrSwaySpd, sc.grSwaySpeed);
	Ed(TrRdDist, sc.trRdDist);		Ed(TrImpDist, sc.trDistImp);
	Ed(GrTerMaxAngle, sc.grTerMaxAngle);
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
	bGI = true;
}


void App::btnNewGame(WP)
{
	if (trkName)  trkName->setCaption(sListTrack.c_str());
	pSet->track = sListTrack;
	pSet->track_user = bListTrackU;  //UpdWndTitle();//? load
	LoadTrack();
}



///  tools  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
/// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

//  track files
const int cnTrkF = 5, cnTrkFd = 2, cnTrkFo = 3;
const Ogre::String csTrkFo[cnTrkFo] = {"/grass1.png", "/grassColor.png", "/grassDensity.png"},
	csTrkF[cnTrkF] = {"/heightmap.f32", "/road.xml", "/scene.xml", "/surfaces.txt", "/track.txt"},  // copy, new
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
	if (sCopyTrack == pSet->track && bCopyTrackU == (pSet->track_user ? 1 : 0))
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
	String from = PathCopyTrk();
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
	String from = PathCopyTrk();
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
	String from = PathCopyTrk();
	Scene sF;  sF.LoadXml(from + "/scene.xml");

	sc.densGrass = sF.densGrass;  sc.densTrees = sF.densTrees;
	sc.grPage = sF.grPage;  sc.grDist = sF.grDist;
	sc.trPage = sF.trPage;  sc.trDist = sF.trDist;
	sc.grMinSx = sF.grMinSx;  sc.grMaxSx = sF.grMaxSx;
	sc.grMinSy = sF.grMinSy;  sc.grMaxSy = sF.grMaxSy;
	sc.grSwayDistr = sF.grSwayDistr;  sc.grSwayLen = sF.grSwayLen;
	sc.grSwaySpeed = sF.grSwaySpeed;
	sc.grTerMaxAngle = sF.grTerMaxAngle;
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
	Real sf = std::max(0.1f, s2r(edScaleAllMul->getCaption()) );  // scale mul
	
	//  road
	for (int i=0; i < road->getNumPoints(); ++i)
	{
		road->Scale1(i, sf - 1.f);
		road->mP[i].width *= sf;
	}
	road->bSelChng = true;
	
	//  fluids
	for (int i=0; i < sc.fluids.size(); ++i)
	{
		FluidBox& fb = sc.fluids[i];
		fb.pos.x *= sf;  fb.pos.z *= sf;
		fb.size.x *= sf;  fb.size.z *= sf;
	}

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
void App::btnTrackNew(WP)  //- ..
{
	String name = trkName->getCaption();
	name = StringUtil::replaceAll(name, "*", "");
	if (TrackExists(name))  {	Message::createMessageBox(
			"Message", "New Track", "Track " + name + " already exists.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
		return;  }

	String st = pathTrk[bListTrackU] + sListTrack, sto = st + "/objects";  // from
	String t = pathTrk[1] + name, to = t + "/objects";  // to, new

	CreateDir(t);  CreateDir(to);
	for (int i=0; i < cnTrkFo; ++i)  Copy(sto + csTrkFo[i], to + csTrkFo[i]);
	for (int i=0; i < cnTrkF; ++i)   Copy(st + csTrkF[i], t + csTrkF[i]);
	//Copy(pathTrkPrv + sListTrack + ".jpg");  // no preview
	Copy(pathTrkPrv[bListTrackU] + sListTrack + "_mini.png", pathTrkPrv[1] + name + "_mini.png");
	Copy(pathTrkPrv[bListTrackU] + sListTrack + "_ter.jpg", pathTrkPrv[1] + name + "_ter.jpg");

	sListTrack = name;  pSet->track = name;  pSet->track_user = 1;  UpdWndTitle();
	TrackListUpd();
}

///  Rename
void App::btnTrackRename(WP)  // ?...
{
	String name = trkName->getCaption();

	/*if (bListTrackU==0)  {  // could force when originals writable..
		Message::createMessageBox(
			"Message", "Rename Track", "Track " + name + " is original and can't be renamed.",
			MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
			return;  }/**/

	if (name != sListTrack)
	{	if (TrackExists(name))  {	Message::createMessageBox(
				"Message", "Rename Track", "Track " + name + " already exists.",
				MessageBoxStyle::IconWarning | MessageBoxStyle::Ok);
			return;  }
		
		//  track dir
		Rename(pathTrk[bListTrackU] + sListTrack, pathTrk[/*1*/bListTrackU] + name);
		//  preview shot, minimap
		String from = pathTrkPrv[bListTrackU] + sListTrack, to = pathTrkPrv[/*1*/bListTrackU] + name;
		Rename(from + ".jpg", to + ".jpg");
		Rename(from + "_mini.png", to + "_mini.png");
		Rename(from + "_ter.jpg", to + "_ter.jpg");
		
		sListTrack = name;  pSet->track = sListTrack;  pSet->track_user = 1;/**/  UpdWndTitle();
		TrackListUpd();  //listTrackChng(trkList,0);
	}
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
	String t = pathTrk[bListTrackU] + sListTrack, to = t + "/objects";
	for (int i=0; i < cnTrkFo; ++i)  Delete(to + csTrkFo[i]);
	for (int i=0; i < cnTrkF; ++i)   Delete(t + csTrkF[i]);
	for (int i=0; i < cnTrkFd; ++i)   Delete(t + csTrkFd[i]);
	DeleteDir(to);  DeleteDir(t);
	Delete(pathTrkPrv[bListTrackU] + sListTrack + ".jpg");
	Delete(pathTrkPrv[bListTrackU] + sListTrack + "_mini.png");
	Delete(pathTrkPrv[bListTrackU] + sListTrack + "_ter.jpg");

	String st = pSet->track;
	TrackListUpd();
	if (st != pSet->track)
		LoadTrack();  //load 1st if deleted cur
}



//-----------------------------------------------------------------------------------------------------------
//  Surfaces
//-----------------------------------------------------------------------------------------------------------

bool App::LoadSurf()
{
	std::string path = pathTrk[bListTrackU] + pSet->track + "/surfaces.txt";
	CONFIGFILE cf;
	if (!cf.Load(path))
	{	LogO("Can't find surfaces configfile: " + path);  return false;  }
	
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
	return true;
}

bool App::SaveSurf(const std::string& path)
{
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

			const TRACKSURFACE& surf = su[i];  std::string ss = s;
			cf.SetParam(ss + ".ID", surf.type);
			cf.SetParam(ss + ".BumpWaveLength", surf.bumpWaveLength);
			cf.SetParam(ss + ".BumpAmplitude", surf.bumpAmplitude);
			cf.SetParam(ss + ".FrictionNonTread", surf.frictionNonTread);
			cf.SetParam(ss + ".FrictionTread", surf.frictionTread);
			cf.SetParam(ss + ".RollResistanceCoefficient", surf.rollResistanceCoefficient);
			cf.SetParam(ss + ".RollingDrag", surf.rollingDrag);
		}
	}
	return cf.Write(true, path);
}


///  Get Materials
//-----------------------------------------------------------------------------------------------------------

void App::GetMaterialsFromDef(String filename, bool clear)
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
}

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
