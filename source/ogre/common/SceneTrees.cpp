#include "pch.h"
#include "../common/Defines.h"
#include "RenderConst.h"
#ifdef ROAD_EDITOR
	#include "../../editor/OgreApp.h"
#else
	#include "../OgreGame.h"
	#include "../../vdrift/settings.h"
	#include "../../vdrift/game.h"
	#include "../SplitScreen.h"
#endif
#include "../../vdrift/pathmanager.h"
#include "../../paged-geom/GrassLoader.h"
#include "../../paged-geom/BatchPage.h"
#include "../../paged-geom/WindBatchPage.h"
#include "../../paged-geom/ImpostorPage.h"
#include "../../paged-geom/TreeLoader2D.h"
#include "../../paged-geom/MersenneTwister.h"
#include "BltObjects.h"

#include <boost/filesystem.hpp>
#include <OgreTerrain.h>
using namespace Ogre;


//---------------------------------------------------------------------------------------------------------------
///  Trees  ^ ^ ^ ^
//---------------------------------------------------------------------------------------------------------------

Terrain* gTerrain = NULL;
//bool gbLookAround = false;

inline Real getTerrainHeight(const Real x, const Real z, void *userData)
{
	return gTerrain->getHeightAtWorldPosition(x, 0, z);
}
inline Real getTerrainHeightAround(const Real x, const Real z, void *userData)
{
	float h = gTerrain->getHeightAtWorldPosition(x, 0, z);
	
	#if 0   // testing..
	const float d = 0.4f;
	for (int j=-2; j <= 2; ++j)
	for (int i=-2; i <= 2; ++i)
	if (i != 0 && j != 0)
	{
		float fx = i * d, fz = j * d;
		float hh = gTerrain->getHeightAtWorldPosition(x + fx, 0, z + fz);
		if (hh < h)  // if lower
			h = hh;
	}
	#endif

	return h;
}


