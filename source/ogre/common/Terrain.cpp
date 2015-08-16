#include "pch.h"
#include "../common/Def_Str.h"
#include "../common/RenderConst.h"
#include "../common/ShapeData.h"
#include "../common/GuiCom.h"
#include "../common/CScene.h"
#include "../common/data/SceneXml.h"
#include "TerrainMaterial.h"
#include "../vdrift/pathmanager.h"
#ifdef SR_EDITOR
	#include "../../editor/CApp.h"
	#include "../../editor/settings.h"
	#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#else
	#include "../CGame.h"
	#include "../../vdrift/game.h"
	#include "../../vdrift/settings.h"
#endif
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <OgreTimer.h>
#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
using namespace Ogre;


///  Setup Terrain
//---------------------------------------------------------------------------------------------------------------
void CScene::UpdTerErr()
{
	if (mTerrainGlobals)
		mTerrainGlobals->setMaxPixelError(app->pSet->terdetail * sc->td.errorNorm);  // 1.7 ..20
}

void CScene::SetupTerrain()
{
	TerrainMaterialGeneratorPtr matGen;
	TerrainMaterial* matGenP = new TerrainMaterial(this);
	matGen.bind(matGenP);

	mTerrainGlobals->setDefaultMaterialGenerator(matGen);
	UpdTerErr();

	mTerrainGlobals->setLayerBlendMapSize(4);  // we use our own rtt, so reduce this
	mTerrainGlobals->setLightMapDirection(sun->getDerivedDirection());
	mTerrainGlobals->setCompositeMapAmbient(app->mSceneMgr->getAmbientLight());
	mTerrainGlobals->setCompositeMapDiffuse(sun->getDiffuseColour());

	mTerrainGlobals->setCompositeMapSize(sc->td.iTerSize-1);  // par, ..1k
	mTerrainGlobals->setCompositeMapDistance(app->pSet->terdist);  //400
	mTerrainGlobals->setLightMapSize(ciShadowSizesA[app->pSet->lightmap_size]);  //256 ..2k
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

	//terrain->getCompositeMapMaterial
	/*// set up a colour map
	if (!terrain->getGlobalColourMapEnabled())
	{
		terrain->setGlobalColourMapEnabled(true);
		Image colourMap;
		colourMap.load("colormap.jpg", rgDef);
		terrain->getGlobalColourMap()->loadImage(colourMap);
	}/**/


	///  layer textures
	int ls = sc->td.layers.size();
	di.layerList.resize(ls);
	for (int i=0; i < ls; ++i)
	{
		TerLayer& l = sc->td.layersAll[sc->td.layers[i]];
		di.layerList[i].worldSize = l.tiling;

		//  combined rgb,a from 2 tex
		String p = PATHMANAGER::Data() + (app->pSet->tex_size > 0 ? "/terrain/" : "/terrain_s/");
		String d_d, d_s, n_n, n_h;
		
		///  diff
		d_d = l.texFile;  // ends with _d
		d_s = StringUtil::replaceAll(l.texFile,"_d.","_s.");

		if (!PATHMANAGER::FileExists(p+ d_d))
			texLayD[i].LoadTer(p+ "grass_green_d.jpg", p+ "grass_green_n.jpg", 0.f);
		else
		if (PATHMANAGER::FileExists(p+ d_s))
			texLayD[i].LoadTer(p+ d_d, p+ d_s, 0.f);
		else  // use _s from norm tex name
		{	d_s = StringUtil::replaceAll(l.texNorm,"_n.","_s.");
			texLayD[i].LoadTer(p+ d_d, p+ d_s, 0.f);
		}
		///  norm
		n_n = l.texNorm;  // ends with _n
		n_h = StringUtil::replaceAll(l.texNorm,"_n.","_h.");

		if (PATHMANAGER::FileExists(p+ n_n))
			texLayN[i].LoadTer(p+ n_n, p+ n_h, 1.f);
		else
			texLayN[i].LoadTer(p+ "flat_n.png", p+ n_h, 1.f);
		
		di.layerList[i].textureNames.push_back("layD"+toStr(i));
		di.layerList[i].textureNames.push_back("layN"+toStr(i));
	}
}

