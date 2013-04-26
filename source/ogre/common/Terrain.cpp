#include "pch.h"
#include "../common/Defines.h"
#include "../common/RenderConst.h"
#include "../../road/Road.h"  // sun rot
#include "../shiny/Main/Factory.hpp"

#include "TerrainMaterial.h"

#ifdef ROAD_EDITOR
	#include "../../editor/OgreApp.h"
	#include "../../editor/settings.h"
	#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#else
	#include "../OgreGame.h"
	#include "../../vdrift/game.h"
	#include "../../vdrift/settings.h"
#endif
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include <OgreRoot.h>
#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreManualObject.h>
#include <OgreMeshManager.h>
#include <OgreMaterialManager.h>
#include <OgreEntity.h>
#include "../common/QTimer.h"

using namespace Ogre;


//  fill Blend maps
//--------------------------------------------------------------------------------------------------------------------------
void App::initBlendMaps(Terrain* terrain, int xb,int yb, int xe,int ye, bool full)
{
	QTimer ti;  ti.update();  /// time
	//for (float f=-1.f; f<=2.f; f+=0.02f)  // test
	//	LogO(toStr(f) + " = " + toStr( linRange(f,-0.5f,1.5f,0.2f) ));

	int b = sc->td.layers.size()-1, i;
	float* pB[6];	TerrainLayerBlendMap* bMap[6];
	int t = terrain->getLayerBlendMapSize(), x,y;
	//LogO(String("Ter blendmap size: ")+toStr(t));
	const float f = 0.8f / t * 2 * sc->td.fTerWorldSize / t * 3.14f;  //par-

	//  mtr map
	#ifndef ROAD_EDITOR
	delete[] blendMtr;  blendMtr = 0;
	blendMtr = new char[t*t];
	#endif

	for (i=0; i < b; ++i)
	{	bMap[i] = terrain->getLayerBlendMap(i+1);  pB[i] = bMap[i]->getBlendPointer();  }
	float *fHmap = terrain->getHeightData();

	Math math;
	#define sin_(a)  math.Sin(a,true)
	#define cos_(a)  math.Cos(a,true)
	#define m01(v)  std::max(0.f, std::min(1.f, v ))
	
	//  default layer params
	Real val[5], aMin[5],aMax[5],aSm[5], hMin[5],hMax[5],hSm[5], noise[5];  bool bNOnly[5];
	for (i=0; i < 5; ++i)
	{	val[i]=0.f;  aMin[i]=0.f; aMax[i]=90.f;  aSm[i]=5.f;  hSm[i]=20.f;  bNOnly[i]=1;
		hMin[i]=-300.f; hMax[i]=300.f;  noise[i]=1.f;  }
	
	//  params from layers
	for (i=0; i < std::min(5, (int)sc->td.layers.size()); ++i)
	{
		const TerLayer& l = sc->td.layersAll[sc->td.layers[i]];
		aMin[i] = l.angMin;	aMax[i] = l.angMax;
		hMin[i] = l.hMin;	hMax[i] = l.hMax;  noise[i] = l.noise;
		aSm[i] = l.angSm;	hSm[i] = l.hSm;    bNOnly[i] = l.bNoiseOnly;
	}
	
	//  fill blendmap  ---------------
	int w = sc->td.iTerSize;
	float ft = t, fw = w;
	int xB, yB, xE, yE;
	if (full)
	{	xB = 0;  yB = 0;
		xE = t;  yE = t;
	}else{
		xB = xb / fw * ft;  yB = yb / fw * ft;
		xE = xe / fw * ft;  yE = ye / fw * ft;
	}
	for (y = yB; y < yE; ++y)  {  int aa = y*t + xB, bb = (t-1-y)*t + xB;
	for (x = xB; x < xE; ++x,++aa,++bb)
	{
		//float fx = f*x*0.2, fy = f*y*0.4;	//  val,val1:  0 0 - [0]   1 0  - [1]   0 1 - [2]
		//Real p = (b >= 4) ? 3.f : ( (b >= 3) ? 2.f : 1.f );  p += 3;  //test
		float fx = f*x, fy = f*y;	//  val,val1:  0 0 - [0]   1 0  - [1]   0 1 - [2]
		const Real p = (b >= 4) ? 3.f : ( (b >= 3) ? 2.f : 1.f ), q = 1.f;
		if (b >= 1)  val[0] =                      pow(0.5f + 0.5f *sin_(24.f* fx)*cos_(24.f* fy), p);
		if (b >= 2)  val[1] = std::max(0.f, (float)pow(0.5f + 0.5f *cos_(18.f* fy)*sin_(18.f* fx), p) - val[0]);
		if (b >= 3)  val[2] = std::max(0.f, (float)   (0.5f + 0.5f *cos_(22.f* fy)*sin_(21.f* fx)   ) - val[0]-val[1]);
		if (b >= 4)  val[3] = std::max(0.f, (float)   (0.5f + 0.5f *cos_(19.f* fy)*sin_(20.f* fx)   ) - val[0]-val[1]-val[2]);
		// todo: noise par is only working on [1] mul val[i] *= ...

		//  ter angle and height ranges
		#if 1
		int tx = (float)(x)/ft * w, ty = (float)(y)/ft * w, tt = ty * w + tx;
		float a = sc->td.hfAngle[tt], h = fHmap[tt];  // sc->td.hfHeight[tt];
		for (i=0; i < b; ++i)  if (!bNOnly[i]) {  const int i1 = i+1;
			val[i] = m01( val[i1]*noise[i] + linRange(a,aMin[i1],aMax[i1],aSm[i1]) * linRange(h,hMin[i1],hMax[i1],hSm[i1]) );  }
		#endif

		char mtr = 1;
		for (i=0; i < b; ++i)
		{	*(pB[i]+bb) = val[i];  if (val[i] > 0.5f)  mtr = i+2;  }

		#ifndef ROAD_EDITOR
		blendMtr[aa] = mtr;
		#endif
	}	}
	
	for (i=0; i < b; ++i)
	{
		if (full)	bMap[i]->dirty();
		else		bMap[i]->dirtyRect(Rect(xB,t-yE,xE,t-yB));  //t-1? max(0,
		bMap[i]->update();
	}
	
	iBlendMaps = b+1;  blendMapSize = t;

	//terrain->getLayerBlendTexture(
	//bMap[i]->loadImage();
	//bMap[0]->loadImage("blendmap.png", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	//bMap[0]->loadImage("mapB.jpg", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	//bMap[0]->dirty();  bMap[0]->update();
	/*Image bl0;  // ?-
	terrain->getLayerBlendTexture(0)->convertToImage(bl0);
	bl0.save("blendmap.png");/**/
	//terrain->getCompositeMapMaterial
	
	/*// set up a colour map
	if (!terrain->getGlobalColourMapEnabled())
	{
		terrain->setGlobalColourMapEnabled(true);
		Image colourMap;
		colourMap.load("testcolourmap.jpg", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		terrain->getGlobalColourMap()->loadImage(colourMap);
	}*/
	
	#ifndef ROAD_EDITOR  // game
	ti.update();  /// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Blendmap: ") + toStr(dt) + " ms");
	#endif
}


