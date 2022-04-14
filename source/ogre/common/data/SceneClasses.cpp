#include "pch.h"
#include "../Def_Str.h"
#include "SceneClasses.h"
#include "../Axes.h"
#include <OgreSceneNode.h>
using namespace std;
using namespace Ogre;


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

SEmitter::SEmitter()
	:name(""), pos(0,0,0), size(1,1,1), up(0,1,0), rot(0.f)
	,rate(0.f), stat(false), upd(2.f)
	,nd(0), ps(0)
{	}


///  bullet to ogre  ----------
void Object::SetFromBlt()
{
	if (!nd)  return;
	nd->setPosition(Axes::toOgre(pos));
	nd->setOrientation(Axes::toOgreW(rot));
}


//  TerData
//------------------------------------------
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
