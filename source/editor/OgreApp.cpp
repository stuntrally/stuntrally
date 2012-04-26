#include "pch.h"
#include "../ogre/common/Defines.h"
#include "OgreApp.h"
#include "../road/Road.h"
#include "../vdrift/pathmanager.h"
#include "../paged-geom/PagedGeometry.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/MaterialGen/MaterialFactory.h"

#if OGRE_PLATFORM != OGRE_PLATFORM_WIN32
	// dir listing
	#include <dirent.h>
	#include <sys/types.h>
#endif
using namespace Ogre;

#include <string>


//  ctor
//----------------------------------------------------------------------------------------------------------------------
App::App()  //  gui wigdets--
	:mToolTip(0), mToolTipTxt(0)
	,valAnisotropy(0), valViewDist(0), valTerDetail(0), valTerDist(0), valRoadDist(0)  //detail graphics
	,valTrees(0), valGrass(0), valTreesDist(0), valGrassDist(0), valWaterSize(0)  //paged
	,valShaders(0), valShadowType(0), valShadowCount(0), valShadowSize(0), valShadowDist(0), valShadowFilter(0)  // shadow
	,brImg(0), wndTabs(0), bnQuit(0)  // brush
	,valSizeMinmap(0), valCamSpeed(0), valCamInert(0)  // settings
	,valTerUpd(0), valSizeRoadP(0), valMiniUpd(0)
	,cmbSky(0), cmbRain1(0),cmbRain2(0), valRain1Rate(0),valRain2Rate(0)  // sun
	,valSunPitch(0),valSunYaw(0), valFogStart(0),valFogEnd(0)
	,edLiAmb(0),edLiDiff(0),edLiSpec(0), edFogClr(0), chkFog(0)
	,cmbTexDiff(0),cmbTexNorm(0), imgTexDiff(0), terrain(0)  // terrain
	,valTerLAll(0),tabsHmap(0),tabsTerLayers(0), idTerLay(0),bTerLay(1)
	,chkTerLay(0),chkTexNormAuto(0), bTexNormAuto(1), chkTerLNoiseOnly(0)
	,valTerTriSize(0),edTerTriSize(0), edTerLScale(0),valTerLScale(0)
	,edLDust(0),edLDustS(0), edLMud(0),edLSmoke(0), edLTrlClr(0)  //ter particles
	,cmbParDust(0),cmbParMud(0),cmbParSmoke(0)
	,cmbSurfType(0),edSuBumpWave(0),edSuBumpAmp(0),edSuRollDrag(0),edSuFrict(0),edSuFrict2(0)  //ter surfaces
	,edGrassDens(0),edTreesDens(0), edGrPage(0),edGrDist(0), edTrPage(0),edTrDist(0)  // vegetation
	,edGrMinX(0),edGrMaxX(0), edGrMinY(0),edGrMaxY(0)
	,edGrSwayDistr(0), edGrSwayLen(0), edGrSwaySpd(0), edTrRdDist(0), edTrImpDist(0)
	,edGrDensSmooth(0), edGrTerMaxAngle(0),edGrTerMaxHeight(0), edSceneryId(0), cmbGrassMtr(0), cmbGrassClr(0)
	,cmbPgLay(0), chkPgLay(0), tabsPgLayers(0), idPgLay(0)  //paged layers
	,imgPaged(0), valLTrAll(0)
	,valLTrDens(0),valLTrRdDist(0), valLTrMinSc(0),valLTrMaxSc(0), valLTrWindFx(0),valLTrWindFy(0)
	,valLTrMaxTerAng(0),edLTrMinTerH(0),edLTrMaxTerH(0),edLTrFlDepth(0)
	,edRdTcMul(0),edRdLenDim(0),edRdWidthSteps(0),edRdHeightOfs(0)  // road
	,edRdSkirtLen(0),edRdSkirtH(0), edRdMergeLen(0),edRdLodPLen(0)
	,edRdColN(0),edRdColR(0), edRdPwsM(0),edRdPlsM(0)
	,imgTrkIco1(0),imgTrkIco2(0), edFind(0)
	,trkMList(0),trkName(0),bListTrackU(0)  // track

	,mTerrainGroup(0), mTerrainPaging(0), mPageManager(0), mTerrainGlobals(0)
	,bTerUpd(0), curBr(2), bGuiReinit(0), noBlendUpd(0), bGI(0), resList(0)
	,ndPos(0), mpos(0), asp(4.f/3.f)
	,ndCar(0),entCar(0), ndStBox(0),entStBox(0), ndFluidBox(0),entFluidBox(0), ndObjBox(0),entObjBox(0)
	,grass(0), trees(0), sun(0), terMaxAng(0.f)
	,eTrkEvent(TE_None), bNewHmap(0), bTrGrUpd(0)
	,iFlCur(0), bRecreateFluids(0), world(0), iObjCur(0), iObjNew(0)
	
	,materialFactory(0), bTerUpdBlend(1)
{
	imgPrv[0]=0; imgMini[0]=0; imgTer[0]=0;  trkDesc[0]=0;
	
	pathTrk[0] = PATHMANAGER::GetTrackPath() + "/";      pathTrkPrv[0] = pathTrk[0] + "_previews/";  resTrk = "";
	pathTrk[1] = PATHMANAGER::GetTrackPathUser() + "/";  pathTrkPrv[1] = pathTrk[1] + "_previews/";
	strFSerrors = "";

	mBrSize[0] = 16.f;	mBrSize[1] = 24.f;	mBrSize[2] = 16.f;	mBrSize[3] = 16.f;
	mBrIntens[0] = 20.f;mBrIntens[1] = 20.f;mBrIntens[2] = 20.f;mBrIntens[3] = 20.f;
	mBrPow[0] = 2.f;	mBrPow[1] = 2.f;	mBrPow[2] = 2.f;	mBrPow[3] = 2.f;
	mBrFq[0] = 1.f;		mBrFq[1] = 1.f;		mBrFq[2] = 1.f;		mBrFq[3] = 1.f;
	mBrOct[0] = 5;		mBrOct[1] = 5;		mBrOct[2] = 5;		mBrOct[3] = 5;
	mBrShape[0] = BRS_Sinus;  mBrShape[1] = BRS_Sinus;	mBrShape[2] = BRS_Sinus;  mBrShape[3] = BRS_Sinus;
	terSetH = 10.f;     mBrFilt = 2.f;  mBrFiltOld = 1.f;  pBrFmask = 0;
	mBrushData = new float[BrushMaxSize*BrushMaxSize];
	sBrushTest[0]=0;   updBrush();
	iSnap = 0;  angSnap = crAngSnaps[iSnap];

	int i;
	for (i=0; i<BR_TXT; ++i)  brTxt[i]=0;
	for (i=0; i<RD_TXT; ++i)  rdTxt[i]=0;
	for (i=0; i<RDS_TXT;++i)  rdTxtSt[i]=0;
	for (i=0; i<ST_TXT; ++i)  stTxt[i]=0;
	for (i=0; i<FL_TXT; ++i)  flTxt[i]=0;
	for (i=0; i<OBJ_TXT;++i)  objTxt[i]=0;
	
	for (i=0; i < StTrk; ++i)  stTrk[0][i] = 0;
	for (i=0; i < 4; ++i)  {  cmbRoadMtr[i]=0;  cmbPipeMtr[i]=0;  }
}

