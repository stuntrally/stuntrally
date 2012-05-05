#ifndef __Defines_h__
#define __Defines_h__


#define PI_d  3.14159265358979323846

#include <OgreLogManager.h>
#define LogO(s)  Ogre::LogManager::getSingleton().logMessage(s)

#include <OgreStringConverter.h>
#define toStr(v)   Ogre::StringConverter::toString(v)
#define toStrC(v)  Ogre::StringConverter::toString(v).c_str()

static Ogre::String fToStr(const float v, unsigned short precision, unsigned short width=0, const char fill=' ')
{
	std::ostringstream s;
	if (width!=0) s.width(width);
	s.fill(fill);
	s << std::fixed << std::setprecision(precision) << v;
	return s.str();
} 

#define s2r(s)  Ogre::StringConverter::parseReal(s)
#define s2i(s)  Ogre::StringConverter::parseInt(s)
#define s2c(s)  Ogre::StringConverter::parseColourValue(s)
#define s2v(s)  Ogre::StringConverter::parseVector3(s)

/// translation
#define TR(s)  MyGUI::LanguageManager::getInstance().replaceTags(s)


//  info for collision shapes  (hit, triggers)
enum EShapeType
{
	ST_Car=0,
	ST_Fluid,
	ST_Wheel,
	//ST_Terrain, ST_BorderPlane,
	//ST_Vegetation,  //-> stone, wood, plant ..
	//ST_Road, ST_RoadWall, //ST_RoadColumn, ST_RoadPipe glass-
	ST_Other
};

class CARDYNAMICS;  class FluidBox;
class ShapeData
{
public:
	EShapeType type;
	CARDYNAMICS* pCarDyn;
	FluidBox* pFluid;
	int whNum;

	ShapeData( EShapeType type1)
		: type(type1), pCarDyn(0), pFluid(0), whNum(0)
	{	}
	ShapeData( EShapeType type1, CARDYNAMICS* pCarDyn1, FluidBox* pFluid1, int whNum1=0)
		: type(type1), pCarDyn(pCarDyn1), pFluid(pFluid1), whNum(whNum1)
	{	}
};

/*const static ShapeData
	gSD_Terrain(ST_Terrain), gSD_BorderPlane(ST_BorderPlane),
	gSD_Road(ST_Road), gSD_RoadWall(ST_RoadWall),
	gSD_Other(ST_Other);/**/


///  boost fuel params  ----
const static float gfBoostFuelStart = 3.f,  // seconds (each lap)
	gfBoostFuelMax = 3.f,  // max val, tank	
	gfBoostFuelAddSec = 0.1f;  // add value each second

#endif
