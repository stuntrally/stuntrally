#pragma once
#include "PreviewTex.h"
#include <Ogre.h>
// #include <OgreVector3.h>
// #include <OgreVector4.h>
// #include <OgreString.h>
// #include <OgreTexture.h>
// #include <OgreShadowCameraSetup.h>

namespace Forests {  class PagedGeometry;  }
namespace Ogre  {  class Terrain;  class TerrainGlobalOptions;  class TerrainGroup;  class StaticGeometry;
	class Light;  class SceneNode;  class Camera;  class Texture;  class SceneManager;  class Entity;
	class Rectangle2D;  class RenderTexture;  class Viewport;  class Root;  class ParticleSystem; }
class App;  class Scene;  class WaterRTT;  class CData;  class SplineRoad;  class PaceNotes;


class CScene
{
public:
	App* app;
	CScene(App* app1);
	~CScene();

	void destroyScene();


	//  Shadows
	void changeShadows(), UpdShaderParams(), UpdPaceParams(), UpdPSSMMaterials();
	Ogre::Vector4 splitPoints;
	Ogre::ShadowCameraSetupPtr mPSSMSetup;
	

	///  Setup  scene.xml
	Scene* sc;

	//  const, xmls
	CData* data;

	
	//  Sun
	Ogre::Light* sun;
	void UpdFog(bool bForce=false), UpdSun(), UpdSky();
	void CreateSkyDome(Ogre::String sMater, Ogre::Vector3 scale, float yaw);

	//  Weather  rain, snow
	Ogre::ParticleSystem *pr,*pr2;
	void CreateWeather(), DestroyWeather();
	void UpdateWeather(Ogre::Camera* cam, float mul = 1.f);


	//  Emitters
	void CreateEmitters(), DestroyEmitters(bool clear);


	//  Fluids  water, mud
	std::vector<Ogre::String/*MeshPtr*/> vFlSMesh;
	std::vector<Ogre::Entity*> vFlEnt;
	std::vector<Ogre::SceneNode*> vFlNd;
	void CreateFluids(), DestroyFluids(), CreateBltFluids();

	WaterRTT* mWaterRTT;
	void UpdateWaterRTT(Ogre::Camera* cam);


	//  Road
	int rdCur = 0;  // cur
	std::vector<SplineRoad*> roads;
	SplineRoad* road = 0;  // main
	
	SplineRoad* trail = 0;  // driving aids
	PaceNotes* pace = 0;
	void DestroyRoads(), DestroyPace(), DestroyTrail();

	
	//  Vegetation
	Forests::PagedGeometry *trees, *grass;
	void CreateTrees(), DestroyTrees(), RecreateTrees(), updGrsTer(), UpdCamera();


	///  Terrain
	//-----------------------------------
	PreviewTex texLayD[6],texLayN[6];  // layers
	void CreateTerrain(bool bNewHmap=false, bool bTer=true, bool terLoad=true);
	void DestroyTerrain(), CreateBltTerrain(), copyTerHmap();

	Ogre::Terrain* terrain;
	Ogre::TerrainGlobalOptions* mTerrainGlobals;
	Ogre::TerrainGroup* mTerrainGroup;
	void SetupTerrain(), UpdTerErr();

	Ogre::Terrain* horizon;
	Ogre::TerrainGlobalOptions* mHorizonGlobals;
	Ogre::TerrainGroup* mHorizonGroup;
	void SetupHorizon();

	
	//  Blendmap, rtt
	//-----------------------------------
	void CreateBlendTex(), UpdBlendmap(), UpdLayerPars();
	void UpdGrassDens(), UpdGrassPars();
	
	//  tex, mtr names
	const static Ogre::String sHmap, sAng,sAngMat,
		sBlend,sBlendMat, sGrassDens,sGrassDensMat;

	Ogre::TexturePtr heightTex, angleRTex, blendRTex;  // height, angles, blend
	Ogre::TexturePtr grassDensRTex;  // grass density and channels
	PreviewTex roadDens;

	struct RenderToTex  // rtt common
	{
		Ogre::RenderTexture* rnd;  Ogre::Texture* tex;
		Ogre::SceneManager* scm;  Ogre::Camera* cam;  Ogre::Viewport* vp;
		Ogre::Rectangle2D* rect;  Ogre::SceneNode* nd;

		void Null()
		{	rnd = 0;  tex = 0;  scm = 0;  cam = 0;  vp = 0;  rect = 0;  nd = 0;   }
		RenderToTex()
		{	Null();   }

		void Setup(Ogre::Root* rt, Ogre::String sName, Ogre::TexturePtr pTex, Ogre::String sMtr);
	};
	RenderToTex blendRTT, angleRTT, grassDensRTT;


	//  noise  -------
	static float Noise(float x, float zoom, int octaves, float persistence);
	static float Noise(float x, float y, float zoom, int octaves, float persistance);
	//     xa  xb
	//1    .___.
	//0__./     \.___
	//   xa-s    xb+s    // val, min, max, smooth range
	inline static float linRange(const float& x, const float& xa, const float& xb, const float& s)
	{
		if (x <= xa-s || x >= xb+s)  return 0.f;
		if (x >= xa && x <= xb)  return 1.f;
		if (x < xa)  return (x-xa)/s+1;
		if (x > xb)  return (xb-x)/s+1;
		return 0.f;
	}

};