void CScene::SetupHorizon()
{
	TerrainMaterialGeneratorPtr matGen;
	TerrainMaterial* matGenP = new TerrainMaterial(this);
	matGen.bind(matGenP);

	mHorizonGlobals->setDefaultMaterialGenerator(matGen);
	mTerrainGlobals->setMaxPixelError(app->pSet->terdetail * sc->td.errorNorm);  // 1.7 ..20

	mHorizonGlobals->setLightMapDirection(sun->getDerivedDirection());
	mHorizonGlobals->setCompositeMapAmbient(app->mSceneMgr->getAmbientLight());
	mHorizonGlobals->setCompositeMapDiffuse(sun->getDiffuseColour());

	mHorizonGlobals->setCompositeMapSize(sc->td.iTerSize-1);  // par, 128,256 ..
	mHorizonGlobals->setCompositeMapDistance(app->pSet->terdist);  //..
	mHorizonGlobals->setLightMapSize(ciShadowSizesA[app->pSet->lightmap_size]);  //256 ..2k
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
void CScene::CreateTerrain(bool bNewHmap, bool bTer, bool terLoad)
{
	Ogre::Timer tm;
	terrain = 0;

	
	///  sky
	Vector3 scl = app->pSet->view_distance*Vector3::UNIT_SCALE;
	CreateSkyDome(sc->skyMtr, scl, sc->skyYaw);
	UpdFog();

	//  light
	app->mSceneMgr->destroyAllLights();
	sun = app->mSceneMgr->createLight("Sun");
	sun->setType(Light::LT_DIRECTIONAL);  UpdSun();


if (bTer)
{
	///.
	UpdShaderParams();
	UpdLayerPars();
	

	///  --------  fill HeightField data --------
	//Ogre::Timer ti;
	if (terLoad || bNewHmap)
	{
		int wx = sc->td.iVertsX, wy = sc->td.iVertsY, wxy = wx * wy;  //wy=wx
		delete[] sc->td.hfHeight;  sc->td.hfHeight = new float[wxy];
		const int size = wxy * sizeof(float);

		String name = app->gcom->TrkDir() + (bNewHmap ? "heightmap-new.f32" : "heightmap.f32");

		//  load from f32 HMap +
		{
			std::ifstream fi;
			fi.open(name.c_str(), std::ios_base::binary);
			fi.read((char*)&sc->td.hfHeight[0], size);
			fi.close();
		}
	}

	CreateBlendTex();  //+

	//LogO(String("::: Time Hmap: ") + fToStr(ti.getMilliseconds(),0,3) + " ms");  ti.reset();  // 4MB ~13ms


	///.
	UpdBlendmap();
	

	//  Terrain
	{
		if (!mTerrainGlobals)
			mTerrainGlobals = OGRE_NEW TerrainGlobalOptions();
		OGRE_DELETE mTerrainGroup;

		mTerrainGroup = OGRE_NEW TerrainGroup(app->mSceneMgr, Terrain::ALIGN_X_Z,
			sc->td.iTerSize, sc->td.fTerWorldSize);
		mTerrainGroup->setOrigin(Vector3::ZERO);

		SetupTerrain();

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
			terrain = t;  //<set
			terrain->setVisibilityFlags(RV_Terrain);
		}
		mTerrainGroup->freeTemporaryResources();
	}

	//  Horizon  ----------
	if (app->pSet->horizon)
	{
		if (!mHorizonGlobals)
			mHorizonGlobals = OGRE_NEW TerrainGlobalOptions();
		OGRE_DELETE mHorizonGroup;

		mHorizonGroup = OGRE_NEW TerrainGroup(app->mSceneMgr, Terrain::ALIGN_X_Z,
			sc->td.iTerSize, sc->td.fTerWorldSize * 16.f);
		mHorizonGroup->setOrigin(Vector3::ZERO);

		SetupHorizon();

		//if (sc->td.hfHeight)
		//	mHorizonGroup->defineTerrain(0,0, sc->td.hfHeight);
		//else
			mHorizonGroup->defineTerrain(0,0, 0.f);

		mHorizonGroup->loadAllTerrains(true);

		TerrainGroup::TerrainIterator ti = mHorizonGroup->getTerrainIterator();
		while (ti.hasMoreElements())
		{
			Terrain* t = ti.getNext()->instance;
			//initBlendMaps(t);
			horizon = t;  //<set
			horizon->setVisibilityFlags(RV_Terrain);
		}
		mHorizonGroup->freeTemporaryResources();
	}
}//bTer

	
	///.
	changeShadows();

	//UpdBlendmap();  //-

	LogO(String("::: Time Terrain: ") + fToStr(tm.getMilliseconds(),0,3) + " ms");
}


//  save ter hmap to mem (all editing would be lost)
void CScene::copyTerHmap()
{
	if (!terrain) return;
	float *fHmap = terrain->getHeightData();
	int size = sc->td.iVertsX * sc->td.iVertsY * sizeof(float);
	memcpy(sc->td.hfHeight, fHmap, size);
}


//  Destroy
void CScene::DestroyTerrain()
{
	for (int i=0; i < 6; ++i)
	{
		texLayD[i].Destroy();
		texLayN[i].Destroy();
	}
	terrain = 0;
	if (mTerrainGroup)
		mTerrainGroup->removeAllTerrains();
}


//  Bullet Terrain
//---------------------------------------------------------------------------------------------------------------
void CScene::CreateBltTerrain()
{
	btHeightfieldTerrainShape* hfShape = new btHeightfieldTerrainShape(
		sc->td.iVertsX, sc->td.iVertsY, sc->td.hfHeight, sc->td.fTriangleSize,
		/*>?*/-300.f,300.f, 2, PHY_FLOAT,false);
	
	hfShape->setUseDiamondSubdivision(true);

	btVector3 scl(sc->td.fTriangleSize, sc->td.fTriangleSize, 1);
	hfShape->setLocalScaling(scl);
	hfShape->setUserPointer((void*)SU_Terrain);

	btCollisionObject* col = new btCollisionObject();
	col->setCollisionShape(hfShape);
	//col->setWorldTransform(tr);
	col->setFriction(0.9);   //+
	col->setRestitution(0.0);
	//col->setHitFraction(0.1f);
	col->setCollisionFlags(col->getCollisionFlags() |
		btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
	#ifndef SR_EDITOR  // game
		app->pGame->collision.world->addCollisionObject(col);
		app->pGame->collision.shapes.push_back(hfShape);
	#else
		app->world->addCollisionObject(col);
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

		app->pGame->collision.world->addCollisionObject(col);
		app->pGame->collision.shapes.push_back(shp);
	}
	#endif
}
