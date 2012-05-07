#include "pch.h"
#include "../common/Defines.h"
#include "../common/RenderConst.h"
#include "../../road/Road.h"  // sun rot
#include "../common/MaterialGen/TerrainMaterialGen.h"
#include "../common/MaterialGen/MaterialFactory.h"

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
inline float linRange(const float& x, const float& xa, const float& xb, const float& s)  // min, max, smooth range
//     xa  xb
//1    .___.
//0__./     \.___
//   xa-s    xb+s
{
	//float r = std::max(0.1f, xb-xa);  // range
	if (x <= xa-s || x >= xb+s)  return 0.f;
	if (x >= xa && x <= xb)  return 1.f;
	if (x < xa)  return (x-xa)/s+1;
	if (x > xb)  return (xb-x)/s+1;
	return 0.f;
}

void App::initBlendMaps(Ogre::Terrain* terrain)
{
	QTimer ti;  ti.update();  /// time
	//for (float f=-1.f; f<=2.f; f+=0.02f)  // test
	//	LogO(toStr(f) + " = " + toStr( linRange(f,-0.5f,1.5f,0.2f) ));

	int b = sc.td.layers.size()-1, i;
	float* pB[6];	TerrainLayerBlendMap* bMap[6];
	Ogre::uint16 t = terrain->getLayerBlendMapSize(), x,y;
	//LogO(String("Ter blendmap size: ")+toStr(t));
	//const float f = 0.8f / t * 3.14f;  //par-
	const float f = 0.8f / t * 2 * sc.td.fTerWorldSize / t * 3.14f;  //par-

	//  mtr map
	#ifndef ROAD_EDITOR
	delete[] blendMtr;  blendMtr = 0;
	blendMtr = new char[t*t];
	#endif

	for (i=0; i < b; ++i)  {
		bMap[i] = terrain->getLayerBlendMap(i+1);  pB[i] = bMap[i]->getBlendPointer();  }

	//#define sin_(a)  sinf(a)
	//#define cos_(a)  sinf(a)
	/**/
	
	Math math;
	#define sin_(a)  math.Sin(a,true)
	#define cos_(a)  math.Cos(a,true)
	#define m01(v)  std::max(0.f, std::min(1.f, v ))
	
	//  params from layers
	Real val[5], aMin[5],aMax[5],aSm[5], hMin[5],hMax[5],hSm[5], noise[5];  bool bNOnly[5];
	for (i=0; i < 5; ++i)  //-
	{	val[i]=0.f;  aMin[i]=0.f; aMax[i]=90.f;  aSm[i]=5.f;  hSm[i]=20.f;  bNOnly[i]=1;
		hMin[i]=-300.f; hMax[i]=300.f;  noise[i]=1.f;  }
	
	for (i=0; i < std::min(5, (int)sc.td.layers.size()); ++i)
	{
		const TerLayer& l = sc.td.layersAll[sc.td.layers[i]];
		aMin[i] = l.angMin;	aMax[i] = l.angMax;
		hMin[i] = l.hMin;	hMax[i] = l.hMax;  noise[i] = l.noise;
		aSm[i] = l.angSm;	hSm[i] = l.hSm;    bNOnly[i] = l.bNoiseOnly;
	}
	
	//  fill blendmap  ---------------
	float ft = t;  int w = sc.td.iTerSize;
	for (y = 0; y < t; ++y)  {  int aa = y*t;
	for (x = 0; x < t; ++x,++aa)
	{
		float fx = f*x, fy = f*y;	//  val,val1:  0 0 - [0]   1 0  - [1]   0 1 - [2]
		const Real p = (b >= 4) ? 3.f : ( (b >= 3) ? 2.f : 1.f ), q = 1.f;
		if (b >= 1)  val[0] =                      pow(0.5f + 0.5f *sin_(24.f* fx)*cos_(24.f* fy), p);
		if (b >= 2)  val[1] = std::max(0.f, (float)pow(0.5f + 0.5f *cos_(18.f* fy)*sin_(18.f* fx), p) - val[0]);
		if (b >= 3)  val[2] = std::max(0.f, (float)   (0.5f + 0.5f *cos_(22.f* fy)*sin_(21.f* fx)   ) - val[0]-val[1]);
		if (b >= 4)  val[3] = std::max(0.f, (float)   (0.5f + 0.5f *cos_(19.f* fy)*sin_(20.f* fx)   ) - val[0]-val[1]-val[2]);
		// todo: noise par is only working on [1] mul val[i] *= ...

		//  ter angle and height ranges
		#if 1
		int tx = (float)(x)/ft * w, ty = (float)(t-1-y)/ft * w, tt = ty * w + tx;
		float a = sc.td.hfAngle[tt], h = sc.td.hfHeight[tt];
		for (i=0; i < b; ++i)  if (!bNOnly[i]) {  const int i1 = i+1;
			val[i] = m01( val[i1]*noise[i] + linRange(a,aMin[i1],aMax[i1],aSm[i1]) * linRange(h,hMin[i1],hMax[i1],hSm[i1]) );  }
		#endif

		char mtr = 1;
		for (i=0; i < b; ++i)
		{	*(pB[i])++ = val[i];  if (val[i] > 0.5f)  mtr = i+2;  }

		#ifndef ROAD_EDITOR
		blendMtr[aa] = mtr;
		#endif
	}	}
	
	for (i=0; i < b; ++i)  {
		//bMap[i]->dirtyRect();
		bMap[i]->dirty();  bMap[i]->update();  
//		bMap[i]->
	}
	
	/**/
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
	}
	*/
	ti.update();  /// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Blendmap: ") + toStr(dt) + " ms");
}


