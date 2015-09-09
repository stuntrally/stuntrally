#include "pch.h"
#include "../Def_Str.h"
#include "../vdrift/par.h"
#include "SceneXml.h"
#include "FluidsXml.h"
#include "TracksXml.h"
#include "../Axes.h"
#include "tinyxml.h"
#include "tinyxml2.h"
#include <OgreSceneNode.h>
#include "../vdrift/game.h"  // for surfaces map
using namespace std;
using namespace Ogre;
using namespace tinyxml2;


Scene::Scene()
	: pGame(0)
{
	pFluidsXml = 0;  pReverbsXml = 0;
	Default();
}
void Scene::Default()
{
	ter = true;  vdr = false;  secEdited = 0;

	asphalt = false;  denyReversed = false;  noWrongChks = false;
	windAmt = 0.f;  damageMul = 1.f;
	gravity = 9.81f;
	
	sAmbient = "";  sReverbs = "";

	skyMtr = "World/NoonSky";  skyYaw = 0.f;
	rainEmit = 0;  rainName = "";
	rain2Emit = 0;  rain2Name = "";

	fogStart = 600;  fogEnd = 1600;
	fogClr = fogClr2 = fogClrH = SColor(0.73f, 0.86f, 1.0f, 1.f);
	fogHeight = -300.f;  fogHDensity = 100.f;  fogHStart = 0;  fogHEnd = 400;
	fHDamage = 0.f;

	ldPitch = 50.f;  ldYaw = 30.f;
	lAmb  = SColor(0.16f, 0.0f, 0.45f);
	lDiff = SColor(0.16f, 0.0f, 0.7f);	lSpec = SColor(0.16f, 0.05f, 1.f);

	sParDust = "Dust";  sParMud = "Mud";  sParSmoke = "Smoke";

	td.Default();

	densTrees=0;  densGrass=0;  grDensSmooth=6;
	grPage = 80;  grDist = 80;

	for (int i=0; i < ciNumGrLay; ++i)
	{
		SGrassLayer* gr = &grLayersAll[i];
		gr->on = i == 0;
		gr->material = "grassJungle";  gr->colorMap = "grClrJungle.png";
		gr->minSx = 1.2f;  gr->minSy = 1.2f;  gr->maxSx = 1.6f;  gr->maxSy = 1.6f;
		gr->swayDistr = 4.0f;  gr->swayLen = 0.2f;  gr->swaySpeed = 0.5f;
	}
	trPage = 200;  trDist = 200;  trDistImp = 800;  trRdDist = 3;

	camPos = Vector3(10.f,20.f,10.f);  camDir = Vector3(0.f,-0.3f,1.f);

	fluids.clear();  //
	objects.clear();  //
}

PagedLayer::PagedLayer()
{
	on = 0;  name = "";  dens = 0.1f;  cnt = 0;
	windFx = 0.0f;  windFy = 0.0f;  addRdist = 0;  maxRdist = 100;
	minScale = 0.1f;  maxScale = 0.25f;  ofsY = 0.f;
	maxTerAng = 50.f;  minTerH = -100.f;  maxTerH = 100.f;
	maxDepth = 5.f;
}

SGrassLayer::SGrassLayer()
{
	on = false;  grl = 0;
	dens = 0.1f;  iChan = 0;
	minSx = 1.2f;  minSy = 1.2f;  maxSx = 1.6f;  maxSy = 1.6f;
	swayDistr = 4.f;  swayLen = 0.2f; swaySpeed = 0.5f;
	material = "grassForest";  colorMap = "grClrForest.png";
}

SGrassChannel::SGrassChannel()
{
	angMin = 0.f;  angMax = 30.f;  angSm = 20.f;
	hMin = -100.f;  hMax = 100.f;  hSm = 20.f;  rdPow = 0.f;
	noise = 0.0f;  nFreq = 25.f;  nPers = 0.3f;  nPow = 1.2f;  nOct = 3;
}


FluidBox::FluidBox()
	:cobj(0), id(-1), idParticles(0), solid(false), deep(false)
	,pos(Vector3::ZERO), rot(Vector3::ZERO)
	,size(Vector3::ZERO), tile(0.01,0.01)
{	}

