#include "stdafx.h"
#include "../../road/Road.h"
#ifdef ROAD_EDITOR
	#include "../../editor/OgreApp.h"
	#include "../../editor/settings.h"
#else
	#include "../OgreGame.h"
	#include "../vdrift/settings.h"
#endif


//  fill Blend maps
//--------------------------------------------------------------------------------------------------------------------------
void App::initBlendMaps(Terrain* terrain)
{
	int b = sc.td.layers.size(), i;
	float* pB[6];	TerrainLayerBlendMap* bMap[6];
	Ogre::uint16 t = terrain->getLayerBlendMapSize(), x,y;
	const float f = 0.8f / t * 3.14f;  //par-

	//  mtr map
	#ifndef ROAD_EDITOR
	delete[] blendMtr;  blendMtr = 0;
	blendMtr = new char[t*t];
	#endif

	for (i=0; i < b-1; ++i)  {
		bMap[i] = terrain->getLayerBlendMap(i+1);  pB[i] = bMap[i]->getBlendPointer();  }

	for (y = 0; y < t; ++y)
	for (x = 0; x < t; ++x)
	{
		float fx = f*x, fy = f*y;	//  val,val1:  0 0 - [0]   1 0  - [1]   0 1 - [2]
		const Real p = (b > 4) ? 3.f : ( (b > 3) ? 2.f : 1.f ), q = 1.f;
		Real val =			 pow(0.5f + 0.5f*sinf(24.f* fx)*cosf(24.f* fy), p);
		Real val1 = max(0.f, pow(0.5f + 0.5f*cosf(18.f* fy)*sinf(18.f* fx), p) - val);
		Real val2 = max(0.f, pow(0.5f + 0.5f*cosf(22.f* fy)*sinf(21.f* fx), q) - val-val1);
		Real val3 = max(0.f, pow(0.5f + 0.5f*cosf(19.f*fy)*sinf(20.f*fx), q) - val-val1-val2);

		char mtr = 1;
		if (b > 1)  {  *(pB[0])++ = val;   if (val > 0.5f)  mtr = 2;  }
		if (b > 2)  {  *(pB[1])++ = val1;  if (val1> 0.5f)  mtr = 3;  }
		if (b > 3)  {  *(pB[2])++ = val2;  if (val2> 0.5f)  mtr = 4;  }
		if (b > 4)  {  *(pB[3])++ = val3;  if (val3> 0.5f)  mtr = 5;  }

		#ifndef ROAD_EDITOR
		blendMtr[y*t + x] = mtr;
		#endif
	}
	for (i=0; i < b-1; ++i)  {
		bMap[i]->dirty();  bMap[i]->update();  }

	iBlendMaps = b;  blendMapSize = t;
	
	//blendMap[0]->loadImage("blendmap1.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);  // v^
	//Image bl0;  // ?-
	//terrain->getLayerBlendTexture(1)->convertToImage(bl0);
	//bl0.save("blend0.png");
	
	/*// set up a colour map
	if (!terrain->getGlobalColourMapEnabled())
	{
		terrain->setGlobalColourMapEnabled(true);
		Image colourMap;
		colourMap.load("testcolourmap.jpg", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		terrain->getGlobalColourMap()->loadImage(colourMap);
	}
	*/
}


///  Setup Terrain
//--------------------------------------------------------------------------------------------------------------------------
void App::configureTerrainDefaults(Light* l)
{
	mTerrainGlobals->setMaxPixelError(pSet->terdetail);  // 1- 4-8+
	//mTerrainGlobals->setUseRayBoxDistanceCalculation(true);
	//mTerrainGlobals->getDefaultMaterialGenerator()->setDebugLevel(1);
	if (l)  {
	mTerrainGlobals->setLightMapDirection(l->getDerivedDirection());
	mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
	mTerrainGlobals->setCompositeMapDiffuse(l->getDiffuseColour());  }
	//mTerrainGlobals->setShadowVal(0.6f);  //+ new, for compositeMap shadow

	mTerrainGlobals->setCompositeMapSize(1024);  // par,..
	mTerrainGlobals->setCompositeMapDistance(pSet->terdist);  //100
	mTerrainGlobals->setLightMapSize(256);  //256, 2k
	mTerrainGlobals->setSkirtSize(1);  //`
	//matProfile->setLightmapEnabled(false);

	// Configure default import settings for if we use imported image
	Terrain::ImportData& di = mTerrainGroup->getDefaultImportSettings();
	di.terrainSize = sc.td.iTerSize; // square []-
	di.worldSize = sc.td.fTerWorldSize;  //di.inputScale = td.Hmax;
	di.minBatchSize = 33; //17;   //17 o: 33   3 5 9 17 33 65 129
	di.maxBatchSize = min(65, sc.td.iTerSize); //65;  //65 size of one tile in vertices
	//const uint16 numPages = 4;  // 2^n
	//const uint16 numLods = 4;  // 1..8
	//di.maxBatchSize = (TERRAIN_SIZE-1) / numPages +1;
	//di.minBatchSize = (di.maxBatchSize-1) >> numLods +1;

	//  textures  iBlendMaps-
	int ls = sc.td.layers.size();
	di.layerList.resize(ls);
	for (int i=0; i < ls; ++i)
	{
		TerLayer& l = sc.td.layersAll[sc.td.layers[i]];
		di.layerList[i].worldSize = l.tiling;
		di.layerList[i].textureNames.push_back(l.texFile);
		di.layerList[i].textureNames.push_back(l.texNorm);
	}
}


