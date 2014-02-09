#include "pch.h"
#include "../common/Def_Str.h"
#include "../common/RenderConst.h"
#include "../common/ShapeData.h"
#include "../common/QTimer.h"
#include "../common/GuiCom.h"
#include "TerrainMaterial.h"
#include "../vdrift/pathmanager.h"
#ifdef SR_EDITOR
	#include "../../editor/CApp.h"
	#include "../../editor/settings.h"
	#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"
#else
	#include "../CGame.h"
	#include "../../vdrift/settings.h"
#endif
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
using namespace Ogre;


///  Setup Terrain
//---------------------------------------------------------------------------------------------------------------
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

	mTerrainGlobals->setLayerBlendMapSize(4);  // we use our own rtt, so reduce this
	mTerrainGlobals->setLightMapDirection(l->getDerivedDirection());
	mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
	mTerrainGlobals->setCompositeMapDiffuse(l->getDiffuseColour());

	mTerrainGlobals->setCompositeMapSize(sc->td.iTerSize-1);  // par, ..1k
	mTerrainGlobals->setCompositeMapDistance(pSet->terdist);  //400
	mTerrainGlobals->setLightMapSize(ciShadowSizesA[pSet->lightmap_size]);  //256 ..2k
	mTerrainGlobals->setSkirtSize(1);  // low because in water reflect

	//  import settings
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

		#if 1  // new, combine rgb,a
		String pt = PATHMANAGER::Data()+"/terrain",
			pt2=pt+"2/", p;  pt+="/";
		
		//  diff
		p = PATHMANAGER::FileExists(pt2+ l.texFile) ? pt2 : pt;
		String tx_d, tx_s;		
		if (!StringUtil::match(l.texFile, "*_d.*", false))
		{	//  old no _d
			tx_d = StringUtil::replaceAll(l.texFile,".","_d.");
			tx_s = StringUtil::replaceAll(l.texFile,".","_s.");
		}else
		{	tx_d = l.texFile;  // new with _d
			tx_s = StringUtil::replaceAll(l.texFile,"_d.","_s.");
		}
		//if (PATHMANAGER::FileExists(p+ tx_s))
			texLayD[i].LoadTer(p+ tx_d, p+ tx_s, 0.f);
		//else
		//	texLayD[i].LoadTer(p+ tx_d, pt2+ "flat_s.png");

		//  norm
		bool fl = l.texNorm == "flat_n.png";
		p = fl||PATHMANAGER::FileExists(pt2+ l.texNorm) ? pt2 : pt;
		String n_n, n_h;
		if (fl||StringUtil::match(l.texFile, "*_nh.*", false))
		{	//  old _nh
			n_n = fl ? "flat_n.png" : StringUtil::replaceAll(l.texNorm,"_nh.","_n.");
			n_h = fl ? "flat_h.png" : StringUtil::replaceAll(l.texNorm,"_nh.","_h.");
		}else
		{	n_n = l.texNorm;  // new with _n
			n_h = StringUtil::replaceAll(l.texNorm,"_n.","_h.");
		}
		//if (PATHMANAGER::FileExists(p+ n_h))
			texLayN[i].LoadTer(p+ n_n, p+ n_h, 1.f);
		//else
		//	texLayN[i].LoadTer(p+ n_n, pt2+ "flat_h.png");
		
		di.layerList[i].textureNames.push_back("layD"+toStr(i));
		di.layerList[i].textureNames.push_back("layN"+toStr(i));
		#else  // old
		di.layerList[i].textureNames.push_back(l.texFile);
		di.layerList[i].textureNames.push_back(l.texNorm);
		#endif
	}
}