void App::postInit()
{
	materialFactory = new MaterialFactory();
	materialFactory->pApp = this;
	materialFactory->setSceneManager(mSceneMgr);
	materialFactory->setShadows(pSet->shadow_type >= 2);
	materialFactory->setShadowsDepth(pSet->shadow_type >= 3);
	materialFactory->setShadowsSoft(pSet->shadow_type == 4);
	materialFactory->setShaderQuality(pSet->shaders);
	materialFactory->setShadowsFilterSize(pSet->shadow_filter);
	materialFactory->setReflect(pSet->water_reflect);
	materialFactory->setRefract(pSet->water_refract);
	if (pSet->tex_size == 0)
		materialFactory->setTexSize(0);
	else if (pSet->tex_size == 1)
		materialFactory->setTexSize(4096);
}

const Ogre::String App::csBrShape[BRS_ALL] = { "Triangle", "Sinus", "Noise" };  // static


App::~App()
{
	delete[] pBrFmask;  pBrFmask = 0;

	delete materialFactory;
	delete[] mBrushData;
	delete road;
	if (mTerrainPaging)
	{
		OGRE_DELETE mTerrainPaging;
		OGRE_DELETE mPageManager;
	}else
		OGRE_DELETE mTerrainGroup;

	OGRE_DELETE mTerrainGlobals;
}