///  Hmap angles  .....
void App::GetTerAngles(int xb,int yb, int xe,int ye, bool full)
{
	int wx = sc->td.iVertsX, wy = sc->td.iVertsY;
	int xB,xE, yB,yE;
	if (full)
	{	xB = 1;  xE = wx-1;
		yB = 1;  yE = wy-1;
	}else
	{	xB = std::max(1,xb);  xE = std::min(wx-1,xe);
		yB = std::max(1,yb);  yE = std::min(wy-1,ye);
	}
	float* hf = terrain ? terrain->getHeightData() : sc->td.hfHeight;

	Real t = sc->td.fTriangleSize * 2.f;
	for (int j = yB; j < yE; ++j)  // 1 from borders
	{
		int a = j * wx + xB;
		for (int i = xB; i < xE; ++i,++a)
		{
			Vector3 vx(t, hf[a+1] - hf[a-1], 0);  // x+1 - x-1
			Vector3 vz(0, hf[a+wx] - hf[a-wx], t);	// z+1 - z-1
			Vector3 norm = -vx.crossProduct(vz);  norm.normalise();
			Real ang = Math::ACos(norm.y).valueDegrees();

			sc->td.hfAngle[a] = ang;
		}
	}
	if (!full)  return;
	
	//  only corner[] vals
	//sc->td.hfNorm[0] = 0.f;
	
	//  only border[] vals  todo: like above
	for (int j=0; j < wy; ++j)  // |
	{	int a = j * wx;
		sc->td.hfAngle[a + wx-1] = 0.f;
		sc->td.hfAngle[a] = 0.f;
	}
	int a = (wy-1) * wx;
	for (int i=0; i < wx; ++i,++a)  // --
	{	sc->td.hfAngle[i] = 0.f;
		sc->td.hfAngle[a] = 0.f;
	}
}


