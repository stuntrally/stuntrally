#pragma once
#include <OgreRenderQueue.h>

#define rgDef  ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME


//  Render Queue Groups used  //search for setRenderQueueGroup
//------------------------------------------------------------------------

const Ogre::uint8
	RQG_Sky = Ogre::RENDER_QUEUE_SKIES_EARLY,	// 5
	
	RQG_BatchOpaque  = Ogre::RENDER_QUEUE_MAIN,	// 50  paged geom
	RQG_Road         = Ogre::RENDER_QUEUE_7,

	RQG_RoadBlend    = Ogre::RENDER_QUEUE_7+1,
	RQG_BatchAlpha   = Ogre::RENDER_QUEUE_7+4,	// paged geom transparent
	RQG_Fluid        = Ogre::RENDER_QUEUE_7+5,
	RQG_RoadMarkers  = Ogre::RENDER_QUEUE_7+6,

	RQG_CarGlass     = Ogre::RENDER_QUEUE_7+7,
	RQG_CarTrails    = Ogre::RENDER_QUEUE_8,	//trails after glass
	RQG_PipeGlass    = Ogre::RENDER_QUEUE_8+2,	// glass pipe road`
	RQG_CarParticles = Ogre::RENDER_QUEUE_8+3,	//particles after trails

	RQG_Weather      = Ogre::RENDER_QUEUE_8+5,
	RQG_CarGhost     = Ogre::RENDER_QUEUE_8+7,

	RQG_Hud1         = Ogre::RENDER_QUEUE_OVERLAY-5,	// 95
	RQG_Hud2         = Ogre::RENDER_QUEUE_OVERLAY-2,
	RQG_Hud3         = Ogre::RENDER_QUEUE_OVERLAY-1;	// 99


//  Visibility Flags used  //search for setVisibility
//------------------------------------------------------------------------
const Ogre::uint32
	RV_Road = 1,	// road only, for road textures
	RV_Hud = 2,		// hud and markers
	RV_Terrain = 4,	// terrain and fluids, for terrain texture
	RV_Vegetation = 8,  // vegetation, paged geom
	RV_VegetGrass = 64, // grass, paged geom
	RV_Objects = 256,  // all objects (static meshes and props)
	RV_Sky = 32,	// sky, editor only

	RV_Car = 128,		// car,tires in game, (hide in reflection render)
	RV_Particles = RV_Car,
	RV_CarGlass = 16,	// car glass in game, (hide for in car camera)
	RV_MaskReflect = RV_Road + RV_Terrain + RV_Vegetation + RV_Objects,  // hide 2: hud, car,glass,tires
	
	RV_WaterReflect = RV_Terrain + RV_Vegetation + RV_Road /*+ RV_Objects /*+ RV_Car*/,
	RV_WaterRefract = RV_Terrain + RV_Vegetation + RV_Road + RV_Objects + RV_Car,

	RV_MaskAll = 511,
	RV_MaskPrvCam = 512;