void App::CreateTrees()
{
	QTimer ti;  ti.update();  /// time
	gTerrain = terrain;
	
	//-------------------------------------- Grass --------------------------------------
	int imgRoadSize = 0;
	Image imgRoad;
	try{
		imgRoad.load(String("roadDensity.png"),"General");
	}catch(...)
	{
		imgRoad.load(String("grassDensity.png"),"General");
	}
	imgRoadSize = imgRoad.getWidth();  // square[]
		
	// remove old BinFolder's (paged geom temp resource groups)
	if (ResourceGroupManager::getSingleton().resourceGroupExists("BinFolder"))
	{
		StringVectorPtr locations = ResourceGroupManager::getSingleton().listResourceLocations("BinFolder");
		for (StringVector::const_iterator it=locations->begin(); it!=locations->end(); ++it)
		{
			ResourceGroupManager::getSingleton().removeResourceLocation( (*it), "BinFolder" );
		}
	}

	using namespace Forests;
	Real tws = sc.td.fTerWorldSize * 0.5f;
	TBounds tbnd(-tws, -tws, tws, tws);
	//  pos0 - original  pos - with offset
	Vector3 pos0 = Vector3::ZERO, pos = Vector3::ZERO;  Radian yaw;

	bool bWind = 1;	 /// WIND

	Real fGrass = pSet->grass * sc.densGrass * 3.0f;  // std::min(pSet->grass, 
	#ifdef ROAD_EDITOR
	Real fTrees = pSet->gui.trees * sc.densTrees;
	#else
	Real fTrees = pSet->game.trees * sc.densTrees;
	#endif
	
	if (fGrass > 0.f)
	{
		#ifndef ROAD_EDITOR
		grass = new PagedGeometry(mSplitMgr->mCameras.front(), sc.grPage);  //30
		#else
		grass = new PagedGeometry(mCamera, sc.grPage);  //30
		#endif
		
		// create dir if not exist
		boost::filesystem::create_directory(PATHMANAGER::GetCacheDir() + "/" + toStr(sc.sceneryId));
		grass->setTempDir(PATHMANAGER::GetCacheDir() + "/" + toStr(sc.sceneryId) + "/");
		
		grass->addDetailLevel<GrassPage>(sc.grDist * pSet->grass_dist);

		GrassLoader *grassLoader = new Forests::GrassLoader(grass);
		grassLoader->setRenderQueueGroup(RQG_BatchAlpha);
		grass->setPageLoader(grassLoader);
		grassLoader->setHeightFunction(&getTerrainHeight);

		//  Add grass
		GrassLayer *l = grassLoader->addLayer(sc.grassMtr);
		l->setMinimumSize(sc.grMinSx, sc.grMinSy);
		l->setMaximumSize(sc.grMaxSx, sc.grMaxSy);
		l->setDensity(fGrass);  l->setSwayDistribution(sc.grSwayDistr);
		l->setSwayLength(sc.grSwayLen);  l->setSwaySpeed(sc.grSwaySpeed);

		l->setAnimationEnabled(true);  //l->setLightingEnabled(true);
		l->setRenderTechnique(GRASSTECH_CROSSQUADS);  //GRASSTECH_SPRITE-
		l->setFadeTechnique(FADETECH_ALPHA);  //FADETECH_GROW-

		l->setColorMap(sc.grassColorMap);
		l->setDensityMap("grassDensity.png",CHANNEL_RED);
		l->setMapBounds(tbnd);
		grass->setShadersEnabled(true);
	}
	ti.update();  /// time
	float dt = ti.dt * 1000.f;
	LogO(String("::: Time Grass: ") + toStr(dt) + " ms");


	//---------------------------------------------- Trees ----------------------------------------------
	if (fTrees > 0.f)
	{
		// fast: 100_ 80 j1T!,  400 400 good sav2f  200 220 both`-
		#ifndef ROAD_EDITOR
		trees = new PagedGeometry(mSplitMgr->mCameras.front(), sc.trPage);
		#else
		trees = new PagedGeometry(mCamera, sc.trPage);
		#endif
		
		// create dir if not exist
		boost::filesystem::create_directory(PATHMANAGER::GetCacheDir() + "/" + toStr(sc.sceneryId));
		trees->setTempDir(PATHMANAGER::GetCacheDir() + "/" + toStr(sc.sceneryId) + "/");

		if (bWind)
			 trees->addDetailLevel<WindBatchPage>(sc.trDist * pSet->trees_dist, 0);
		else trees->addDetailLevel<BatchPage>	 (sc.trDist * pSet->trees_dist, 0);
		if (pSet->use_imposters)
			trees->addDetailLevel<ImpostorPage>(sc.trDistImp * pSet->trees_dist, 0);

		TreeLoader2D* treeLoader = new TreeLoader2D(trees, tbnd);
		trees->setPageLoader(treeLoader);
		treeLoader->setHeightFunction(getTerrainHeightAround /*,userdata*/);
		treeLoader->setMaximumScale(4);//6
		tws = sc.td.fTerWorldSize;
		int r = imgRoadSize, cntr = 0, cntshp = 0, txy = sc.td.iVertsX*sc.td.iVertsY-1;

		//  set random seed  /// todo: seed in scene.xml and in editor gui...
		MTRand rnd((MTRand::uint32)1213);
		#define getTerPos()		(rnd.rand()-0.5) * sc.td.fTerWorldSize

		//  layers
		for (size_t l=0; l < sc.pgLayers.size(); l++)
		{
			PagedLayer& pg = sc.pgLayersAll[sc.pgLayers[l]];

			Entity* ent = mSceneMgr->createEntity(pg.name);
			ent->setVisibilityFlags(RV_Vegetation);  ///vis+  disable in render targets
			if (pg.windFx > 0.f)  {
				trees->setCustomParam(ent->getName(), "windFactorX", pg.windFx);
				trees->setCustomParam(ent->getName(), "windFactorY", pg.windFy);  }

			///  collision object
			const BltCollision* col = objs.Find(pg.name);
			Vector3 ofs(0,0,0);  if (col)  ofs = col->offset;  // mesh offset

			//  num trees  ----------------------------------------------------------------
			int cnt = fTrees * 6000 * pg.dens;
			for (int i = 0; i < cnt; i++)
			{
				#if 0  ///  for new objects - test shapes
					yaw = Degree((i*45)%360);  // grid
					pos.z = -100 +(i / 10) * 20;  pos.x = -100 +(i % 10) * 20;
					Real scl = rnd.rand() * (pg.maxScale-pg.minScale) + pg.minScale;
				#else
					yaw = Degree(rnd.rand(360.0));
					pos.x = getTerPos();  pos.z = getTerPos();
					Real scl = rnd.rand() * (pg.maxScale-pg.minScale) + pg.minScale;
				#endif
				pos0 = pos;  // store original place
				bool add = true;

				//  offset mesh  pos, rotY, scl
				Vector2 vo;  float yr = -yaw.valueRadians();
				float cyr = cos(yr), syr = sin(yr);
				vo.x = ofs.x * cyr - ofs.y * syr;  // ofs x,y for pos x,z
				vo.y = ofs.x * syr + ofs.y * cyr;
				pos.x += vo.x * scl;  pos.z += vo.y * scl;
				
				//  check if on road - uses grassDensity
				if (r > 0)  //  ----------------
				{
				int mx = (pos.x + 0.5*tws)/tws*r,
					my = (pos.z + 0.5*tws)/tws*r;

					int c = sc.trRdDist + pg.addTrRdDist;
					for (int jj = -c; jj <= c; ++jj)
					for (int ii = -c; ii <= c; ++ii)
						if (imgRoad.getColourAt(
							std::max(0,std::min(r-1, mx+ii)),
							std::max(0,std::min(r-1, my+jj)), 0).r < 0.75f)  //par-
								add = false;
				}
				if (!add)  continue;  //?faster

				//  check ter angle  ------------
				int mx = (pos.x + 0.5*tws)/tws*sc.td.iVertsX,
					my = (pos.z + 0.5*tws)/tws*sc.td.iVertsY;
				int a = std::max(0, std::min(txy, my*sc.td.iVertsX+mx));
				if (sc.td.hfAngle[a] > pg.maxTerAng)
					add = false;

				if (!add)  continue;  //

				//  check ter height  ------------
				pos.y = terrain->getHeightAtWorldPosition(pos.x, 0, pos.z);
				if (pos.y < pg.minTerH || pos.y > pg.maxTerH)
					add = false;				
				
				if (!add)  continue;  //
				
				//  check if in fluids  ------------
				float fa = 0.f;  // depth
				for (int fi=0; fi < sc.fluids.size(); ++fi)
				{
					const FluidBox& fb = sc.fluids[fi];
					if (fb.pos.y - pos.y > 0.f)  // dont check when above fluid, ..or below its size-
					{
						const float sizex = fb.size.x*0.5f, sizez = fb.size.z*0.5f;
						//  check rect 2d - no rot !
						if (pos.x > fb.pos.x - sizex && pos.x < fb.pos.x + sizex &&
							pos.z > fb.pos.z - sizez && pos.z < fb.pos.z + sizez)
						{
							float f = fb.pos.y - pos.y;
							if (f > fa)  fa = f;
						}
					}
				}
				if (fa > pg.maxDepth)
					add = false;
				
				if (!add)  continue;

				treeLoader->addTree(ent, pos0, yaw, scl);
				cntr++;
					
				
				///  add to bullet world
				#ifndef ROAD_EDITOR  //  in Game
				if (pSet->game.collis_veget && col)
				for (int c=0; c < col->shapes.size(); ++c)  // all shapes
				{
					const BltShape* shp = &col->shapes[c];
					Vector3 pos = pos0;  // restore original place
					Vector3 ofs = shp->offset;
					//  offset shape  pos, rotY, scl
					Vector2 vo;  float yr = -yaw.valueRadians();
					float cyr = cos(yr), syr = sin(yr);
					vo.x = ofs.x * cyr - ofs.y * syr;
					vo.y = ofs.x * syr + ofs.y * cyr;
					pos.x += vo.x * scl;  pos.z += vo.y * scl;

					//  apply pos offset xyz, rotY, mul by scale
					pos.y = terrain->getHeightAtWorldPosition(pos.x, 0, pos.z);
					btVector3 pc(pos.x, -pos.z, pos.y + ofs.z * scl);  // center
					btTransform tr;  tr.setIdentity();  tr.setOrigin(pc);

					btCollisionShape* bshp = 0;
					if (shp->type == BLT_CapsZ)
						bshp = new btCapsuleShapeZ(shp->radius * scl, shp->height * scl);
					else
						bshp = new btSphereShape(shp->radius * scl);
					//shp->setUserPointer((void*)7777);  // mark as vegetation ..

					btCollisionObject* bco = new btCollisionObject();
					bco->setActivationState(DISABLE_SIMULATION);
					bco->setCollisionShape(bshp);	bco->setWorldTransform(tr);
					bco->setFriction(shp->friction);	bco->setRestitution(shp->restitution);
					bco->setCollisionFlags(bco->getCollisionFlags() |
						btCollisionObject::CF_STATIC_OBJECT /*| btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
					pGame->collision.world->addCollisionObject(bco);
					pGame->collision.shapes.push_back(bshp);  cntshp++;
				}
				#endif
			}
		}
		LogO(String("***** Vegetation objects count: ") + toStr(cntr) + "  shapes: " + toStr(cntshp));
	}
	//imgRoadSize = 0;
	ti.update();  /// time
	dt = ti.dt * 1000.f;
	LogO(String("::: Time Trees: ") + toStr(dt) + " ms");
}