///  Hmap angles  .....
void App::GetTerAngles(int xb,int yb, int xe,int ye)
{
	int wx = sc.td.iVertsX, wy = sc.td.iVertsY;
	bool full = (xe-xb == wx-1) && (ye-yb == wy-1);
	int xB = std::max(1,xb), xE = std::min(wx-1,xe);
	int yB = std::max(1,yb), yE = std::min(wy-1,ye);
	float* hf = terrain ? terrain->getHeightData() : sc.td.hfHeight;

	Real t = sc.td.fTriangleSize * 2.f;
	terMaxAng = 0.f;
	for (int j = yB; j < yE; ++j)  // 1 from borders
	{
		int a = j * wx + xB;
		for (int i = xB; i < xE; ++i,++a)
		{
			Vector3 vx(t, hf[a+1] - hf[a-1], 0);  // x+1 - x-1
			Vector3 vz(0, hf[a+wx] - hf[a-wx], t);	// z+1 - z-1
			Vector3 norm = -vx.crossProduct(vz);  norm.normalise();
			Real ang = Math::ACos(norm.y).valueDegrees();

			sc.td.hfAngle[a] = ang;
			if (ang > terMaxAng)  terMaxAng = ang;
			//if (i==j)
			//	LogO(toStr(sc.td.hfNorm[a]));
		}
	}
	if (!full)  return;
	//  only corner[] vals
	//sc.td.hfNorm[0] = 0.f;
	//  only border[] vals  todo: like above
	for (int j=0; j < wy; ++j)  // |
	{	int a = j * wx;
		sc.td.hfAngle[a + wx-1] = 0.f;
		sc.td.hfAngle[a] = 0.f;
	}
	int a = (wy-1) * wx;
	for (int i=0; i < wx; ++i,++a)  // --
	{	sc.td.hfAngle[i] = 0.f;
		sc.td.hfAngle[a] = 0.f;
	}
	//LogO(String("Terrain max angle: ") + toStr(terMaxAng));
}


