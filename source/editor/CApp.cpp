#include "pch.h"
#include "../ogre/common/Defines.h"
#include "../ogre/common/RenderConst.h"
#include "../ogre/common/CData.h"
#include "../vdrift/pathmanager.h"
#include "CApp.h"
#include "CGui.h"
#include "../road/Road.h"
#include "../paged-geom/PagedGeometry.h"
#include "../ogre/common/WaterRTT.h"
#include "../ogre/common/RenderBoxScene.h"

#include "../shiny/Main/Factory.hpp"
#include "../shiny/Platforms/Ogre/OgrePlatform.hpp"
#include <OgreTerrainPaging.h>
#include <OgreTerrainGroup.h>

#if OGRE_PLATFORM != OGRE_PLATFORM_WIN32
	// dir listing
	#include <dirent.h>
	#include <sys/types.h>
#endif
using namespace Ogre;


//  ctor
//----------------------------------------------------------------------------------------------------------------------
App::App(SETTINGS* pSet1)  //  gui wigdets--
	:mTerrainGroup(0), mTerrainPaging(0), mPageManager(0), mTerrainGlobals(0)
	,bTerUpd(0), curBr(0)//, bGuiReinit(0)//, noBlendUpd(0)
	,ndPos(0), mpos(0), asp(4.f/3.f)
	,ndCar(0),entCar(0), ndStBox(0),entStBox(0), ndFluidBox(0),entFluidBox(0), ndObjBox(0),entObjBox(0)
	,grass(0), trees(0), sun(0), pr(0),pr2(0)
	,eTrkEvent(TE_None), bNewHmap(0), bTrGrUpd(0)
	,iFlCur(0), bRecreateFluids(0)
	
	,bTerUpdBlend(1), track(0)
	,world(0), config(0), dispatcher(0), broadphase(0), solver(0)  //blt
	,trackObject(0), trackMesh(0)
	,mStaticGeom(0), mTimer(0.f), bUpdTerPrv(0)
{
	pSet = pSet1;
	//imgPrv[0]=0; imgMini[0]=0; imgTer[0]=0;  trkDesc[0]=0;
	
	//pathTrk[0] = PATHMANAGER::Tracks() + "/";
	//pathTrk[1] = PATHMANAGER::TracksUser() + "/";
	//resTrk = "";  strFSerrors = "";

	mBrSize[0] = 16.f;	mBrSize[1] = 24.f;	mBrSize[2] = 16.f;	mBrSize[3] = 16.f;
	mBrIntens[0] = 20.f;mBrIntens[1] = 20.f;mBrIntens[2] = 20.f;mBrIntens[3] = 20.f;
	mBrPow[0] = 2.f;	mBrPow[1] = 2.f;	mBrPow[2] = 2.f;	mBrPow[3] = 2.f;
	mBrFq[0] = 1.f;		mBrFq[1] = 1.f;		mBrFq[2] = 1.f;		mBrFq[3] = 1.f;
	mBrNOf[0] = 0.f;	mBrNOf[1] = 0.f;	mBrNOf[2] = 0.f;	mBrNOf[3] = 0.f;
	mBrOct[0] = 5;		mBrOct[1] = 5;		mBrOct[2] = 5;		mBrOct[3] = 5;
	mBrShape[0] = BRS_Sinus;  mBrShape[1] = BRS_Sinus;
	mBrShape[2] = BRS_Sinus;  mBrShape[3] = BRS_Sinus;
	terSetH = 10.f;     mBrFilt = 2.f;  mBrFiltOld = 1.f;  pBrFmask = 0;
	mBrushData = new float[BrushMaxSize*BrushMaxSize];
	sBrushTest[0]=0;   updBrush();
	iSnap = 0;  angSnap = crAngSnaps[iSnap];

	///  new
	mWaterRTT = new WaterRTT();
	data = new CData();
	gui = new CGui(this);
	gui->viewBox = new wraps::RenderBoxScene();
	
	track = new TRACK(std::cout, std::cerr);  //!
	sc = new Scene();
	gui->sc = sc;
}

const Ogre::String App::csBrShape[BRS_ALL] = { "Triangle", "Sinus", "Noise", "Noise2", "N-gon" };  // static


///  material factory setup
//---------------------------------------------------------------------------------------------------------------------------
void App::postInit()
{
	sh::OgrePlatform* platform = new sh::OgrePlatform("General", PATHMANAGER::Data() + "/" + "materials");
	platform->setCacheFolder(PATHMANAGER::ShaderDir());
	
	mFactory = new sh::Factory(platform);
	SetFactoryDefaults();
}


App::~App()
{
	gui->viewBox->destroy();
	delete gui->viewBox;

	BltWorldDestroy();
	
	delete track;  //!
	delete[] pBrFmask;  pBrFmask = 0;

	delete[] mBrushData;
	delete road;
	if (mTerrainPaging)
	{
		OGRE_DELETE mTerrainPaging;
		OGRE_DELETE mPageManager;
	}else
		OGRE_DELETE mTerrainGroup;

	OGRE_DELETE mTerrainGlobals;
	delete sc;
	
	delete gui;
}

void App::destroyScene()
{
	//NewCommon(false);  //?
	if (road)
	{	road->DestroyRoad();  delete road;  road = 0;  }

	if (grass) {  delete grass->getPageLoader();  delete grass;  grass=0;   }
	if (trees) {  delete trees->getPageLoader();  delete trees;  trees=0;   }

	DestroyWeather();

	delete[] sc->td.hfHeight;
	delete[] sc->td.hfAngle;

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
