#include "stdafx.h"
#ifdef ROAD_EDITOR
	#include "../../editor/OgreApp.h"
#else
	#include "../OgreGame.h"
	#include "../vdrift/settings.h"
	#include "../vdrift/game.h"
#endif
#include "../../paged-geom/GrassLoader.h"
#include "../../paged-geom/BatchPage.h"
#include "../../paged-geom/WindBatchPage.h"
#include "../../paged-geom/ImpostorPage.h"
#include "../../paged-geom/TreeLoader2D.h"


//---------------------------------------------------------------------------------------------------------------
///  Trees  ^ ^ ^ ^
//---------------------------------------------------------------------------------------------------------------

Terrain* gTerrain = NULL;

inline Real getTerrainHeight(const Real x, const Real z, void *userData)
{
	//Real ofsY = 0.f;
	//if (userData)
	//	ofsY = *((Real*)userData);
	return gTerrain->getHeightAtWorldPosition(x, 0, z);// + ofsY;
}

#define getTerPos()		Math::RangeRandom(-0.5f, 0.5f) * sc.td.fTerWorldSize
#define getRndAngle()	Degree(Math::RangeRandom(0, 360))


void App::CreateTrees()
{
	gTerrain = terrain;
	
	//-------------------------------------- Grass --------------------------------------
	reloadMtrTex("grass"); //+

	TexturePtr rdtex = (TexturePtr)Ogre::TextureManager::getSingleton().getByName("grassDensity.png");
	if (!rdtex.isNull())
		rdtex->reload();

	int imgRoadSize = 0;
	Image imgRoad;  imgRoad.load("grassDensity.png","General");
	imgRoadSize = imgRoad.getWidth();  // square[]
	//imgRoad.save("grassDens.png");
	//Log("grass img " + toStr(imgRoadSize));


	using namespace Forests;
	Real tws = sc.td.fTerWorldSize * 0.5f;
	TBounds tbnd(-tws, -tws, tws, tws);
	Vector3 pos = Vector3::ZERO;  Radian yaw;

	bool bWind = 1;	 /// WIND

	Real fGrass = pSet->grass * sc.densGrass * 3.0f;  // min(pSet->grass, 
	Real fTrees = pSet->trees * sc.densTrees;
	
	if (fGrass > 0.f)
	{
		grass = new PagedGeometry(mCamera, sc.grPage);  //30
		grass->addDetailLevel<GrassPage>(sc.grDist * pSet->grass_dist);

		GrassLoader *grassLoader = new Forests::GrassLoader(grass);
		grass->setPageLoader(grassLoader);
		grassLoader->setHeightFunction(&getTerrainHeight);

		//  Add grass
		GrassLayer *l = grassLoader->addLayer("grass");
		l->setMinimumSize(sc.grMinSx, sc.grMinSy);
		l->setMaximumSize(sc.grMaxSx, sc.grMaxSy);
		l->setDensity(fGrass);  l->setSwayDistribution(sc.grSwayDistr);
		l->setSwayLength(sc.grSwayLen);  l->setSwaySpeed(sc.grSwaySpeed);

		l->setAnimationEnabled(true);
		//l->setLightingEnabled(true);  //!
		l->setRenderTechnique(/*GRASSTECH_SPRITE*/GRASSTECH_CROSSQUADS);
		l->setFadeTechnique(FADETECH_ALPHA/*FADETECH_GROW*/);

		l->setColorMap("grassColor.png");
		l->setDensityMap("grassDensity.png");
		l->setMapBounds(tbnd);
		grass->setShadersEnabled(true);//`
	}


	//-------------------------------------- Trees --------------------------------------
	if (fTrees > 0.f)
	{
		// fast: 100_ 80 j1T!,  400 400 good sav2f  200 220 both`-
		trees = new PagedGeometry(mCamera, sc.trPage);  //trees->setInfinite();
		if (bWind)
			 trees->addDetailLevel<WindBatchPage>(sc.trDist * pSet->trees_dist, 0);
		else trees->addDetailLevel<BatchPage>	 (sc.trDist * pSet->trees_dist, 0);
		trees->addDetailLevel<ImpostorPage>(sc.trDistImp * pSet->trees_dist, 0);

		TreeLoader2D* treeLoader = new TreeLoader2D(trees, tbnd);
		trees->setPageLoader(treeLoader);
		treeLoader->setHeightFunction(getTerrainHeight);
		treeLoader->setMaximumScale(4);//6
		tws = sc.td.fTerWorldSize;
		int r = imgRoadSize;

		//  layers
		for (size_t l=0; l < sc.pgLayers.size(); l++)
		{
			PagedLayer& pg = sc.pgLayersAll[sc.pgLayers[l]];
			//-treeLoader->setHeightFunction(getTerrainHeight, &pg.ofsY);

			Entity* ent = mSceneMgr->createEntity(pg.name);
			ent->setVisibilityFlags(8);  ///vis+  disable in render targets
			if (pg.windFx > 0.f)  {
				trees->setCustomParam(ent->getName(), "windFactorX", pg.windFx);
				trees->setCustomParam(ent->getName(), "windFactorY", pg.windFy);  }

			//  num trees
			int cnt = fTrees * 6000 * pg.dens;
			for (int i = 0; i < cnt; i++)
			{
			#if 1
				yaw = getRndAngle();
				pos.x = getTerPos();  pos.z = getTerPos();
				Vector3 pos0 = pos;
				Real scl = Math::RangeRandom(pg.minScale, pg.maxScale);
			#else
				yaw = Degree((i*45)%360);
				pos.z = -100 +(i / 10) * 20;  pos.x = -100 +(i % 10) * 20;
				Vector3 pos0 = pos;
				Real scl = 1.f;
			#endif
				bool add = true;

				//  ofs pos, rotY, scl
				Vector2 vofs(1.2,-0.5), vo;  float yr = -yaw.valueRadians();
				vo.x = vofs.x * cos(yr) - vofs.y * sin(yr);
				vo.y = vofs.x * sin(yr) + vofs.y * cos(yr);
				pos.x += vo.x * scl;  pos.z += vo.y * scl;
				
				//  check if on road
				if (r > 0)
				{
					int mx = (pos.x + 0.5*tws)/tws*r,
						my = (pos.z + 0.5*tws)/tws*r;

					int c = sc.trRdDist + pg.addTrRdDist;
					for (int jj = -c; jj <= c; ++jj)
					for (int ii = -c; ii <= c; ++ii)
						if (imgRoad.getColourAt(
							max(0,min(r-1, mx+ii)),
							max(0,min(r-1, my+jj)), 0).g < 0.95f)
								add = false;
				}
				
				if (!add)  continue;

				treeLoader->addTree(ent, pos0, yaw, scl);
					
			#if 0
			#ifndef ROAD_EDITOR  //  in Game
				///  add to bullet world, ..gui opt	// lower fps  83 > 72
				pos.y = terrain->getHeightAtWorldPosition(pos.x, 0, pos.z);
				btVector3 pc(pos.x, -pos.z, pos.y + 1.4*scl);  // center
				// offset xyz, mul by rotY mat, mul scale + ..
				
				btTransform tr;  tr.setIdentity();  tr.setOrigin(pc);

				//btCollisionShape* shp = new btSphereShape(0.5f);
				btCollisionShape* shp = new btCapsuleShapeZ(0.8f * scl, 2.f * scl);
				//shp->setUserPointer((void*)7777);  // mark as ..

				btCollisionObject* col = new btCollisionObject();
				col->setCollisionShape(shp);	col->setWorldTransform(tr);
				col->setFriction(0.2);			col->setRestitution(0.9);
				col->setCollisionFlags(col->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);
				pGame->collision.world.addCollisionObject(col);
				pGame->collision.shapes.push_back(shp);/**/
			#endif
			#endif
			}
		}

	}
	//imgRoadSize = 0;
}


//  reload material textures
//----------------------------------------------------------------------------------------------------------------
void App::reloadMtrTex(String mtrName)
{
	MaterialPtr mtr = (MaterialPtr)MaterialManager::getSingleton().getByName(mtrName);
	if (!mtr.isNull())
	{	Material::TechniqueIterator techIt = mtr->getTechniqueIterator();
		while (techIt.hasMoreElements())
		{	Technique* tech = techIt.getNext();
			Technique::PassIterator passIt = tech->getPassIterator();
			while (passIt.hasMoreElements())
			{	Pass* pass = passIt.getNext();
				Pass::TextureUnitStateIterator tusIt = pass->getTextureUnitStateIterator();
				while (tusIt.hasMoreElements())
				{	TextureUnitState* tus = tusIt.getNext();  String name = tus->getTextureName();
					if (name != "ReflectionCube")
					{
						Ogre::LogManager::getSingletonPtr()->logMessage( "Tex Reload: " + name );
						TexturePtr tex = (TexturePtr)Ogre::TextureManager::getSingleton().getByName( name );
						if (!tex.isNull())
						{							
							tex->reload();
						}
					}
				}
}	}	}	}