Object::Object()
	:nd(0),ent(0),ms(0),co(0),rb(0), dyn(false)
	,pos(0,0,0),rot(0,-1,0,0), tr1(0)
	,scale(Vector3::UNIT_SCALE)
{	}


///  start
///------------------------------
pair <MATHVECTOR<float,3>, QUATERNION<float> > Scene::GetStart(int index)
{
	pair <MATHVECTOR<float,3>, QUATERNION<float> > sp = make_pair(startPos, startRot);
	if (index == 0)
		return sp;

	MATHVECTOR<float,3> backward(-gPar.startNextDist * index,0,0);
	sp.second.RotateVector(backward);
	sp.first = sp.first + backward;
	return sp;
}


///  bullet to ogre  ----------
void Object::SetFromBlt()
{
	if (!nd)  return;
	nd->setPosition(Axes::toOgre(pos));
	nd->setOrientation(Axes::toOgreW(rot));
}


void Scene::UpdRevSet()
{
	if (!pReverbsXml)  return;
	string s = sReverbs == "" ? "base" : sReverbs;

	int id = pReverbsXml->revmap[sReverbs]-1;
	if (id == -1)
	{	LogO("!scene.xml reverb set not found in xml: "+sReverbs);
		//..
	}else
	{	const ReverbSet &r = pReverbsXml->revs[id], &b = pReverbsXml->base;
		revSet.descr   = r.descr   != "" ? r.descr   : b.descr;
		revSet.normal  = r.normal  != "" ? r.normal  : b.normal;
		revSet.cave    = r.cave    != "" ? r.cave    : b.cave;
		revSet.cavebig = r.cavebig != "" ? r.cavebig : b.cavebig;
		revSet.pipe    = r.pipe    != "" ? r.pipe    : b.pipe;
		revSet.pipebig = r.pipebig != "" ? r.pipebig : b.pipebig;
		revSet.influid = r.influid != "" ? r.influid : b.influid;
	}
}


void Scene::UpdateFluidsId()
{
	if (!pFluidsXml)  return;
	
	//  set fluids id from name
	for (int i=0; i < fluids.size(); ++i)
	{
		int id = pFluidsXml->flMap[fluids[i].name]-1;
		fluids[i].id = id;
		fluids[i].idParticles = id == -1 ? -1    : pFluidsXml->fls[id].idParticles;
		fluids[i].solid       = id == -1 ? false : pFluidsXml->fls[id].solid;
		fluids[i].deep        = id == -1 ? false : pFluidsXml->fls[id].deep;
		if (id == -1)
			LogO("!Warning: Scene fluid name: " + fluids[i].name + " not found in xml!");
	}
}


void Scene::UpdateSurfId()
{
	if (!pGame)  return;
	//  update surfId from surfName
	int i;
	//  terrain
	for (i=0; i < td.ciNumLay; ++i)
	{
		const std::string& s = td.layersAll[i].surfName;
		int id = pGame->surf_map[s]-1;
		if (id == -1)
		{	id = 0;  // default if not found
			LogO("! Warning: Surface not found (terrain): "+s);
		}
		td.layersAll[i].surfId = id;  // cached
	}
	//  road
	for (i=0; i < 4; ++i)
	{
		const std::string& s = td.layerRoad[i].surfName;
		int id = pGame->surf_map[s]-1;
		if (id == -1)
		{	id = 0;
			LogO("! Warning: Surface not found (road): "+s);
		}
		// road1mtr ?
		td.layerRoad[i].surfId = id;
	}
}


TerData::TerData()
{
	hfHeight = NULL;
	Default();
}
void TerData::Default()
{
	iVertsX = 512*2 +1;
	fTriangleSize = 1.f;  // scale
	triplanarLayer1 = 8;  triplanarLayer2 = 8;  triplCnt = 0;  // off
	errorNorm = 1.7;  normScale = 1.f;
	emissive = false;  specularPow = 32.f;  specularPowEm = 2.f;
	road1mtr = true;

	for (int i=0; i < ciNumLay; ++i)
	{	
		TerLayer& l = layersAll[i];  l.tiling = 8.5f;
		l.on = i==0;
		l.texFile = "";  l.texNorm = "";
		l.dust = 0.f;  l.mud = 1.f;  l.smoke = 0.f;
		l.tclr = SColor(0.16f,0.5f,0.2f,0.7f);
		l.fDamage = 0.f;
	}
	for (int i=0; i < 4; ++i)
	{
		TerLayer& r = layerRoad[i];
		r.dust = 0.f;  r.mud = 0.f;  // r.smoke = 1.f;
		r.tclr = SColor(0.16f,0.5f,0.2f,0.7f);  r.tcl = r.tclr.GetRGBA();
		r.fDamage = 0.f;
	}
	UpdVals();  UpdLayers();
}

