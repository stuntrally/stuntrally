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
//bool gbLookAround = false;

inline Real getTerrainHeight(const Real x, const Real z, void *userData)
{
	return gTerrain->getHeightAtWorldPosition(x, 0, z);
}
inline Real getTerrainHeightAround(const Real x, const Real z, void *userData)
{
	float h = gTerrain->getHeightAtWorldPosition(x, 0, z);
	
	#if 0   // testing..
	const float d = 0.3f;
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

#define getTerPos()		Math::RangeRandom(-0.5f, 0.5f) * sc.td.fTerWorldSize
#define getRndAngle()	Degree(Math::RangeRandom(0, 360))

/*
struct BltShape
{
	int type;  // 0 sphere, 1 capsule
	float radius, height;
	Vector3 offset;  Quaternion rot;
	float friction, restitution;
};
class BCollision
{
	vector<BltShape>
};
map<string,BCollision> ?
/**/

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
		treeLoader->setHeightFunction(getTerrainHeightAround /*,userdata*/);
		treeLoader->setMaximumScale(4);//6
		tws = sc.td.fTerWorldSize;
		int r = imgRoadSize, cntr = 0;

		//  layers
		for (size_t l=0; l < sc.pgLayers.size(); l++)
		{
			PagedLayer& pg = sc.pgLayersAll[sc.pgLayers[l]];

			Entity* ent = mSceneMgr->createEntity(pg.name);
			ent->setVisibilityFlags(8);  ///vis+  disable in render targets
			if (pg.windFx > 0.f)  {
				trees->setCustomParam(ent->getName(), "windFactorX", pg.windFx);
				trees->setCustomParam(ent->getName(), "windFactorY", pg.windFy);  }
				
			///  bullet shape params  todo: to xml...
			//--------------------------------------------------
			#ifndef ROAD_EDITOR  //  in Game
			enum BLT_SHP {  BLT_None=0, BLT_Sphere, BLT_CapsZ, } eShp = BLT_Sphere;
			btScalar frict = 0.2, restit = 0.9;
			Vector3 ofs(0,0,0);  Real radius = 1.f, height = 1.f;

			if (pSet->veget_collis)
			{	 if (pg.name == "tree.07.mesh")	{	ofs = Vector3( 1.2, -0.7,  1.4);  radius = 1.1;  height = 2.0;  eShp = BLT_CapsZ;	}
			else if (pg.name == "tree.09.mesh")	{	ofs = Vector3(-1.2, -0.35, 1.4);  radius = 1.3;  height = 2.0;  eShp = BLT_CapsZ;	}
			else if (pg.name == "rock.05.mesh"
				 ||  pg.name == "rock.05w.mesh"){	ofs = Vector3(0.0, 0.0, 0.2);  radius = 12.0;  eShp = BLT_Sphere;	}
			else if (pg.name == "rock.07.mesh"
				 ||  pg.name == "rock.07w.mesh"){	ofs = Vector3(0.0, 0.0, 0.2);  radius = 12.0;  eShp = BLT_Sphere;	}
			else if (pg.name == "farn1.mesh")	{	ofs = Vector3(0,0,0);  radius = 1.4;  eShp = BLT_Sphere;  frict = 0.3;  }
			else if (pg.name == "farn2.mesh")	{	ofs = Vector3(0,0,0);  radius = 1.6;  eShp = BLT_Sphere;  frict = 0.3;  }
			else if (pg.name == "fir05.mesh"
				 ||  pg.name == "fir05w.mesh")	{	ofs = Vector3(0,0,30);  radius = 4.1;  height = 69.0;  eShp = BLT_CapsZ;	}
			else if (pg.name == "fir06.mesh"
				 ||  pg.name == "fir06w.mesh")	{	ofs = Vector3(0,0,30);  radius = 4.1;  height = 69.0;  eShp = BLT_CapsZ;	}
			else if (pg.name == "fir14.mesh"
				 ||  pg.name == "fir14w.mesh")	{	ofs = Vector3(0,0,30);  radius = 4.1;  height = 69.0;  eShp = BLT_CapsZ;	}
			else if (pg.name == "palm.mesh")	{	ofs = Vector3(0.0, 0.0, 1.4);  radius = 0.4;  height = 3.0;  eShp = BLT_CapsZ;  frict = 0.7;  }
			//none: plant1 plant2 shroom1_1 shroom1_2 shroom1_3 shroom2_1 shroom2_2 shroom2_3
			}
			#endif
			
			//--------------------------------------------------

			//  num trees
			int cnt = fTrees * 6000 * pg.dens;
			for (int i = 0; i < cnt; i++)
			{
			#if 1
				yaw = getRndAngle();
				pos.x = getTerPos();  pos.z = getTerPos();
				Real scl = Math::RangeRandom(pg.minScale, pg.maxScale);
			#else  // test angle
				yaw = Degree((i*45)%360);
				pos.z = -100 +(i / 10) * 20;  pos.x = -100 +(i % 10) * 20;
				Real scl = Math::RangeRandom(pg.minScale, pg.maxScale);
			#endif
				Vector3 pos0 = pos;
				bool add = true;

			#ifndef ROAD_EDITOR  //  in Game
				//  ofs pos, rotY, scl
				Vector2 vo;  float yr = -yaw.valueRadians();
				vo.x = ofs.x * cos(yr) - ofs.y * sin(yr);
				vo.y = ofs.x * sin(yr) + ofs.y * cos(yr);
				pos.x += vo.x * scl;  pos.z += vo.y * scl;
			#endif
				
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
				cntr++;
					
				#ifndef ROAD_EDITOR  //  in Game
				if (pSet->veget_collis && eShp != BLT_None)
				{
					///  add to bullet world  lower fps..  83 > 72
					//  apply pos offset xyz, rotY, mul by scale
					pos.y = terrain->getHeightAtWorldPosition(pos.x, 0, pos.z);
					btVector3 pc(pos.x, -pos.z, pos.y + ofs.z * scl);  // center
					btTransform tr;  tr.setIdentity();  tr.setOrigin(pc);

					btCollisionShape* shp = 0;
					if (eShp == BLT_Sphere)
						shp = new btSphereShape(radius * scl);
					else
						shp = new btCapsuleShapeZ(radius * scl, height * scl);
					//shp->setUserPointer((void*)7777);  // mark as ..

					btCollisionObject* col = new btCollisionObject();
					col->setActivationState(DISABLE_SIMULATION);
					col->setCollisionShape(shp);	col->setWorldTransform(tr);
					col->setFriction(frict);		col->setRestitution(restit);
					col->setCollisionFlags(col->getCollisionFlags() |
						btCollisionObject::CF_STATIC_OBJECT /*| btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT/**/);
					pGame->collision.world.addCollisionObject(col);
					pGame->collision.shapes.push_back(shp);
				}
				#endif
			}
		}
		Ogre::LogManager::getSingletonPtr()->logMessage(string("***** Vegetation objects count: ") + toStr(cntr));
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