void App::configureHorizonDefaults(Ogre::Light* l)
{
	TerrainMaterialGeneratorPtr matGen;
	TerrainMaterial* matGenP = new TerrainMaterial();
	matGen.bind(matGenP);

	mHorizonGlobals->setDefaultMaterialGenerator(matGen);
	mTerrainGlobals->setMaxPixelError(pSet->terdetail * sc->td.errorNorm);  // 1.7 ..20

	mHorizonGlobals->setLightMapDirection(l->getDerivedDirection());
	mHorizonGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
	mHorizonGlobals->setCompositeMapDiffuse(l->getDiffuseColour());

	mHorizonGlobals->setCompositeMapSize(sc->td.iTerSize-1);  // par, 128,256 ..
	mHorizonGlobals->setCompositeMapDistance(pSet->terdist);  //..
	mHorizonGlobals->setLightMapSize(ciShadowSizesA[pSet->lightmap_size]);  //256 ..2k
	mHorizonGlobals->setSkirtSize(1);
	//matProfile->setLightmapEnabled(false);

	Terrain::ImportData& di = mHorizonGroup->getDefaultImportSettings();
	di.terrainSize = sc->td.iTerSize;
	di.worldSize = sc->td.fTerWorldSize;
	di.minBatchSize = 65;
	di.maxBatchSize = Terrain::TERRAIN_MAX_BATCH_SIZE;  // 129
	LogO("Horizon size: "+toStr(sc->td.iTerSize)+"  err:"+fToStr(mHorizonGlobals->getMaxPixelError(),2,4)+
		"  batch: "+toStr(di.minBatchSize)+" "+toStr(di.maxBatchSize));

	//  textures
	int ls = 1; //sc->td.layers.size();
	di.layerList.resize(ls);
	for (int i=0; i < ls; ++i)
	{
		TerLayer& l = sc->td.layersAll[sc->td.layers[i]];
		di.layerList[i].worldSize = l.tiling * 16.f;
		di.layerList[i].textureNames.push_back(l.texFile);
		di.layerList[i].textureNames.push_back(l.texNorm);
	}
}


///--------------------------------------------------------------------------------------------------------------
//  Create Terrain
///--------------------------------------------------------------------------------------------------------------
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
		const int size = wxy * sizeof(float);

		String name = gcom->TrkDir() + (bNewHmap ? "heightmap-new.f32" : "heightmap.f32");

		//  load from f32 HMap +
		{
			std::ifstream fi;
			fi.open(name.c_str(), std::ios_base::binary);
			fi.read((char*)&sc->td.hfHeight[0], size);
			fi.close();
		}

		CreateBlendTex();  //+

		GetTerAngles();
		
		ti.update();  /// time
		float dt = ti.dt * 1000.f;
		LogO(String("::: Time Hmap: ") + fToStr(dt,0,3) + " ms");
	}
	///

	//  Terrain
	if (bTer)
	{
		QTimer tm;  tm.update();  /// time

		if (!mTerrainGlobals)
			mTerrainGlobals = OGRE_NEW TerrainGlobalOptions();
		OGRE_DELETE mTerrainGroup;

		mTerrainGroup = OGRE_NEW TerrainGroup(mSceneMgr, Terrain::ALIGN_X_Z,
			sc->td.iTerSize, sc->td.fTerWorldSize);
		mTerrainGroup->setOrigin(Vector3::ZERO);

		configureTerrainDefaults(sun);

		if (sc->td.hfHeight)
			mTerrainGroup->defineTerrain(0,0, sc->td.hfHeight);
		else
			mTerrainGroup->defineTerrain(0,0, 0.f);

		//  sync load since we want everything in place when we start
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

	//  Horizon  ----------
	if (pSet->horizon)
	{
		if (!mHorizonGlobals)
			mHorizonGlobals = OGRE_NEW TerrainGlobalOptions();
		OGRE_DELETE mHorizonGroup;

		mHorizonGroup = OGRE_NEW TerrainGroup(mSceneMgr, Terrain::ALIGN_X_Z,
			sc->td.iTerSize, sc->td.fTerWorldSize * 16.f);
		mHorizonGroup->setOrigin(Vector3::ZERO);

		configureHorizonDefaults(sun);

		//if (sc->td.hfHeight)
		//	mHorizonGroup->defineTerrain(0,0, sc->td.hfHeight);
		//else
			mHorizonGroup->defineTerrain(0,0, 0.f);

		mHorizonGroup->loadAllTerrains(true);

		ti = mHorizonGroup->getTerrainIterator();
		while (ti.hasMoreElements())
		{
			Terrain* t = ti.getNext()->instance;
			//initBlendMaps(t);
			horizon = t;  //<set
			horizon->setVisibilityFlags(RV_Terrain);
		}
		mHorizonGroup->freeTemporaryResources();
	}

		tm.update();	/// time
		float dt = tm.dt * 1000.f;
		LogO(String("::: Time Terrain: ") + fToStr(dt,0,3) + " ms");
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
	#ifndef SR_EDITOR  // game
		pGame->collision.world->addCollisionObject(col);
		pGame->collision.shapes.push_back(hfShape);
	#else
		world->addCollisionObject(col);
	#endif
	
	#ifndef SR_EDITOR
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