///  Setup Terrain
//--------------------------------------------------------------------------------------------------------------------------
void App::configureTerrainDefaults(Light* l)
{
	//TerrainMaterialGeneratorPtr matGen = static_cast<TerrainMaterialGeneratorPtr>(new TerrainMaterialGeneratorB());
	TerrainMaterialGeneratorPtr matGen;
	TerrainMaterialGeneratorB* matGenP = new TerrainMaterialGeneratorB();
	matGen.bind(matGenP);
	mTerrainGlobals->setDefaultMaterialGenerator(matGen);

	mTerrainGlobals->setMaxPixelError(pSet->terdetail);  // 1- 4-8+
	//mTerrainGlobals->setUseRayBoxDistanceCalculation(true);
	//mTerrainGlobals->getDefaultMaterialGenerator()->setDebugLevel(1);
	if (l)  {
	mTerrainGlobals->setLightMapDirection(l->getDerivedDirection());
	mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
	mTerrainGlobals->setCompositeMapDiffuse(l->getDiffuseColour());  }
	//mTerrainGlobals->setShadowVal(0.6f);  //+ new, for compositeMap shadow

	mTerrainGlobals->setCompositeMapSize(sc.td.iTerSize-1);  // par,..  1k
	mTerrainGlobals->setCompositeMapDistance(pSet->terdist);  //100
	mTerrainGlobals->setLightMapSize(ciShadowSizesA[pSet->lightmap_size]);  //256, 2k
	mTerrainGlobals->setSkirtSize(MaterialFactory::getSingleton().getReflect() ? 0.1 : 1);  //`
	//matProfile->setLightmapEnabled(false);

	// Configure default import settings for if we use imported image
	Terrain::ImportData& di = mTerrainGroup->getDefaultImportSettings();
	di.terrainSize = sc.td.iTerSize; // square []-
	di.worldSize = sc.td.fTerWorldSize;  //di.inputScale = td.Hmax;
	di.minBatchSize = 33; //17;   //17 o: 33   3 5 9 17 33 65 129
	di.maxBatchSize = std::min(65, sc.td.iTerSize); //65;  //65 size of one tile in vertices
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
		QTimer ti;  ti.update();  /// time

		int wx = sc.td.iVertsX, wy = sc.td.iVertsY, wxy = wx * wy;  //wy=wx
		delete[] sc.td.hfHeight;  sc.td.hfHeight = new float[wxy];
		delete[] sc.td.hfAngle;   sc.td.hfAngle = new float[wxy];
		int siz = wxy * sizeof(float);

		String name = TrkDir() + (bNewHmap ? "heightmap-new.f32" : "heightmap.f32");

		//  load from f32 HMap +
		{
			std::ifstream fi;
			fi.open(name.c_str(), std::ios_base::binary);
			fi.read((char*)&sc.td.hfHeight[0], siz);
			fi.close();
		}

		GetTerAngles(1,1,wx-1,wy-1);
		
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
			sc.td.iTerSize, sc.td.fTerWorldSize);
		mTerrainGroup->setOrigin(Vector3::ZERO);

		configureTerrainDefaults(sun);

		if (sc.td.hfHeight)
			mTerrainGroup->defineTerrain(0,0, sc.td.hfHeight);
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
		sc.td.iVertsX, sc.td.iVertsY, sc.td.hfHeight, sc.td.fTriangleSize,
		/*>?*/-200.f,200.f, 2, PHY_FLOAT,false);
	
	hfShape->setUseDiamondSubdivision(true);

	btVector3 scl(sc.td.fTriangleSize, sc.td.fTriangleSize, 1);
	hfShape->setLocalScaling(scl);
	//col->setUserPointer((void*)&gSD_Terrain);

	/*btRigidBody::btRigidBodyConstructionInfo infoHm(0.f, 0, hfShape);
	infoHm.m_restitution = 0.5;  //
	infoHm.m_friction = 0.9;  ///.. 0.9~
	pGame->collision.AddRigidBody(infoHm);/**/

	btCollisionObject* col = new btCollisionObject();
	col->setCollisionShape(hfShape);
	//col->setWorldTransform(tr);
	col->setFriction(0.9);
	col->setRestitution(0.5);
	col->setCollisionFlags(col->getCollisionFlags() |
		btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
	#ifndef ROAD_EDITOR  // game
		pGame->collision.world->addCollisionObject(col);
		pGame->collision.shapes.push_back(hfShape);/**/
	#else
		world->addCollisionObject(col);
	#endif
	
	#ifndef ROAD_EDITOR
	///  border planes []
	const float px[4] = {-1, 1, 0, 0};
	const float py[4] = { 0, 0,-1, 1};

	for (int i=0; i < 4; i++)
	{
		btVector3 vpl(px[i], py[i], 0);
		btCollisionShape* shp = new btStaticPlaneShape(vpl,0);
		//shp->setUserPointer(gSD_BorderPlane);
		
		btTransform tr;  tr.setIdentity();
		tr.setOrigin(vpl * -0.5 * sc.td.fTerWorldSize);

		btDefaultMotionState* ms = new btDefaultMotionState(tr);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0.f,ms,shp);
		pGame->collision.AddRigidBody(rbInfo);
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

inline ColourValue Clr3(const Vector3& v)
{
	return ColourValue(v.x, v.y, v.z);
}

void App::UpdFog(bool bForce)
{
	ColourValue clr = Clr3(sc.fogClr);
	if (!pSet->bFog || bForce)
		mSceneMgr->setFog(sc.fogMode, clr, 1.f, sc.fogStart, sc.fogEnd);
	else
		mSceneMgr->setFog(sc.fogMode, clr, 1.f, 3000, 3200);
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