///  Setup Terrain
//--------------------------------------------------------------------------------------------------------------------------
void App::UpdTerErr()
{
	if (mTerrainGlobals)
		mTerrainGlobals->setMaxPixelError(pSet->terdetail * sc->td.errorNorm);  // 1.7 ..20
}

void App::configureTerrainDefaults(Light* l)
{
	TerrainMaterialGeneratorPtr matGen;
	TerrainMaterial* matGenP = new TerrainMaterial();
	matGen.bind(matGenP);

	mTerrainGlobals->setDefaultMaterialGenerator(matGen);
	UpdTerErr();

	//mTerrainGlobals->setUseRayBoxDistanceCalculation(true);
	//mTerrainGlobals->getDefaultMaterialGenerator()->setDebugLevel(1);
	if (l)  {
	mTerrainGlobals->setLightMapDirection(l->getDerivedDirection());
	mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
	mTerrainGlobals->setCompositeMapDiffuse(l->getDiffuseColour());
	}
	mTerrainGlobals->setCompositeMapSize(sc->td.iTerSize-1);  // par, ..1k
	mTerrainGlobals->setCompositeMapDistance(pSet->terdist);  //400
	mTerrainGlobals->setLightMapSize(ciShadowSizesA[pSet->lightmap_size]);  //256 ..2k
	mTerrainGlobals->setSkirtSize(1);  //`
	//matProfile->setLightmapEnabled(false);

	// Configure default import settings for if we use imported image
	Terrain::ImportData& di = mTerrainGroup->getDefaultImportSettings();
	di.terrainSize = sc->td.iTerSize; // square []-
	di.worldSize = sc->td.fTerWorldSize;  //di.inputScale = td.Hmax;
	//di.minBatchSize = 33;  // 17 33 65 129
	//di.maxBatchSize = std::min(65, sc->td.iTerSize);  // 65 size of one tile in vertices
	// not 33 65^, _65 129 makes less batches
	di.minBatchSize = 65;
	di.maxBatchSize = Terrain::TERRAIN_MAX_BATCH_SIZE;  // 129
	LogO("Terrain size: "+toStr(sc->td.iTerSize)+"  err:"+fToStr(mTerrainGlobals->getMaxPixelError(),2,4)+
		"  batch: "+toStr(di.minBatchSize)+" "+toStr(di.maxBatchSize)+" /"+toStr(Terrain::TERRAIN_MAX_BATCH_SIZE));

	//  textures  iBlendMaps-
	int ls = sc->td.layers.size();
	di.layerList.resize(ls);
	for (int i=0; i < ls; ++i)
	{
		TerLayer& l = sc->td.layersAll[sc->td.layers[i]];
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
	CreateSkyDome(sc->skyMtr, scl);
	UpdFog();

	//  light
	mSceneMgr->destroyAllLights();
	sun = mSceneMgr->createLight("Sun");
	sun->setType(Light::LT_DIRECTIONAL);  UpdSun();


	///  --------  fill HeightField data --------
	if (bTer)
	{
		QTimer ti;  ti.update();  /// time

		int wx = sc->td.iVertsX, wy = sc->td.iVertsY, wxy = wx * wy;  //wy=wx
		delete[] sc->td.hfHeight;  sc->td.hfHeight = new float[wxy];
		delete[] sc->td.hfAngle;   sc->td.hfAngle = new float[wxy];
		int siz = wxy * sizeof(float);

		String name = TrkDir() + (bNewHmap ? "heightmap-new.f32" : "heightmap.f32");

		//  load from f32 HMap +
		{
			std::ifstream fi;
			fi.open(name.c_str(), std::ios_base::binary);
			fi.read((char*)&sc->td.hfHeight[0], siz);
			fi.close();
		}

		GetTerAngles();
		
		ti.update();  /// time
		float dt = ti.dt * 1000.f;
		LogO(String("::: Time Hmap: ") + toStr(dt) + " ms");
	}
	///

	//  Terrain
	if (bTer)
	{
		QTimer tm;  tm.update();  /// time

		if (!mTerrainGlobals)
			mTerrainGlobals = OGRE_NEW TerrainGlobalOptions();
		if (mTerrainGroup)
		{
			OGRE_DELETE mTerrainGroup;
			mTerrainGroup = 0;
		}
		mTerrainGroup = OGRE_NEW TerrainGroup(mSceneMgr, Terrain::ALIGN_X_Z,
			sc->td.iTerSize, sc->td.fTerWorldSize);
		mTerrainGroup->setOrigin(Vector3::ZERO);

		configureTerrainDefaults(sun);

		if (sc->td.hfHeight)
			mTerrainGroup->defineTerrain(0,0, sc->td.hfHeight);
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
			terrain->setVisibilityFlags(RV_Terrain);
		}

		mTerrainGroup->freeTemporaryResources();

		tm.update();	/// time
		float dt = tm.dt * 1000.f;
		LogO(String("::: Time Terrain: ") + toStr(dt) + " ms");
	}
	
	changeShadows();
}

