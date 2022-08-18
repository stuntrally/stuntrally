#pragma once
#include "SColor.h"
#include "SceneClasses.h"

#include <Ogre.h>
// #include <OgreCommon.h>
// #include <OgreVector2.h>
// #include <OgreVector3.h>
// #include <OgreVector4.h>
// #include <OgreColourValue.h>
// #include <OgreQuaternion.h>
#include "../../../vdrift/mathvector.h"
#include "../../../vdrift/quaternion.h"

namespace Ogre {  class SceneNode;  class Entity;  class ParticleSystem;  }
namespace Forests {  class GrassLayer;  }


///  Scene setup xml
//  . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
class Scene
{
public:

	//  car start pos
	MATHVECTOR <float,3> startPos[2];  // [1] is end for not looped, and start for reversed
	QUATERNION <float>   startRot[2];
	std::pair <MATHVECTOR<float,3>, QUATERNION<float>> GetStart(int index, bool looped);

	//  preview cam
	Ogre::Vector3 camPos,camDir;


	//  Sky  ()
	Ogre::String skyMtr;  float skyYaw;
	int  rainEmit,rain2Emit;  Ogre::String rainName,rain2Name;
	//  Light
	float ldPitch, ldYaw;  // sun dir angles
	SColor lAmb,lDiff,lSpec;

	//  Fog
	float fogStart, fogEnd;  // lin range
	SColor fogClr,fogClr2;  // 2colors sun-away  .a = intensity

	SColor fogClrH;  // height fog color
	float fogHeight, fogHDensity, fogHStart, fogHEnd;
	float fHDamage;  // damage from low height fog


	//  game
	bool asphalt;  // use asphalt tires car
	bool denyReversed;  // track (road) dir
	bool noWrongChks;  // dont show "wrong checkpoint" messages

	float windAmt;  //, windDirYaw, windTurbulFreq,windTurbulAmp;
	float damageMul;  // reduce car damage in loops
	float gravity;  // 9.81
	

	//  sound  <)
	std::string sAmbient, sReverbs;  void UpdRevSet();
	struct RevSet  // copy from ReverbSet, name = sReverbs, from base if ""
	{	std::string descr,
			normal, cave, cavebig, pipe, pipebig, influid;
	} revSet;
	class ReverbsXml* pReverbsXml;  //! set this after Load
	

	//  particle types
	Ogre::String  sParDust, sParMud, sParSmoke;
	
	///  Terrain  ----
	bool ter;  // has terrain
	TerData td;

	
	///  Vegetation params  --------
	float densTrees, densGrass;  int grDensSmooth;
	float grPage, grDist;
	float trPage, trDist, trDistImp;
	int trRdDist;  // dist from road to trees

	//  grass layers
	const static int ciNumGrLay = 6;  // all, for edit
	SGrassLayer grLayersAll[ciNumGrLay];
	SGrassChannel grChan[4];

	//  paged layers  (models: trees,rocks,etc)
	const static int ciNumPgLay = 10;  // all, for edit
	PagedLayer pgLayersAll[ciNumPgLay];
	std::vector<int> pgLayers;    // active only (on)
	void UpdPgLayers();

	
	//  Fluids  ~~~
	std::vector<FluidBox> fluids;
	class FluidsXml* pFluidsXml;  //! set this after Load
	float GetDepthInFluids(Ogre::Vector3 pos);
	
	//  Objects  []o
	std::vector<Object> objects;

	//  Particles  Emitters  ::
	std::vector<SEmitter> emitters;


	//  base track (new from) for info
	std::string baseTrk;
	int secEdited;  // time in seconds of track editing

	
	//  Main methods  ----
	Scene();  void Default();
	void UpdateFluidsId(), UpdateSurfId();

	class GAME* pGame;  // for all surfaces by name
	bool LoadXml(Ogre::String file, bool bTer = true), SaveXml(Ogre::String file);
};
