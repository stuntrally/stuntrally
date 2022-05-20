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
	ter = true;  secEdited = 0;

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

	fluids.clear();  objects.clear();  emitters.clear();  //
}


///  start
///------------------------------
pair <MATHVECTOR<float,3>, QUATERNION<float> > Scene::GetStart(int index, bool notLoopedReverse)
{
	int st = notLoopedReverse ? 1 : 0;
	pair <MATHVECTOR<float,3>, QUATERNION<float> > sp = make_pair(startPos[st], startRot[st]);
	if (index == 0)
		return sp;

	MATHVECTOR<float,3> backward(-gPar.startNextDist * index,0,0);
	sp.second.RotateVector(backward);
	sp.first = sp.first + backward;
	return sp;
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


void Scene::UpdPgLayers()
{
	pgLayers.clear();
	for (int i=0; i < ciNumPgLay; ++i)
	{
		if (pgLayersAll[i].on)
			pgLayers.push_back(i);
	}
}


//  util
float Scene::GetDepthInFluids(Vector3 pos)
{
	float fa = 0.f;
	for (int i=0; i < fluids.size(); ++i)
	{
		const FluidBox& fb = fluids[i];
		if (fb.pos.y - pos.y > 0.f)  // dont check when above fluid, ..or below its size-
		{
			const float sizex = fb.size.x*0.5f, sizez = fb.size.z*0.5f;
			//  check rect 2d - no rot !
			if (pos.x > fb.pos.x - sizex && pos.x < fb.pos.x + sizex &&
				pos.z > fb.pos.z - sizez && pos.z < fb.pos.z + sizez)
			{
				float f = fb.pos.y - pos.y;
				if (f > fa)  fa = f;
	}	}	}
	return fa;
}