//  Bullet Terrain
//---------------------------------------------------------------------------------------------------------------
void App::CreateBltTerrain()
{
	btHeightfieldTerrainShape* hfShape = new btHeightfieldTerrainShape(
		sc->td.iVertsX, sc->td.iVertsY, sc->td.hfHeight, sc->td.fTriangleSize,
		/*>?*/-200.f,200.f, 2, PHY_FLOAT,false);
	
	hfShape->setUseDiamondSubdivision(true);

	btVector3 scl(sc->td.fTriangleSize, sc->td.fTriangleSize, 1);
	hfShape->setLocalScaling(scl);
	hfShape->setUserPointer((void*)SU_Terrain);

	btCollisionObject* col = new btCollisionObject();
	col->setCollisionShape(hfShape);
	//col->setWorldTransform(tr);
	col->setFriction(0.9);   //+
	col->setRestitution(0.0);
	col->setCollisionFlags(col->getCollisionFlags() |
		btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
	#ifndef ROAD_EDITOR  // game
		pGame->collision.world->addCollisionObject(col);
		pGame->collision.shapes.push_back(hfShape);
	#else
		world->addCollisionObject(col);
	#endif
	
	#ifndef ROAD_EDITOR
	///  border planes []
	const float px[4] = {-1, 1, 0, 0};
	const float py[4] = { 0, 0,-1, 1};

	for (int i=0; i < 4; ++i)
	{
		btVector3 vpl(px[i], py[i], 0);
		btCollisionShape* shp = new btStaticPlaneShape(vpl,0);
		shp->setUserPointer((void*)SU_Border);
		
		btTransform tr;  tr.setIdentity();
		tr.setOrigin(vpl * -0.5 * sc->td.fTerWorldSize);

		btCollisionObject* col = new btCollisionObject();
		col->setCollisionShape(shp);
		col->setWorldTransform(tr);
		col->setFriction(0.3);   //+
		col->setRestitution(0.0);
		col->setCollisionFlags(col->getCollisionFlags() |
			btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);

		pGame->collision.world->addCollisionObject(col);
		pGame->collision.shapes.push_back(shp);
	}
	#endif
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
	float a,b;  const float B = PI_d/2.f, A = 2.f*PI_d;
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
	m->setRenderQueueGroup(RQG_Sky);
	m->setCastShadows(false);
	#ifdef ROAD_EDITOR
	m->setVisibilityFlags(RV_Sky);  // hide on minimap
	#endif

	ndSky = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	ndSky->attachObject(m);
	ndSky->setScale(sc);
}

//  fog
void App::UpdFog(bool bForce)
{
	const ColourValue clr(0.5,0.6,0.7,1);
	bool ok = !pSet->bFog || bForce;
	if (ok)
		mSceneMgr->setFog(FOG_LINEAR, clr, 1.f, sc->fogStart, sc->fogEnd);
	else
		mSceneMgr->setFog(FOG_NONE, clr, 1.f, 9000, 9200);

	mFactory->setSharedParameter("fogColorSun",  sh::makeProperty<sh::Vector4>(new sh::Vector4(sc->fogClr2.x, sc->fogClr2.y, sc->fogClr2.z, sc->fogClr2.w)));
	mFactory->setSharedParameter("fogColorAway", sh::makeProperty<sh::Vector4>(new sh::Vector4(sc->fogClr.x,  sc->fogClr.y,  sc->fogClr.z,  sc->fogClr.w)));
	mFactory->setSharedParameter("fogColorH",    sh::makeProperty<sh::Vector4>(new sh::Vector4(sc->fogClrH.x, sc->fogClrH.y, sc->fogClrH.z, sc->fogClrH.w)));
	mFactory->setSharedParameter("fogParamsH",   sh::makeProperty<sh::Vector4>(new sh::Vector4(
		sc->fogHeight, ok ? 1.f/sc->fogHDensity : 0.f, sc->fogHStart, 1.f/(sc->fogHEnd - sc->fogHStart) )));
}

inline ColourValue Clr3(const Vector3& v)
{
	return ColourValue(v.x, v.y, v.z);
}

void App::UpdSun()
{
	if (!sun)  return;
	Vector3 dir = SplineRoad::GetRot(sc->ldYaw, -sc->ldPitch);
	sun->setDirection(dir);
	sun->setDiffuseColour(Clr3(sc->lDiff));
	sun->setSpecularColour(Clr3(sc->lSpec));
	mSceneMgr->setAmbientLight(Clr3(sc->lAmb));
}


//  Material Factory defaults
//----------------------------------------------------------------------------------------------------------------------
void App::SetFactoryDefaults()
{
	sh::Factory& fct = sh::Factory::getInstance();
	fct.setReadSourceCache(true);
	fct.setWriteSourceCache(true);
	fct.setReadMicrocodeCache(true);
	fct.setWriteMicrocodeCache(true);
	fct.setGlobalSetting("fog", "true");
	fct.setGlobalSetting("wind", "true");
	fct.setGlobalSetting("mrt_output", "false");
	fct.setGlobalSetting("shadows", "false");
	fct.setGlobalSetting("shadows_pssm", "false");
	fct.setGlobalSetting("shadows_depth", b2s(pSet->shadow_type >= Sh_Depth));
	fct.setGlobalSetting("lighting", "true");
	fct.setGlobalSetting("terrain_composite_map", "false");
	fct.setGlobalSetting("soft_particles", "false");
	#ifdef ROAD_EDITOR
	fct.setGlobalSetting("editor", "true");
	#else
	fct.setGlobalSetting("editor", "false");
	#endif

	fct.setSharedParameter("fogColorSun",  sh::makeProperty<sh::Vector4>(new sh::Vector4(0,0,0,0)));
	fct.setSharedParameter("fogColorAway", sh::makeProperty<sh::Vector4>(new sh::Vector4(0,0,0,0)));
	fct.setSharedParameter("fogColorH",    sh::makeProperty<sh::Vector4>(new sh::Vector4(0,0,0,0)));
	fct.setSharedParameter("fogParamsH",   sh::makeProperty<sh::Vector4>(new sh::Vector4(0,0,0,0)));

	fct.setSharedParameter("pssmSplitPoints", sh::makeProperty<sh::Vector3>(new sh::Vector3(0,0,0)));
	fct.setSharedParameter("shadowFar_fadeStart", sh::makeProperty<sh::Vector4>(new sh::Vector4(0,0,0,0)));
	fct.setSharedParameter("arrowColour1", sh::makeProperty <sh::Vector3>(new sh::Vector3(0,0,0)));
	fct.setSharedParameter("arrowColour2", sh::makeProperty <sh::Vector3>(new sh::Vector3(0,0,0)));
	fct.setSharedParameter("windTimer", sh::makeProperty <sh::FloatValue>(new sh::FloatValue(0)));
	fct.setSharedParameter("posSph0", sh::makeProperty <sh::Vector4>(new sh::Vector4(0,500,0,-1)));
	fct.setSharedParameter("posSph1", sh::makeProperty <sh::Vector4>(new sh::Vector4(0,500,0,-1)));
	fct.setSharedParameter("terrainWorldSize", sh::makeProperty <sh::FloatValue>(new sh::FloatValue(1024)));
	fct.setSharedParameter("waterDepth", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(1.0)));

	fct.setGlobalSetting("terrain_specular", b2s(pSet->ter_mtr >= 1));
	fct.setGlobalSetting("terrain_normal",   b2s(pSet->ter_mtr >= 2));
	fct.setGlobalSetting("terrain_parallax", b2s(pSet->ter_mtr >= 3));
	fct.setGlobalSetting("terrain_triplanarType", toStr(pSet->ter_tripl));
	fct.setGlobalSetting("terrain_triplanarLayer", toStr(sc->td.triplanar1Layer));

	fct.setGlobalSetting("water_reflect", b2s(pSet->water_reflect));
	fct.setGlobalSetting("water_refract", b2s(pSet->water_refract));
	fct.setSharedParameter("waterEnabled", sh::makeProperty<sh::FloatValue> (new sh::FloatValue(0.0)));
	fct.setSharedParameter("waterLevel", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(0)));
	fct.setSharedParameter("waterTimer", sh::makeProperty<sh::FloatValue>(new sh::FloatValue(0)));
	fct.setSharedParameter("waterSunFade_sunHeight", sh::makeProperty<sh::Vector2>(new sh::Vector2(1, 0.6)));
	fct.setSharedParameter("windDir_windSpeed", sh::makeProperty<sh::Vector3>(new sh::Vector3(0.5, -0.8, 0.2)));


	///  uncomment to enable shader output to files
	//mFactory->setShaderDebugOutputEnabled(true);

	sh::Language lang;
	if (pSet->shader_mode == "")
	{
	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		lang = sh::Language_HLSL;
	#else
		lang = sh::Language_GLSL;
	#endif
	}else
	{	     if (pSet->shader_mode == "glsl") lang = sh::Language_GLSL;
		else if (pSet->shader_mode == "cg")	  lang = sh::Language_CG;
		else if (pSet->shader_mode == "hlsl") lang = sh::Language_HLSL;
		else  assert(0);
	}
	mFactory->setCurrentLanguage(lang);

	mFactory->loadAllFiles();
}