///--------------------------------------------------------------------------------------------------------------------------
///  Create Terrain
///--------------------------------------------------------------------------------------------------------------------------
void App::CreateTerrain(bool bNewHmap, bool bTer)
{
	iBlendMaps = 0;  terrain = 0;

	///  sky
	Vector3 scl = pSet->view_distance*Vector3::UNIT_SCALE;
	CreateSkyDome(bTer ? sc.skyMtr : "sky/blue_clouds", scl);
	UpdFog();

	//  light
	mSceneMgr->destroyAllLights();
	sun = mSceneMgr->createLight("Sun");
	sun->setType(Light::LT_DIRECTIONAL);  UpdSun();


	///  --------  fill HeightField data --------
	if (bTer)
	{
		delete[] sc.td.hfData;
		sc.td.hfData = new float[sc.td.iVertsX * sc.td.iVertsY];
		int siz = sc.td.iVertsX * sc.td.iVertsY * sizeof(float);

		String name = TrkDir() + (bNewHmap ? "heightmap-new.f32" : "heightmap.f32");

		if (sc.td.GENERATE_HMAP)	//  generate height -
		{
			for (int j=0; j < sc.td.iVertsY; ++j)
			{
				int a = j * sc.td.iVertsX;
				for (int i=0; i < sc.td.iVertsX; ++i,++a)
					sc.td.hfData[a] = sc.td.getHeight(i,j);
			}
			if (1)	// save f32 HMap
			{
				ofstream of;
				of.open(name.c_str(), ios_base::binary);
				of.write((const char*)&sc.td.hfData[0], siz);
				of.close();
			}
		}
		else	//  load from f32 HMap +
		{
			ifstream fi;
			fi.open(name.c_str(), ios_base::binary);
			fi.read((char*)&sc.td.hfData[0], siz);
			fi.close();
		}
	}
	///

	//  Terrain
	if (bTer)
	{
		if (!mTerrainGlobals)
		mTerrainGlobals = OGRE_NEW TerrainGlobalOptions();
		mTerrainGroup = OGRE_NEW TerrainGroup(mSceneMgr, Terrain::ALIGN_X_Z,
			sc.td.iTerSize, sc.td.fTerWorldSize);
		mTerrainGroup->setOrigin(Vector3::ZERO);

		configureTerrainDefaults(sun);

		if (sc.td.hfData)
			mTerrainGroup->defineTerrain(0,0, sc.td.hfData);
		else
			mTerrainGroup->defineTerrain(0,0, 0.f);

		// sync load since we want everything in place when we start
		mTerrainGroup->loadAllTerrains(true);

		TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();
		while (ti.hasMoreElements())
		{
			Terrain* t = ti.getNext()->instance;
			initBlendMaps(t);
			terrain = t;  //<set
			terrain->setVisibilityFlags(4);  // hide terrain in render target
		}

		mTerrainGroup->freeTemporaryResources();
	}
	
	changeShadows();
}


//  Sky Dome
//----------------------------------------------------------------------------------------------------------------------
void App::CreateSkyDome(String sMater, Vector3 sc)
{
	ManualObject* m = mSceneMgr->createManualObject();
	//m->setUseIdentityView(true);
	m->begin(sMater, RenderOperation::OT_TRIANGLE_LIST);

	//  divisions- quality
	int ia = 32*2, ib = 24,iB = 24 +1/*below_*/, i=0;
	//int ia = 4, ib = 4, i=0;
	//  angles, max
	float a,b;  const float B = PI/2.f, A = 2.f*PI;
	float bb = B/ib, aa = A/ia;  // add
	ia += 1;

	//  up/dn y  )
	for (b = 0.f; b <= B+bb/*1*/*iB; b += bb)
	{
		float cb = sinf(b), sb = cosf(b);
		float y = sb;

		//  circle xz  o
		for (a = 0.f; a <= A; a += aa, ++i)
		{
			float x = cosf(a)*cb, z = sinf(a)*cb;
			m->position(x,y,z);

			m->textureCoord(a/A, b/B);

			if (a > 0.f && b > 0.f)  // rect 2tri
			{
				m->index(i-1);  m->index(i);     m->index(i-ia);
				m->index(i-1);  m->index(i-ia);  m->index(i-ia-1);
			}
		}
	}
	m->end();	AxisAlignedBox aabInf;	aabInf.setInfinite();
	m->setBoundingBox(aabInf);  // always visible
	m->setRenderQueueGroup(RENDER_QUEUE_SKIES_EARLY);
	m->setCastShadows(false);
	#ifdef ROAD_EDITOR
	m->setVisibilityFlags(32);  // hide on minimap
	#endif

	ndSky = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	ndSky->attachObject(m);
	ndSky->setScale(sc);
}

inline ColourValue Clr3(const Vector3& v)
{
	return ColourValue(v.x, v.y, v.z);
}

void App::UpdFog(bool bForce)
{
	//  fog  directx has 4x denser fog, why ???-
	bool dx = strncmp(mRoot->getRenderSystem()->getName().c_str(), "Direct", 6)==0;
	Real mul = dx ? 0.25f : 1.f;  // sc.fogExp
	ColourValue clr = Clr3(sc.fogClr);
	if (!pSet->bFog || bForce)
		mSceneMgr->setFog(sc.fogMode, clr, mul, sc.fogStart, sc.fogEnd);
	else
		mSceneMgr->setFog(sc.fogMode, clr, mul, 3000, 3200);
}

void App::UpdSun()
{
	if (!sun)  return;
	Vector3 dir = SplineRoad::GetRot(sc.ldYaw, -sc.ldPitch);
	sun->setDirection(dir);
	sun->setDiffuseColour(Clr3(sc.lDiff));
	sun->setSpecularColour(Clr3(sc.lSpec));
	mSceneMgr->setAmbientLight(Clr3(sc.lAmb));
}
