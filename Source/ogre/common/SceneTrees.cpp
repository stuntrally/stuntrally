#include "stdafx.h"
#ifdef ROAD_EDITOR
	#include "../../editor/OgreApp.h"
#else
	#include "../OgreGame.h"
	#include "../vdrift/settings.h"
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
				yaw = getRndAngle();
				pos.x = getTerPos();  pos.z = getTerPos();
				bool add = true;
				
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

				treeLoader->addTree(ent, pos, yaw,
					Math::RangeRandom(pg.minScale, pg.maxScale));
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