TerLayer::TerLayer() :
	on(true), tiling(4.f), triplanar(false),
	dust(0.f),dustS(0.2f), mud(0.f), smoke(0.f),
	tclr(0.16f,0.5f,0.2f,0.7f),
	angMin(0.f),angMax(90.f), angSm(20.f),
	hMin(-300.f),hMax(300.f), hSm(20.f), nOnly(false),
	noise(1.f), nprev(0.f), nnext2(0.f),
	surfName("Default"), surfId(0),  //!
	fDamage(0.f)
{
	nFreq[0]=25.f; nPers[0]=0.30f; nPow[0]=1.5f; nOct[0]=3;
	nFreq[1]=30.f; nPers[1]=0.40f; nPow[1]=1.2f; nOct[1]=3;
}

void TerData::UpdVals()
{
	iVertsY = iVertsX;  //square only-[]
	iTerSize = iVertsX;
	fTerWorldSize = (iTerSize-1)*fTriangleSize;
}


//  fill only active layers
//------------------------------------------
void TerData::UpdLayers()
{
	layers.clear();  int li = 0;
	triplanarLayer1 = 8;  triplanarLayer2 = 8;  triplCnt = 0;  // off
	for (int i=0; i < ciNumLay; ++i)
	{
		if (layersAll[i].on)
		{
			if (layersAll[i].triplanar)
			{	++triplCnt;
				if (triplanarLayer1 < 8)
					triplanarLayer2 = li;
				else
					triplanarLayer1 = li;  }
			++li;
			layers.push_back(i);
	}	}
}

void Scene::UpdPgLayers()
{
	pgLayers.clear();
	for (int i=0; i < ciNumPgLay; ++i)
	{
		if (pgLayersAll[i].on)
			pgLayers.push_back(i);
	}
}



///  Presets

PSky::PSky()
	:mtr("sky"), clr("#C0E0FF")
	,ldYaw(0.f), ldPitch(0.f)
{	}

PTer::PTer()
	:tiling(8.f), triplanar(false)
	,dust(0.f), dustS(0.2f), mud(0.f)
	,tclr(0.2f, 0.2f, 0.1f, 0.6f)
	,angMin(0.f), angMax(90.f), dmg(0.f)
	,surfName("Default")
{	}

PRoad::PRoad()
	:dust(0.f), dustS(0.2f), mud(0.f)
	,tclr(0.2f, 0.2f, 0.1f, 0.6f)
{	}

PGrass::PGrass()
	:clr("GrassClrJungle")
	,minSx(1.2f), minSy(1.2f), maxSx(1.6f), maxSy(1.6f)
{	}

PVeget::PVeget()
	:minScale(0.6f), maxScale(1.f)
	,windFx(0.02f), windFy(0.002f)
	,addRdist(1)
	,maxTerAng(30.f)
	,maxDepth(0.f)
{	}


const PSky* Presets::GetSky(std::string mtr)
{
	int id = isky[mtr]-1;
	return id >= 0 ? &sky[id] : 0;
}

const PTer* Presets::GetTer(std::string tex)
{
	int id = iter[tex]-1;
	return id >= 0 ? &ter[id] : 0;
}

const PRoad* Presets::GetRoad(std::string mtr)
{
	int id = ird[mtr]-1;
	return id >= 0 ? &rd[id] : 0;
}

const PGrass* Presets::GetGrass(std::string mtr)
{
	int id = igr[mtr]-1;
	return id >= 0 ? &gr[id] : 0;
}

const PVeget* Presets::GetVeget(std::string mesh)
{
	int id = iveg[mesh]-1;
	return id >= 0 ? &veg[id] : 0;
}