void App::destroyScene()
{
	if (road)
	{	road->DestroyRoad();  delete road;  road = 0;  }

	if (grass) {  delete grass->getPageLoader();  delete grass;  grass=0;   }
	if (trees) {  delete trees->getPageLoader();  delete trees;  trees=0;   }

	delete[] sc.td.hfHeight;
	delete[] sc.td.hfAngle;

	BaseApp::destroyScene();
}
	

//  util
//---------------------------------------------------------------------------------------------------------------
ManualObject* App::Create2D(const String& mat, Real s, bool dyn)
{
	ManualObject* m = mSceneMgr->createManualObject();
	m->setDynamic(dyn);
	m->setUseIdentityProjection(true);
	m->setUseIdentityView(true);
	m->setCastShadows(false);
	m->estimateVertexCount(4);
	m->begin(mat, RenderOperation::OT_TRIANGLE_STRIP);
	m->position(-s,-s*asp, 0);  m->textureCoord(0, 1);
	m->position( s,-s*asp, 0);  m->textureCoord(1, 1);
	m->position(-s, s*asp, 0);  m->textureCoord(0, 0);
	m->position( s, s*asp, 0);  m->textureCoord(1, 0);
	m->end();
 
	AxisAlignedBox aabInf;	aabInf.setInfinite();
	m->setBoundingBox(aabInf);  // always visible
	m->setRenderQueueGroup(RQG_Hud2);
	return m;
}


//  vdr util, get tracks
//----------------------------------------------------------------------------------------------------------------------
bool string_compare(const std::string& s1, const std::string& s2)
{
	return strcmp(s1.c_str(), s2.c_str()) != 0;
}

bool App::GetFolderIndex(std::string dirpath, std::list <std::string>& dirlist, std::string extension)
{
#ifndef _WIN32
	DIR *dp;
	struct dirent *ep;
	dp = opendir(dirpath.c_str());
	if (dp != NULL)
	{
		while ( ( ep = readdir( dp ) ) )
		{
			//puts (ep->d_name);
			std::string newname = ep->d_name;
			if (newname[0] != '.')
			{
				dirlist.push_back(newname);
			}
		}
		(void) closedir(dp);
	}
	else
		return false;
#else
	HANDLE	  hList;
	TCHAR	  szDir[MAX_PATH+1];
	WIN32_FIND_DATA FileData;

	// Get the proper directory path
	sprintf(szDir, "%s\\*", dirpath.c_str ());

	// Get the first file
	hList = FindFirstFile(szDir, &FileData);
	if (hList == INVALID_HANDLE_VALUE)
	{ 
		//no files found.  that's OK
	}
	else
	{
		// Traverse through the directory structure
		while (FindNextFile(hList, &FileData))
		{
			// Check the object is a directory or not
			if (FileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			{} else
			{
				if (FileData.cFileName[0] != '.')
				{
					dirlist.push_back (FileData.cFileName);
				}
			}
		}
	}

	FindClose(hList);
#endif
	
	//remove non-matcthing extensions
	if (!extension.empty())
	{
		std::list <std::list <std::string>::iterator> todel;
		for (std::list <std::string>::iterator i = dirlist.begin(); i != dirlist.end(); ++i)
		{
			if (i->find(extension) != i->length()-extension.length())
				todel.push_back(i);
		}
		
		for (std::list <std::list <std::string>::iterator>::iterator i = todel.begin(); i != todel.end(); ++i)
			dirlist.erase(*i);
	}
	
	dirlist.sort();
	//dirlist.sort(dirlist.begin(), dirlist.end(), string_compare);  //?-
	return true;
}
